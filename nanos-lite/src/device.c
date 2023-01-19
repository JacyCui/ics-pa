#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  putnstr((const char *)buf, len);
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  if (!io_read(AM_INPUT_CONFIG).present) {
    return 0;
  }
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    return 0;
  }
  return snprintf(buf, len, "%s %s\n", ev.keydown ? "kd" : "ku", keyname[ev.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  static char info[128] = {0};
  if (strlen(info) == 0) {
    sprintf(info, "WIDTH : %d\nHEIGHT : %d\n", io_read(AM_GPU_CONFIG).width, io_read(AM_GPU_CONFIG).height);
  }
  int left = strlen(info) - offset;
  len = left < len ? left : len;
  memcpy(buf, info + offset, len);
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  io_write(AM_GPU_MEMCPY, offset, buf, len);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
