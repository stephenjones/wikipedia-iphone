#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "debug.h"
#include "safe.h"

#define MAXLINE 1024
#define MAXBLOCK 20000

uint64_t store[MAXBLOCK];

uint64_t *load_block_map(char *file) {
  int nblocks;
  void *ptr;
  FILE *fp = xfopen(file, "r");

  fread(&nblocks, sizeof(nblocks), 1, fp);
  ptr = xmmap(NULL, nblocks * sizeof(uint64_t), PROT_READ, MAP_SHARED, fileno(fp), 0);

  return ptr + sizeof(nblocks);
}
   
#ifdef INCLUDE_MAIN
int main(int argc, char **argv) {
  char line[MAXLINE];
  int pos = 0;

  while(fgets(line, MAXLINE, stdin))
    store[pos++] = (uint64_t) strtoll(line, NULL, 10);

  FILE *bFile = xfopen(argv[1], "w");

  fwrite(&pos, sizeof(pos), 1, bFile);
  fwrite(store, sizeof(uint64_t), pos, bFile);
  fclose(bFile);

  return 0;
}
#endif
