#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  static char buf[64] = {0};
  while (!NDL_PollEvent(buf, sizeof(buf))) ;
  if (buf[1] == 'd') {
    event->type = SDL_KEYDOWN;
  }
  if (buf[1] == 'u') {
    event->type = SDL_KEYUP;
  }
  int i;
  for (i = 0; i < sizeof(keyname); i++) {
    if (strcmp(buf + 3, keyname[i]) == 0) {
      event->key.keysym.sym = i;
      return 1;
    }
  }
  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  assert(0);
  return NULL;
}
