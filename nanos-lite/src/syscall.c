#include <common.h>
#include <fs.h>
#include <sys/time.h>
#include "syscall.h"

static void exit_handler(int code);
static void yield_handler();
static int open_handler(const char *path, int flags, int mode);
static size_t read_handler(int fd, void *buf, size_t count);
static size_t write_handler(int fd, const void *buf, size_t count);
static int close_handler(int fd);
static size_t lseek_handler(int fd, size_t offset, int whence);
static int brk_handler(intptr_t increment);
static int gettimeofday_handler(struct timeval *tv, void* tz);



void do_syscall(Context *c) {
  // uintptr_t a[4];
  // a[0] = c->GPR1;

  switch (c->GPR1) {
    case SYS_exit:          exit_handler(c->GPR2);                                                      break;
    case SYS_yield:         yield_handler();                                                            break;
    case SYS_open:          c->GPRx = open_handler((const char *)c->GPR2, c->GPR3, c->GPR4);            break;
    case SYS_read:          c->GPRx = read_handler(c->GPR2, (void *)c->GPR3, c->GPR4);                  break;
    case SYS_write:         c->GPRx = write_handler(c->GPR2, (const void *)c->GPR3, c->GPR4);           break;
    case SYS_close:         c->GPRx = close_handler(c->GPR2);                                           break;
    case SYS_lseek:         c->GPRx = lseek_handler(c->GPR2, c->GPR3, c->GPR4);                         break;
    case SYS_brk:           c->GPRx = brk_handler(c->GPR2);                                             break;
    case SYS_gettimeofday:  c->GPRx = gettimeofday_handler((struct timeval *)c->GPR2, (void *)c->GPR3); break;
    default: panic("Unhandled syscall ID = %d", c->GPR1);
  }
}

/* handlers of all kinds of system calls */

static void exit_handler(int code) {
#ifdef CONFIG_STRACE
  printf("%sSTRACE -> SYS_exit: code = %d%s\n", ANSI_FG_YELLOW, code, ANSI_NONE);
#endif
  halt(code);
}

static void yield_handler() {
#ifdef CONFIG_STRACE
  printf("%sSTRACE -> SYS_yield%s\n", ANSI_FG_YELLOW, ANSI_NONE);
#endif
  yield();
}

static int open_handler(const char *path, int flags, int mode) {
#ifdef CONFIG_STRACE
  printf("%sSTRACE -> SYS_open: path = %s, flags = %#08x, mode = %#08x%s\n", ANSI_FG_YELLOW, path, flags, mode, ANSI_NONE);
#endif
  return fs_open(path, flags, mode);
}

static size_t read_handler(int fd, void *buf, size_t count) {
#ifdef CONFIG_STRACE
  printf("%sSTRACE -> SYS_read: fd = %d (path = %s), buf = %#08x, count = %u%s\n", ANSI_FG_YELLOW, fd, fs_name(fd), buf, count, ANSI_NONE);
#endif
  return fs_read(fd, buf, count);
}

static size_t write_handler(int fd, const void *buf, size_t count) {
#ifdef CONFIG_STRACE
  printf("%sSTRACE -> SYS_write: fd = %d (path = %s), buf = %#08x, count = %u%s\n", ANSI_FG_YELLOW, fd, fs_name(fd), buf, count, ANSI_NONE);
#endif
  return fs_write(fd, buf, count);
}

static int close_handler(int fd) {
#ifdef CONFIG_STRACE
  printf("%sSTRACE -> SYS_close: fd = %d (path = %s)%s\n", ANSI_FG_YELLOW, fd, fs_name(fd), ANSI_NONE);
#endif
  return fs_close(fd);
}

static size_t lseek_handler(int fd, size_t offset, int whence) {
#ifdef CONFIG_STRACE
  printf("%sSTRACE -> SYS_lseek: fd = %d (path = %s), offset = %u, count = %d%s\n", ANSI_FG_YELLOW, fd, fs_name(fd), offset, whence, ANSI_NONE);
#endif
  return fs_lseek(fd, offset, whence);
}

static int brk_handler(intptr_t increment) {
#ifdef CONFIG_STRACE
  printf("%sSTRACE -> SYS_brk: increment = %d%s\n", ANSI_FG_YELLOW, increment, ANSI_NONE);
#endif
  return 0;
}

static int gettimeofday_handler(struct timeval *tv, void* tz) {
  if (tv == NULL) {
    return -1;
  }
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  tv->tv_usec = us % 1000000;
  tv->tv_sec = us / 1000000;
  return 0;
}
