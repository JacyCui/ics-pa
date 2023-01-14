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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char *expr;
  word_t value;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

static WP *new_wp(bool *success);
static void free_wp(WP *wp);

void add_wp(char *e, bool *success) {
  WP *wp = new_wp(success);
  if (!*success) {
    printf("%sNo free watch points.%s\n", ANSI_FG_RED, ANSI_NONE);
    return;
  }
  wp->value = expr(e, success);
  if (!*success) {
    printf("%sInvalid Expression %s!%s\n", ANSI_FG_RED, e, ANSI_NONE);
    free_wp(wp);
    return;
  }
  wp->expr = (char *)malloc(strlen(e) + 1);
  strcpy(wp->expr, e);
  printf("%sWatch point %d with expression %s = %#08x is set.%s\n", ANSI_FG_GREEN, wp->NO, wp->expr, wp->value, ANSI_NONE);
}

void update_wp() {
  WP *p = head;
  word_t new_value;
  bool success = true;
  while (p != NULL) {
    new_value = expr(p->expr, &success);
    if (!success) {
      printf("%sInvalid expression %s in watch point %d!%s\n", ANSI_FG_RED, p->expr, p->NO, ANSI_NONE);
    } else {
      if (new_value != p->value) {
        printf("%sWatch point %d with expression %s changes from %#08x to %#08x.%s\n",
          ANSI_FG_YELLOW, p->NO, p->expr, p->value, new_value, ANSI_NONE);
        p->value = new_value;
        if (nemu_state.state == NEMU_RUNNING) {
          nemu_state.state = NEMU_STOP;
        }
      }
    }
    p = p->next;
  }
}

void delete_wp(int NO, bool *success) {
  WP *p = head;
  while (p != NULL) {
    if (p->NO == NO) {
      free(p->expr);
      free_wp(p);
      printf("Watch point numbered %d is deleted.\n", NO);
      return;
    }
    p = p->next;
  }
  printf("Watch point numbered %d not found.\n", NO);
  *success = false;
}

void clear_wp_pool() {
  WP *p = head;
  WP *temp;
  while (p != NULL) {
    temp = p;
    p = p->next;
    free(temp->expr);
    free_wp(temp);
  }
}

void display_wp() {
#ifdef CONFIG_WATCHPOINT
  WP *p = head;
  if (p == NULL) {
#endif
    printf("No watch points.\n");
#ifdef CONFIG_WATCHPOINT
    return;
  }
  printf("Watch Points\n-----------------------------------------------------------------------------------\n");
  printf("%-10s%-20s%-20s%-20s%-20s\n", "No", "Expr", "Value-Hexdecimal", "Value-Unsigned", "Value-Signed");
  printf("-----------------------------------------------------------------------------------\n");
  while (p != NULL) {
    printf("%-10d%-20s%#-20x%-20u%-20d\n", p->NO, p->expr, p->value, p->value, p->value);
    p = p->next;
  }
  printf("-----------------------------------------------------------------------------------\n");
#endif
}


static WP *new_wp(bool *success) {
  if (free_ == NULL) {
    *success = false;
    return NULL;
  }
  if (head == NULL) {
    head = free_;
    free_ = free_->next;
    head->next = NULL;
    return head;
  }
  WP *p = free_;
  free_ = free_->next;
  p->next = head;
  head = p;
  return head;
}

static void free_wp(WP *wp) {
  if (wp == head) {
    head = head->next;
    wp->next = free_;
    free_ = wp;
    return;
  }
  WP *prev, *cur;
  prev = head;
  cur = head->next;
  while (cur != NULL) {
    if (cur == wp) {
      prev->next = cur->next;
      cur->next = free_;
      free_ = cur;
      return;
    }
    cur = cur->next;
    prev = prev->next;
  }
}

