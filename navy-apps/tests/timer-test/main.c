#include <stdio.h>
#include <NDL.h>

int main() {
  NDL_Init(0);
  uint32_t t = NDL_GetTicks();
  while (1) {
    while (NDL_GetTicks() - t < 500) ;
    printf("0.5 second has passed.\n");
    t = NDL_GetTicks();
  }
  NDL_Quit();
  return 0;
}
