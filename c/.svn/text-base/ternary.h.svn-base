#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include "debug.h"
#include "safe.h"

#define PREFIX_LEN 40
#define MAXLINE 1024
#define INDEX_LEN (sizeof(struct node) * 1000 * 1000 * 5)

#define CMP(a, b) ((a) > (b) ? 1 : ((a) == (b) ? 0 : -1))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define NEXT_NODE(node, cmp) ((cmp) < 0 ? (node)->lt : \
			      ((cmp) > 0 ? (node)->gt : (node)->eq))

#define CMASK 1
#define BLOCK_MASK(b) ((b) << 1)
#define BLOCK_DEMASK(b) ((b) >> 1)

struct node {
  uint32_t block;
  uint32_t lt, eq, gt;
  char c;
};

struct index_hdr {
  uint32_t nodes;
  uint32_t node_size;
  char cmd[MAXLINE];
};

typedef char compact_str[13];
typedef bool resultf(char *, uint32_t);

struct cnode {
  uint32_t block;
  compact_str str;
};

#define tolower(c) ('A' <= (c) && (c) <= 'Z' ? (c) | 0x20 : (c))
