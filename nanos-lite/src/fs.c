#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t file_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENTS, FD_DISPINFO,FD_FB};

static size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

static size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]    = {"stdin",          0, 0, invalid_read,  invalid_write, 0},
  [FD_STDOUT]   = {"stdout",         0, 0, invalid_read,  serial_write,  0},
  [FD_STDERR]   = {"stderr",         0, 0, invalid_read,  serial_write,  0},
  [FD_EVENTS]   = {"/dev/events",    0, 0, events_read,   invalid_write, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write, 0},
  [FD_FB]       = {"/dev/fb",        0, 0, invalid_read,  fb_write,      0},
#include "files.h"
};

#define FD_END (sizeof(file_table) / sizeof(Finfo))

void init_fs() {
  file_table[FD_FB].size = io_read(AM_GPU_CONFIG).vmemsz;
}

int fs_open(const char *pathname, int flags, int mode) {
  int fd = -1;
  int i;
  for (i = 0; i < FD_END; i++) {
    if (strcmp(file_table[i].name, pathname) == 0) {
      fd = i;
    }
  }
  assert(fd != -1);
  file_table[fd].file_offset = 0;
  return fd;
}

size_t ramdisk_read(void *buf, size_t offset, size_t len);

size_t fs_read(int fd, void *buf, size_t len) {
  int left;
  if (file_table[fd].read == NULL) {
    left = file_table[fd].size - file_table[fd].file_offset;
    len = left < len ? left : len;
    len = ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].file_offset, len);
  } else {
    len = file_table[fd].read(buf, file_table[fd].file_offset, len);
  }
  file_table[fd].file_offset += len;
  return len;
}

size_t ramdisk_write(const void *buf, size_t offset, size_t len);

size_t fs_write(int fd, const void *buf, size_t len) {
  int left;
  if (file_table[fd].write == NULL) {
    left = file_table[fd].size - file_table[fd].file_offset;
    len = left < len ? left : len;
    len = ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].file_offset, len);
  } else {
    len = file_table[fd].write(buf, file_table[fd].file_offset, len);
  }
  file_table[fd].file_offset += len;
  return len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  switch (whence) {
    case SEEK_SET: file_table[fd].file_offset = offset;                        break;
    case SEEK_CUR: file_table[fd].file_offset += offset;                       break;
    case SEEK_END: file_table[fd].file_offset = file_table[fd].size + offset;  break;
    default: return -1;
  }
  return file_table[fd].file_offset;
}

int fs_close(int fd) {
  file_table[fd].file_offset = 0;
  return 0;
}

const char *fs_name(int fd) {
  return file_table[fd].name;
}
