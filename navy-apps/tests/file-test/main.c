#include <stdio.h>
#include <assert.h>

int main() {
  printf("Testing open ...\n");

  FILE *fp = fopen("/share/files/num", "r+");
  assert(fp);

  printf("Testing seek-end ...\n");

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  assert(size == 5000);

  printf("Testing seek-set and read ...\n");

  fseek(fp, 500 * 5, SEEK_SET);
  int i, n;
  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }

  printf("Testing seek-set and write ...\n");

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fprintf(fp, "%4d\n", i + 1 + 1000);
  }

  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1 + 1000);
  }

  printf("Testing close ...\n");

  fclose(fp);

  printf("PASS!!!\n");

  return 0;
}
