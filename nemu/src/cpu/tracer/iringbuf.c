#include <common.h>

#ifdef CONFIG_IRINGBUF

#define IRINGBUF_LEN 10

static char *iringbuf[IRINGBUF_LEN] = {0};
static int p = 0;

void iringbuf_add(const char *logbuf) {
  if (iringbuf[p] != NULL) {
    free(iringbuf[p]);
  }
  iringbuf[p] = (char *)malloc((strlen(logbuf) + 1) * sizeof(char));
  strcpy(iringbuf[p], logbuf);
  p = (p + 1) % IRINGBUF_LEN;
}

void iringbuf_display() {
  printf("%sInstruction ring buffer:%s\n... ...\n", ANSI_FG_YELLOW, ANSI_NONE);
  int i = p;
  do {
    if (iringbuf[i] != NULL) {
      puts(iringbuf[i]);
    }
    i = (i + 1) % IRINGBUF_LEN;
  } while (i != p);
}

void iringbuf_clear() {
  Log("Clearing instruction ring buffer ...");
  int i = p;
  do {
    if (iringbuf[i] != NULL) {
      free(iringbuf[i]);
      iringbuf[i] = NULL;
    }
    i = (i + 1) % IRINGBUF_LEN;
  } while (i != p);
  p = 0;
}

#endif
