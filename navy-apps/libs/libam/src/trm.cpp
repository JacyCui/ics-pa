#include <stdio.h>
#include <stdlib.h>
#include <am.h>
#include <klib-macros.h>

#define HEAP_SIZE 0x1000000

static char heap_start[HEAP_SIZE];

Area heap = RANGE(heap_start, heap_start + HEAP_SIZE);

void putch(char ch) {
    putchar(ch);
}

void halt(int code) {
    exit(code);
}
