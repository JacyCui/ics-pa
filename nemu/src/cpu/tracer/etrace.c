#include <common.h>

#ifdef CONFIG_ETRACE

typedef struct ETrace {
  vaddr_t pc;
  word_t code;
  struct ETrace *next;
} ETrace;

static ETrace *head = NULL, *tail = NULL;

void etrace_add(vaddr_t pc, word_t code) {
  ETrace *p = (ETrace *)malloc(sizeof(ETrace));
  p->pc = pc;
  p->code = code;
  p->next = NULL;
  if (head == NULL) {
    head = p;
    tail = p;
    return;
  }
  tail->next = p;
  tail = tail->next;
}

void etrace_display() {
  ETrace *p = head;
  printf("%sException trace:%s\n", ANSI_FG_YELLOW, ANSI_NONE);
  while (p != NULL) {
    printf("Trigger exception of code %#08x when pc = %#08x.\n", p->code, p->pc);
    p = p->next;
  }
}

void etrace_clear() {
  Log("Clearing exception trace buffer ...");
  ETrace *p = head;
  ETrace *tmp;
  while (p != NULL) {
    tmp = p;
    p = p->next;
    free(tmp);
  }
  head = NULL;
  tail = NULL;
}

#endif
