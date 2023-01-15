#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  const char *eos = s;
  while (*eos++);
  return (size_t)(eos - s - 1);
}

char *strcpy(char *dst, const char *src) {
  char *cp = dst;
  while ((*cp++ = *src++));
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *start = dst;
  while (n && (*dst++ = *src++)) n--;
  if (n) while (--n) *dst++ = '\0';
  return start;
}


char *strcat(char *dst, const char *src) {
  char *cp = dst;
  while (*cp) cp++;
  while ((*cp++ = *src++));
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s1 == *s2) ++s1, ++s2;
  return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  if (!n) return 0;
  while (--n && *s1 && *s1 == *s2) ++s1, ++s2;
  return *(unsigned char *)s1 - *(unsigned char *)s2;
}

void *memset(void *s, int c, size_t n) {
  char *pb = (char *)s;
  char *pbend = pb + n;
  while (pb != pbend) *pb++ = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  void * ret = dst;
  if (dst <= src || (char *)dst >= ((char *)src + n)) {
    // Non-overlapping buffers; copy from lower addresses to higher addresses
    while (n--) {
      *(char *)dst = *(char *)src;
      dst = (char *)dst + 1;
      src = (char *)src + 1;
    }
  } else {
    // Overlapping buffers; copy from higher addresses to lower addresses
    dst = (char *)dst + n - 1;
    src = (char *)src + n - 1;
    while (n--) {
      *(char *)dst = *(char *)src;
      dst = (char *)dst - 1;
      src = (char *)src - 1;
    }
  }
  return ret;
}

void *memcpy(void *out, const void *in, size_t n) {
  const char *s = (const char *)in;
  const char *end = s + n;
  char *d = (char *)out;
  while (s != end) *d++ = *s++;
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  if (!n) return 0;
  while (--n && *(char *)s1 == *(char *)s2) {
    s1 = (char *)s1 + 1;
    s2 = (char *)s2 + 1;
  }
  return *(unsigned char *)s1 - *(unsigned char *)s2;
}

#endif
