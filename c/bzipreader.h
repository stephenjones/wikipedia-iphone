/*
   Do stuff to bzip2 files. Originally based on bzip2recover, which
   is part of the standard bzip2 distribution.
   Author: Patrick Collison <patrick@collison.ie>
   Original author: Julian Seward <jseward@bzip.org>
*/

/* Caveat: I've a feeling this won't work unmodified on big-endian architectures */

#ifndef __BZIPREADER_H__
#define __BZIPREADER_H__

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <bzlib.h>
#include "debug.h"
#include "safe.h"

#define bytes(x) ((x) >> 3)

#define BLOCK_HEADER_HI  0x00003141UL
#define BLOCK_HEADER_LO  0x59265359UL

#define BLOCK_ENDMARK_HI 0x00001772UL
#define BLOCK_ENDMARK_LO 0x45385090UL

#define BIT_EOF 2

/* Increase if necessary.  However, a .bz2 file with > 50000 blocks
   would have an uncompressed size of at least 40GB, so the chances
   are low you'll need to up this. */
#define BZ_MAX_HANDLED_BLOCKS 500000

/* Needs to be >3.7kb apparently */
#define BZ_MAX_BLOCK (5 * 900 * 1024)

#define BZ_MAX_FILENAME 2000

/* Header bytes */

#define BZ_HDR_B 0x42                         /* 'B' */
#define BZ_HDR_Z 0x5a                         /* 'Z' */
#define BZ_HDR_h 0x68                         /* 'h' */
#define BZ_HDR_0 0x30                         /* '0' */

typedef struct {
  FILE *handle;
  int32_t  buffer;
  int32_t  buffLive;
  char   mode;
} BitStream;

typedef struct {
  char  *buff;
  int32_t  buffer;
  int32_t  buffLive;
  uint64_t pos;
  int32_t crc; /* logically, this doesn't belong here, but RubyInline... */
} BitBuffer;

FILE *xfopen(const char *path, const char *mode);
void xfclose(FILE *fp);

BitBuffer *bbOfSize(uint64_t size);
void bbClose(BitBuffer *bb);

int computeBoundaries(FILE *inFile);
int decompressBlock(char *src, uint32_t srcLen, char *dest, uint32_t *destLen);

uint64_t readBlock(FILE *inFile, uint64_t bitOffset, BitBuffer *bbOut);
uint64_t fixedOffset(uint64_t offset);

#endif
