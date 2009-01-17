#include <stdio.h>
#include "lsearcher.h"
#include "bzipreader.h"

#define START_HEADING 1
#define START_TEXT 2
#define END_TEXT 3

typedef struct {
  char *text;
  int block;
} wp_article;

typedef struct {
  FILE *fp;
  uint64_t *block_map;
  lindex index;
} wp_dump;
