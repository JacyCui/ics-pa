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
#include <cpu/cpu.h>
#include <memory/vaddr.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"


static int is_batch_mode = false;

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}


static int cmd_help(char *args);
static int cmd_c(char *args);
static int cmd_q(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
#ifdef CONFIG_WATCHPOINT
static int cmd_w(char *args);
static int cmd_d(char *args);
#endif
#ifdef CONFIG_FTRACE
static int cmd_bt(char *args);
#endif

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  /* TODO: Add more commands */
  { "si", "Single step forward for N(1 by default) instruction", cmd_si },
  { "info", "Display information of registers(r) or watch points(w) or symbol tables(s)", cmd_info },
  { "x", "Print N words of memory started from Expr", cmd_x },
  { "p", "Evaluate the given expression Expr", cmd_p },
#ifdef CONFIG_WATCHPOINT
  { "w", "Set a watch point for Expr", cmd_w },
  { "d", "Delete the watch point numbered by N", cmd_d },
#endif
#ifdef CONFIG_FTRACE
  { "bt", "Display function call stack", cmd_bt },
#endif
};

#define NR_CMD ARRLEN(cmd_table)


void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("%sUnknown command '%s'%s\n", ANSI_FG_RED, cmd, ANSI_NONE); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  IFDEF(CONFIG_WATCHPOINT, init_wp_pool());
}


// ---------------------------
// Implementation of all monitor commands

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s%s - %s%s\n", ANSI_FG_GREEN, cmd_table[i].name, cmd_table[i].description, ANSI_NONE);
        return 0;
      }
    }
    printf("%sUnknown command '%s'%s\n", ANSI_FG_RED, arg, ANSI_NONE);
  }
  return 0;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    cpu_exec(1);
    return 0;
  }
  int64_t n = strtoll(arg, NULL, 0);
  if (n > 0) {
    cpu_exec(n);
  } else {
    printf("%s%s%s\n", ANSI_FG_RED, "Usage: si [N (N > 0)]", ANSI_NONE);
  }
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("%s%s%s", ANSI_FG_RED, "Usage: info r -> register\n       info w -> watch points\n       info s -> symbol table\n", ANSI_NONE);
    return 0;
  }
  switch (arg[0]) {
    case 'r': isa_reg_display(); break;
    case 'w': display_wp(); break;
    case 's': isa_display_symtab(); break;
    default: printf("%s%s%s\n", ANSI_FG_RED, "Usage: info r -> register\n       info w -> watch points\n       info s -> symbol table\n", ANSI_NONE);
  }
  return 0;
}

static int cmd_x(char *args) {
  char *arg1 = strtok(NULL, " ");
  if (arg1 == NULL) {
    printf("%s%s%s\n", ANSI_FG_RED, "Usage: x N Expr (eg. x 10 $sp, N > 0)", ANSI_NONE);
    return 0;
  }
  char *arg2 = strtok(NULL, " ");
  if (arg2 == NULL) {
    printf("%s%s%s\n", ANSI_FG_RED, "Usage: x N Expr (eg. x 10 $sp, N > 0)", ANSI_NONE);
    return 0;
  }
  int n = strtol(arg1, NULL, 0);
  int i;
  vaddr_t vaddr;
  char *e;
  bool success = true;
  if (n > 0) {
    e = args + strlen(arg1) + 1;
    vaddr = expr(e, &success);
    if (!success) {
      printf("%sInvalid Expression %s!%s\n", ANSI_FG_RED, e, ANSI_NONE);
      return 0;
    }
    for (i = 0; i < n; i++) {
      printf("%#010x: %#010x\n", vaddr + (i << 2), vaddr_read(vaddr + (i << 2), 4));
    }
  } else {
    printf("%s%s%s\n", ANSI_FG_RED, "Usage: x N Expr (eg. x 10 $sp, N > 0)", ANSI_NONE);
  }
  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL) {
    printf("%s%s%s\n", ANSI_FG_RED, "Usage: p Expr", ANSI_NONE);
    return 0;
  }
  bool success = true;
  word_t res = expr(args, &success);
  if (!success) {
    printf("%sInvalid Expression %s!%s\n", ANSI_FG_RED, args, ANSI_NONE);
    return 0;
  }
  printf("%sValid Expr: %s%s\n", ANSI_FG_GREEN, args, ANSI_NONE);
  printf("Value:  %-20s%-20s%-20s\n", "hexdecimal", "unsigned decimal", "signed decimal");
  printf("        %#-20x%-20u%-20d\n", res, res, res);
  return 0;
}

#ifdef CONFIG_WATCHPOINT
static int cmd_w(char *args) {
  if (args == NULL) {
    printf("%s%s%s\n", ANSI_FG_RED, "Usage: w Expr", ANSI_NONE);
    return 0;
  }
  bool success = true;
  add_wp(args, &success);
  if (!success) {
    printf("%sFail to set watch point for %s!%s\n", ANSI_FG_RED, args, ANSI_NONE);
  }
  return 0;
}

static int cmd_d(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("%s%s%s\n", ANSI_FG_RED, "Usage: d N", ANSI_NONE);
    return 0;
  }
  int n = strtol(arg, NULL, 0);
  bool success = true;
  delete_wp(n, &success);
  if (!success) {
    printf("%sFail to delete watch point numbered %d!%s\n", ANSI_FG_RED, n, ANSI_NONE);
  }
  return 0;
}
#endif

#ifdef CONFIG_FTRACE
static int cmd_bt(char *args) {
  IFDEF(CONFIG_FTRACE, display_backtrace());
  return 0;
}
#endif
