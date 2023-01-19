#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int x0 = 0, y0 = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", 0, 0);
  len = read(fd, buf, len);
  close(fd);
  return len > 0 ? 1 : 0;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
    return;
  }
  FILE *fp = fopen("/proc/dispinfo", "r");
  char buf[128];
  int num;
  while(fscanf(fp, "%[^: ]%*[ :]%d\n", buf, &num) != EOF) {
    if (strcmp(buf, "WIDTH") == 0) {
      screen_w = num;
    }
    if (strcmp(buf, "HEIGHT") == 0) {
      screen_h = num;
    }
  }
  fclose(fp);

  if (*w == 0 && *h == 0) {
    *w = screen_w;
    *h = screen_h;
  }
  x0 = (screen_w - *w) / 2;
  y0 = (screen_h - *h) / 2;
  fbdev = open("/dev/fb", 0, 0);
}

static inline size_t calc_offset(int x, int y) {
  return (y0 + y) * screen_w + x0 + x;
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int i, offset;
  for (i = 0; i < h; i++) {
    offset = calc_offset(x, y + i);
    lseek(fbdev, offset, SEEK_SET);
    write(fbdev, pixels + i * w, w * sizeof(uint32_t));
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
  close(fbdev);
}
