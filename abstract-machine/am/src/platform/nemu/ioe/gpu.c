#include <am.h>
#include <klib.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

static int w, h;

void __am_gpu_init() {
  w = (inl(VGACTL_ADDR) >> 16) & 0xffff;
  h = inl(VGACTL_ADDR) & 0xffff;
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = w * h
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  // get frame buffer
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t *pix = (uint32_t *)ctl->pixels;
  int i, j;
  for (j = 0; j < ctl->h; j++) {
    for (i = 0; i < ctl->w; i++) {
      fb[(ctl->y + j) * w + (ctl->x + i)] = pix[j * ctl->w + i];
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_memcpy(AM_GPU_MEMCPY_T *mcp) {
  memcpy((uint32_t *)FB_ADDR + mcp->dest, mcp->src, mcp->size);
  outl(SYNC_ADDR, 1);
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
