#include <common.h>
#include <device/map.h>

#ifdef CONFIG_DTRACE

enum {DREAD, DWRITE};

typedef struct DTrace {
  const char *name;
  vaddr_t pc;
  int type;
  struct DTrace *next;
} DTrace;

static struct DTrace *head = NULL, *tail = NULL;

void dtrace_add(IOMap *map, vaddr_t pc, int type) {
  if (map == NULL) {
    return;
  }
  DTrace *p = (DTrace *)malloc(sizeof(DTrace));
  p->name = map->name;
  p->pc = pc;
  p->type = type;
  p->next = NULL;
  if (head == NULL) {
    head = p;
    tail = p;
    return;
  }
  tail->next = p;
  tail = tail->next;
}

void dtrace_display() {
  DTrace *p = head;
  printf("%sDevice trace:%s\n", ANSI_FG_YELLOW, ANSI_NONE);
  while (p != NULL) {
    if (p->type == DREAD) {
      printf("Read from %s when pc = %#08x.\n", p->name, p->pc);
    }
    if (p->type == DWRITE) {
      printf("Write to %s when pc = %#08x.\n", p->name, p->pc);
    }
    p = p->next;
  }
}

void dtrace_clear() {
  Log("Clearing device trace buffer ...");
  DTrace *p = head;
  DTrace *tmp;
  while (p != NULL) {
    tmp = p;
    p = p->next;
    free(tmp);
  }
  head = NULL;
  tail = NULL;
}

#endif
