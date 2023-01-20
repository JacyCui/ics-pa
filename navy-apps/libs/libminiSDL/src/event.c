#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

#define KEY_NUM (sizeof(keyname) / sizeof(keyname[0]))

int SDL_PushEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

static uint8_t keystate[KEY_NUM] = {0};

int SDL_PollEvent(SDL_Event *ev) {
  static char buf[64] = {0};
  if (!NDL_PollEvent(buf, sizeof(buf))) {
    return 0;
  }
  int i;
  for (i = 0; i < KEY_NUM; i++) {
    if (strcmp(buf + 3, keyname[i]) == 0) {
      break;
    }
  }
  if (i == sizeof(keyname)) {
    return 0;
  }
  ev->key.keysym.sym = i;
  if (buf[1] == 'd') {
    ev->type = SDL_KEYDOWN;
    keystate[i] = 1;
  }
  if (buf[1] == 'u') {
    ev->type = SDL_KEYUP;
    keystate[i] = 0;
  }
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  static char buf[64] = {0};
  while (!NDL_PollEvent(buf, sizeof(buf))) ;
  int i;
  for (i = 0; i < KEY_NUM; i++) {
    if (strcmp(buf + 3, keyname[i]) == 0) {
      break;
    }
  }
  if (i == KEY_NUM) {
    return 0;
  }
  event->key.keysym.sym = i;
  if (buf[1] == 'd') {
    event->type = SDL_KEYDOWN;
    keystate[i] = 1;
  }
  if (buf[1] == 'u') {
    event->type = SDL_KEYUP;
    keystate[i] = 0;
  }
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  if (numkeys != NULL) {
    *numkeys = KEY_NUM;
  }
  return keystate;
}
