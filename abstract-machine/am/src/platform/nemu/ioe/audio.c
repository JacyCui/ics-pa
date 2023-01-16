#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)
#define AUDIO_FRONT_ADDR     (AUDIO_ADDR + 0x18)
#define AUDIO_REAR_ADDR      (AUDIO_ADDR + 0x1c)

static int bufsize;

void __am_audio_init() {
  outl(AUDIO_INIT_ADDR, 0);
  bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = bufsize;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  char *p = (char *)ctl->buf.start;
  uint32_t count;
  uint32_t rear;
  while (p != (char *)ctl->buf.end) {
    count = inl(AUDIO_COUNT_ADDR);
    if (count < bufsize) {
      rear = inl(AUDIO_REAR_ADDR);
      outb(AUDIO_SBUF_ADDR + rear, *p);
      outl(AUDIO_COUNT_ADDR, count + 1);
      outl(AUDIO_REAR_ADDR, (rear + 1) % bufsize);
      p++;
    }
  }
}
