#include <common.h>
#include "syscall.h"

static int write_handler(int fd, const void *buf, size_t count);
static int brk_handler(intptr_t increment);

void do_syscall(Context *c) {
  // uintptr_t a[4];
  // a[0] = c->GPR1;

  switch (c->GPR1) {
    case SYS_exit: 
#ifdef CONFIG_STRACE
      printf("%sSTRACE -> SYS_exit: code = %d%s\n", ANSI_FG_YELLOW, c->GPR2, ANSI_NONE);
#endif
      halt(c->GPR2);
      break;
    case SYS_yield:
#ifdef CONFIG_STRACE
      printf("%sSTRACE -> SYS_yield%s\n", ANSI_FG_YELLOW, ANSI_NONE);
#endif
      yield();
      break;
    case SYS_write: c->GPRx = write_handler(c->GPR2, (const void *)c->GPR3, c->GPR4); break;
    case SYS_brk: c->GPRx = brk_handler(c->GPR2); break;
    default: panic("Unhandled syscall ID = %d", c->GPR1);
  }
}

static int write_handler(int fd, const void *buf, size_t count) {
#ifdef CONFIG_STRACE
  printf("%sSTRACE -> SYS_write: fd = %d, buf = %#08x, count = %u%s\n", ANSI_FG_YELLOW, fd, buf, count, ANSI_NONE);
#endif
  if (fd == 1 || fd == 2) {
    putnstr((const char *)buf, count);
    return count;
  }
  return -1;
}

static int brk_handler(intptr_t increment) {
#ifdef CONFIG_STRACE
  printf("%sSTRACE -> SYS_brk: increment = %d%s\n", ANSI_FG_YELLOW, increment, ANSI_NONE);
#endif
  return 0;
}

