/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  reg_front,
  reg_rear,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

#ifdef CONFIG_AUDIO_PLAY_SOUND
#include <SDL2/SDL.h>

static void play_sound(void *userdata, uint8_t *stream, int len) {
  int i = 0;
  while (i < len && audio_base[reg_count] > 0) {
    stream[i] = sbuf[audio_base[reg_front]];
    audio_base[reg_front] = (audio_base[reg_front] + 1) % audio_base[reg_sbuf_size];
    audio_base[reg_count]--;
    i++;
  }
  if (i < len) {
    memset(stream + i, 0, len - i);
  }
}

static void init_sound() {
  SDL_AudioSpec s = {};
  s.format = AUDIO_S16SYS;
  s.userdata = NULL;
  s.freq = audio_base[reg_freq];
  s.channels = (uint8_t)(audio_base[reg_channels] & 0xff);
  s.samples = (uint16_t)(audio_base[reg_samples] & 0xffff);
  s.size = audio_base[reg_sbuf_size];
  s.callback = play_sound;
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  SDL_OpenAudio(&s, NULL);
  SDL_PauseAudio(0);
}

#endif

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
#ifdef CONFIG_AUDIO_PLAY_SOUND
  if (is_write && audio_base[reg_init]) {
    init_sound();
    audio_base[reg_init] = 0;
  }

#endif
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);

  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
  audio_base[reg_front] = 0;
  audio_base[reg_rear] = 0;
  audio_base[reg_count] = 0;
}
