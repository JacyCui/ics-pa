#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <am.h>
#include <NDL.h>

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

#define KEY_NUM (sizeof(keyname) / sizeof(keyname[0]))

static void __am_input_keybrd(AM_INPUT_KEYBRD_T * kbd) {
  static char buf[64];
  if (!NDL_PollEvent(buf, 64)) {
    kbd->keycode = AM_KEY_NONE;
    kbd->keydown = false;
    return;
  }
  int i;
  for (i = 0; i < KEY_NUM; i++) {
    if (strcmp(buf + 3, keyname[i]) == 0) {
      break;
    }
  }
  kbd->keydown = (buf[1] == 'd');
  kbd->keycode = i;
}

static void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}

static void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  uptime->us = ((uint64_t)tv.tv_sec << 32) + (uint64_t)tv.tv_usec;
}

static int w = 0, h = 0;
static uint32_t *fb;

static void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  if (w == 0 && h == 0) {
    NDL_OpenCanvas(&w, &h);
    fb = (uint32_t *)malloc(w * h * sizeof(uint32_t));
  }
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = w * h
  };
}

static void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  uint32_t *pix = (uint32_t *)ctl->pixels;
  int i, j;
  for (j = 0; j < ctl->h; j++) {
    for (i = 0; i < ctl->w; i++) {
      fb[(ctl->y + j) * w + (ctl->x + i)] = pix[j * ctl->w + i];
    }
  }
  if (ctl->sync) {
    NDL_DrawRect(fb, 0, 0, w, h);
  }
}

static void __am_timer_config(AM_TIMER_CONFIG_T *cfg) { cfg->present = true; cfg->has_rtc = true; }
static void __am_input_config(AM_INPUT_CONFIG_T *cfg) { cfg->present = true;  }
static void __am_uart_config(AM_UART_CONFIG_T *cfg)   { cfg->present = false; }
static void __am_net_config(AM_NET_CONFIG_T *cfg)     { cfg->present = false; }
static void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) { cfg->present = false; }
static void __am_disk_config(AM_DISK_CONFIG_T *cfg)   { cfg->present = false; } 

typedef void (*handler_t)(void *buf);
static void *lut[128] = {
  [AM_TIMER_CONFIG] = __am_timer_config,
  [AM_TIMER_RTC   ] = __am_timer_rtc,
  [AM_TIMER_UPTIME] = __am_timer_uptime,
  [AM_INPUT_CONFIG] = __am_input_config,
  [AM_INPUT_KEYBRD] = __am_input_keybrd,
  [AM_GPU_CONFIG  ] = __am_gpu_config,
  [AM_GPU_FBDRAW  ] = __am_gpu_fbdraw,
  [AM_UART_CONFIG ] = __am_uart_config,
  [AM_AUDIO_CONFIG] = __am_audio_config,
  [AM_DISK_CONFIG ] = __am_disk_config,
  [AM_NET_CONFIG  ] = __am_net_config,
};

static void fail(void *buf) { 
  printf("Access unhandled register!\n");
  assert(0);
}

void ioe_read (int reg, void *buf) { ((handler_t)lut[reg])(buf); }
void ioe_write(int reg, void *buf) { ((handler_t)lut[reg])(buf); }

bool ioe_init() {
  for (int i = 0; i < 128; i++)
    if (!lut[i]) lut[i] = fail;
  NDL_Init(0);
  return true;
}


