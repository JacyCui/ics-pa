/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/vaddr.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include "sdb.h"


enum {
  TK_NOTYPE = 256, 
  TK_HEX_NUM,
  TK_DEC_NUM,
  TK_REG,
  TK_SYMBOL,
  TK_EQ,
  TK_NE,
  TK_GE,
  TK_LE,
  TK_AND,
  TK_OR,
  TK_NEG,
  TK_DE_REF,
  TK_SHIFT_L,
  TK_SHIFT_R
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
  {" +", TK_NOTYPE},    // spaces
  {"0[xX][0-9a-fA-F]+", TK_HEX_NUM},
  {"[0-9]+", TK_DEC_NUM},
  {"\\+", '+'},
  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"%", '%'},
  {"\\$[\\$]?[a-z0-9]+", TK_REG},
  {"[a-zA-Z]+[a-zA-Z0-9_]*", TK_SYMBOL},
  {"\\(", '('},
  {"\\)", ')'},
  {"==", TK_EQ},
  {"!=", TK_NE},
  {">=", TK_GE},
  {"<=", TK_LE},
  {"<<", TK_SHIFT_L},
  {">>", TK_SHIFT_R},
  {">", '>'},
  {"<", '<'},
  {"!", '!'},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"&", '&'},
  {"\\|", '|'},
  {"\\^", '^'},
  {"~", '~'}
};

#define UNARY_OPERATOR 100

static int get_priority(int type) {
  switch (type) {
    case TK_OR: return 0;
    case TK_AND: return 1;
    case TK_EQ: case TK_NE: case '>': case '<': case TK_GE: case TK_LE: return 2;
    case TK_SHIFT_L: case TK_SHIFT_R: return 3;
    case '+': case '-': return 4;
    case '%': case '*': case '/': return 5;
    case '^': return 6;
    case '|': return 7;
    case '&': return 8;
    case '!': case TK_NEG: case TK_DE_REF: case '~': return UNARY_OPERATOR;
    default: return -1;
  }
}


#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

#define MAX_TOKEN_LEN 32
#define MAX_TOKEN_NUM 128

typedef struct token {
  int type;
  char str[MAX_TOKEN_LEN];
} Token;

static Token tokens[MAX_TOKEN_NUM] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {

      if (nr_token >= MAX_TOKEN_NUM) {
        printf("Subexpression ...%s causes a buffer overflow!", e + position);
        return false;
      }

      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case '-':
            if (nr_token != 0 && (tokens[nr_token - 1].type == TK_DEC_NUM ||
            tokens[nr_token - 1].type == TK_HEX_NUM || tokens[nr_token - 1].type == TK_REG ||
            tokens[nr_token - 1].type == ')')) {
              tokens[nr_token++].type = '-';
            } else {
              tokens[nr_token++].type = TK_NEG;
            }
            break;
          case '*':
            if (nr_token != 0 && (tokens[nr_token - 1].type == TK_DEC_NUM ||
            tokens[nr_token - 1].type == TK_HEX_NUM || tokens[nr_token - 1].type == TK_REG ||
            tokens[nr_token - 1].type == ')')) {
              tokens[nr_token++].type = '*';
            } else {
              tokens[nr_token++].type = TK_DE_REF;
            }
            break;
          case TK_DEC_NUM: case TK_HEX_NUM: case TK_REG: case TK_SYMBOL:
            if (substr_len >= MAX_TOKEN_LEN) {
              char *buffer = (char *)malloc(sizeof(char) * (substr_len + 1));
              strncpy(buffer, substr_start, substr_len);
              buffer[substr_len] = '\0';
              printf("Token %s with size %d causes buffer overflow!\n", buffer, substr_len);
              free(buffer);
              return false;
            }
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
          default:
            tokens[nr_token++].type = rules[i].token_type;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


static word_t eval(int p, int q, bool *success);
static bool check_parentheses(int p, int q, bool *success);
static int dominant_operator(int p, int q);

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  return eval(0, nr_token - 1, success);
}


static word_t eval(int p, int q, bool *success) {
  if (p > q) {
    *success = false;
    printf("Empty subexpression at the %dth token.\n", p);
    return 0;
  }
  word_t result;
  if (p == q) {
    switch (tokens[p].type) {
      case TK_DEC_NUM: case TK_HEX_NUM:
        result = strtoul(tokens[p].str, NULL, 0);
        break;
      case TK_REG:
        result = isa_reg_str2val(tokens[p].str + 1, success);
        break;
      case TK_SYMBOL:
        result = isa_lookup_symtab_by_name(tokens[p].str, success);
        break;
      default:
        printf("Unknown primitive type.\n");
        *success = false;
        result = 0;
    }
    return result;
  }
  if (check_parentheses(p, q, success)) {
    return eval(p + 1, q - 1, success);
  }
  if (!*success) {
    return 0;
  }
  int op = dominant_operator(p, q);
  word_t val1, val2;
  if (op != -1) {
    val1 = eval(p, op - 1, success);
    if (!*success) {
      return 0;
    }
    val2 = eval(op + 1, q, success);
    if (!*success) {
      return 0;
    }
    switch (tokens[op].type) {
      case TK_OR: return val1 || val2;
      case TK_AND: return val1 && val2;
      case TK_EQ: return val1 == val2;
      case TK_NE: return val1 != val2;
      case TK_LE: return val1 <= val2;
      case TK_GE: return val1 >= val2;
      case '<': return val1 < val2;
      case '>': return val1 > val2;
      case TK_SHIFT_L: return val1 << val2;
      case TK_SHIFT_R: return val1 >> val2;
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '%':
        if (val2 == 0) {
          printf("Moded by zero.\n");
          *success = false;
          return 0;
        }
        return val1 % val2;
      case '*': return val1 * val2;
      case '/':
        if (val2 == 0) {
          printf("Divided by zero.\n");
          *success = false;
          return 0;
        }
        return val1 / val2;
      case '^': return val1 ^ val2;
      case '|': return val1 | val2;
      case '&': return val1 & val2;
      default:
        printf("Unknown bianry operation.\n");
        *success = false;
        return 0;
    }
  }
  vaddr_t vaddr;
  switch (tokens[p].type) {
    case TK_NEG: return -eval(p + 1, q, success);
    case TK_DE_REF:
      vaddr = eval(p + 1, q, success);
      if (!*success) {
        return 0;
      }
      return vaddr_read(vaddr, 4);
    case '!': return !eval(p + 1, q, success);
    case '~': return ~eval(p + 1, q, success);
    default:
      printf("Unknown unary operation %c.\n", tokens[p].type);
      *success = false;
      return 0;
  }
}

static bool check_parentheses(int p, int q, bool *success) {
  if (tokens[p].type != '(' || tokens[q].type != ')') {
    return false;
  }
  int stack = 0;
  int i;
  for (i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      stack++;
    } else if (tokens[i].type == ')') {
      stack--;
    }
    if (stack < 0) {
      printf("Unbalanced parentheses!\n");
      *success = false;
      return false;
    }
    if (stack == 0 && i != q) {
      return false;
    }
  }
  if (stack != 0) {
    *success = false;
    return false;
  }
  return true;
}

static int dominant_operator(int p, int q) {
  int split = -1, stack = 0;
  bool first = true;
  int i;
  for (i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      stack++;
      continue;
    }
    if (tokens[i].type == ')') {
      stack--;
      continue;
    }
    if (stack == 0 && get_priority(tokens[i].type) != -1 && get_priority(tokens[i].type) != UNARY_OPERATOR) {
      if (first || get_priority(tokens[i].type) <= get_priority(tokens[split].type)) {
        split = i;
        first = false;
      }
    }
  }
  return split;
}



