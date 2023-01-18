#include <fs.h>

// typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
// typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

enum {MODE_READ};

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t file_offset;
  // ReadFn read;
  // WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

// size_t invalid_read(void *buf, size_t offset, size_t len) {
//   panic("should not reach here");
//   return 0;
// }

// size_t invalid_write(const void *buf, size_t offset, size_t len) {
//   panic("should not reach here");
//   return 0;
// }

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0/*, invalid_read, invalid_write*/},
  [FD_STDOUT] = {"stdout", 0, 0/*, invalid_read, invalid_write*/},
  [FD_STDERR] = {"stderr", 0, 0/*, invalid_read, invalid_write*/},
#include "files.h"
};

static int FD_END;

void init_fs() {
  // TODO: initialize the size of /dev/fb
  FD_END = sizeof(file_table) / sizeof(Finfo);
}

int fs_open(const char *pathname, int flags, int mode) {
  int fd = -1;
  int i;
  for (i = FD_FB; i < FD_END; i++) {
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
  if (fd < FD_FB) {
    return 0;
  }
  size_t left = file_table[fd].size - file_table[fd].file_offset;
  len = len <=  left ? len : left;
  if ((int)len <= 0) {
    return 0;
  }
  len = ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].file_offset, len);
  file_table[fd].file_offset += len;
  return len;
}

size_t ramdisk_write(const void *buf, size_t offset, size_t len);

size_t fs_write(int fd, const void *buf, size_t len) {
  if (fd < FD_FB) {
    putnstr((const char *)buf, len);
    return len;
  }
  size_t left = file_table[fd].size - file_table[fd].file_offset;
  len = len <=  left ? len : left;
  if ((int)len == 0) {
    return 0;
  }
  len = ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].file_offset, len);
  file_table[fd].file_offset += len;
  return len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  if (fd < FD_FB) {
    return -1;
  }
  switch (whence) {
    case SEEK_SET: file_table[fd].file_offset = offset;                             break;
    case SEEK_CUR: file_table[fd].file_offset += (int)offset;                       break;
    case SEEK_END: file_table[fd].file_offset = file_table[fd].size + (int)offset;  break;
    default: return -1;
  }
  return file_table[fd].file_offset;
}

int fs_close(int fd) {
  if (fd < FD_FB) {
    return 0;
  }
  file_table[fd].file_offset = 0;
  return 0;
}

const char *fs_name(int fd) {
  return file_table[fd].name;
}
