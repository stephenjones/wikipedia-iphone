/*
   Do stuff to bzip2 files. Originally based on bzip2recover, which
   is part of the standard bzip2 distribution.
   Author: Patrick Collison <patrick@collison.ie>
   Original author: Julian Seward <jseward@bzip.org>
*/

#include "bzipreader.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

uint64_t bStart [BZ_MAX_HANDLED_BLOCKS];
uint64_t bEnd   [BZ_MAX_HANDLED_BLOCKS];
uint64_t rbStart[BZ_MAX_HANDLED_BLOCKS];
uint64_t rbEnd  [BZ_MAX_HANDLED_BLOCKS];

uint64_t bytesOut = 0;
uint64_t bytesIn  = 0;
 
void tooManyBlocks(int32_t max_handled_blocks)
{
  fatal("File appears to contain more than %d blocks, and cannot be handled.\n"
        "To fix, increase BZ_MAX_HANDLED_BLOCKS in %s, and recompile.",
        max_handled_blocks, __FILE__);
}

void bbputc(int32_t c, BitBuffer *bb) {
  bb->buff[(bb->pos)++] = c;
}

BitBuffer *bbOfSize(uint64_t size) {
  BitBuffer *bb = xalloc(sizeof(BitBuffer));
  bb->buff = xalloc(size);
  bb->buffer = 0;
  bb->buffLive = 0;
  bb->pos = 0;
  return bb;  
}

BitStream *bsOpenReadStream(FILE *stream)
{
  BitStream *bs = xalloc(sizeof(BitStream));
  bs->handle = stream;
  bs->buffer = 0;
  bs->buffLive = 0;
  bs->mode = 'r';
  return bs;
}

BitStream *bsOpenWriteStream(FILE *stream)
{
  BitStream *bs = xalloc(sizeof(BitStream));
  bs->handle = stream;
  bs->buffer = 0;
  bs->buffLive = 0;
  bs->mode = 'w';
  return bs;
}

void bbPutBit(BitBuffer *bb, int32_t bit)
{
  if(bb->buffLive == 8) {
     bbputc((uchar_t) bb->buffer, bb);
     bytesOut++;
     bb->buffLive = 1;
     bb->buffer = bit & 0x1;
  } else {
     bb->buffer = ((bb->buffer << 1) | (bit & 0x1));
     bb->buffLive++;
  };
}

void bsPutBit(BitStream *bs, int32_t bit)
{
  if(bs->buffLive == 8) {
    xputc((uchar_t) bs->buffer, bs->handle);
    bytesOut++;
    bs->buffLive = 1;
    bs->buffer = bit & 0x1;
  } else {
    bs->buffer = ((bs->buffer << 1) |(bit & 0x1));
    bs->buffLive++;
  };
}


/* Returns 0 or 1, or 2 to indicate EOF. */
int32_t bsGetBit(BitStream *bs)
{
  if(bs->buffLive > 0) {
    bs->buffLive --;
    return (((bs->buffer) >> (bs->buffLive)) & 0x1);
  } else {
    int32_t retVal = xgetc(bs->handle);
    if(retVal == EOF) return BIT_EOF;
    bs->buffLive = 7;
    bs->buffer = retVal;
    return (((bs->buffer) >> 7) & 0x1);
  }
}

void bsClose(BitStream *bs)
{
  if(bs->mode == 'w') {
    while(bs->buffLive < 8) {
       bs->buffLive++;
       bs->buffer <<= 1;
    };
    xputc((uchar_t)(bs->buffer), bs->handle);
    bytesOut++;
    xfflush(bs->handle);
  }
  xfclose(bs->handle);
  free(bs);
}

void bbClose(BitBuffer *bb)
{
  free(bb->buff);
  free(bb);
}

void bbPutUchar(BitBuffer *bb, uchar_t c) {
  int32_t i;
  for(i = 7; i >= 0; i--)
     bbPutBit(bb, (((uint32_t) c) >> i) & 0x1);  
}

void bbPutUint(BitBuffer *bb, uint32_t c)
{
  debug("writing %d as CRC", c);
  int32_t i;
  for(i = 31; i >= 0; i--)
    bbPutBit(bb, (c >> i) & 0x1);
}

int computeBoundariesUpto(FILE *inFile, int32_t upto) {  
  debug("starting");
  BitStream *bsIn;
  int32_t b, currBlock, rbCtr;
  uint64_t bitsRead, lastBitsRead;
    
  uint32_t buffHi, buffLo;
  
  debug("searching for block boundaries...");
  
  bitsRead = 0;
  lastBitsRead = 0;
  buffHi = buffLo = 0;
  currBlock = 0;
  bStart[currBlock] = 0;
  
  bsIn = bsOpenReadStream(inFile);
  
  rbCtr = 0;
  
  while(true) {
    b = bsGetBit(bsIn);
    bitsRead++;
     
    if(b == BIT_EOF) {
      if(bitsRead >= bStart[currBlock] && (bitsRead - bStart[currBlock]) >= 40) {
        fatal("Corrupted file"); /* I'm not actually certain that this is necessarily the case */
      } else
        currBlock--;
        
      break;
    }
     
    buffHi = (buffHi << 1) | (buffLo >> 31);
    buffLo = (buffLo << 1) | (b & 1);
  
    if (((buffHi & 0x0000ffff) == BLOCK_HEADER_HI && buffLo == BLOCK_HEADER_LO)
        /* beginning of a block */
        || 
       ((buffHi & 0x0000ffff) == BLOCK_ENDMARK_HI && buffLo == BLOCK_ENDMARK_LO)) {
        /* eof */
     
      if(bitsRead > 49) {
       bEnd[currBlock] = bitsRead - 49;
      } else {
       bEnd[currBlock] = 0;
      }
  
      if(currBlock > 0 && (bEnd[currBlock] - bStart[currBlock]) >= 130) {
      //         "%llu (0x%llx) to %llu (0x%llx). Size: %llu\n",
       debug("block %d:\n"                /* block num */
             "  [%llu (%llu)\n"           /* bit offset of block start (bytes) */
             "   %llu\n"                  /* bit offset of data of block start */
             "   %llu\n"                  /* bit offset of data of block end */
             "   %llu (%llu)]\n"          /* bit offset of block end (bytes) */
             "  D size: %llu (%llu)\n"    /* data  size (bytes) */
             "  B size: %llu (%llu)",     /* block size (bytes) */
             rbCtr+1,
             lastBitsRead, bytes(lastBitsRead),
             bStart[currBlock], 
             bEnd[currBlock],
             bitsRead, bytes(bitsRead),
             bEnd[currBlock] - bStart[currBlock],
             bytes(bEnd[currBlock] - bStart[currBlock]),
             bitsRead - lastBitsRead,
             bytes(bitsRead - lastBitsRead));
       rbStart[rbCtr] = bStart[currBlock];
       rbEnd[rbCtr] = bEnd[currBlock];
       lastBitsRead = bitsRead;
       rbCtr++;
      }
      
      if(upto >= 0 && currBlock > upto)
        return rbCtr;
  
      if(currBlock >= BZ_MAX_HANDLED_BLOCKS)
        tooManyBlocks(BZ_MAX_HANDLED_BLOCKS);
     
      currBlock++;
  
      bStart[currBlock] = bitsRead;
    }
  }
  
  bsClose(bsIn);
  
  if(rbCtr < 1)
    fatal("sorry, I couldn't find any block boundaries");
  
  return rbCtr;
}

int computeBoundaries(FILE *inFile)
{
  return computeBoundariesUpto(inFile, -1);
}

uint64_t fixedOffset(uint64_t offset) {
  return offset > 0 ? offset - 80 : 0;
}

void printBoundaries(int32_t numBlocks)
{
  int32_t i;
  for(i = 0; i < numBlocks; i++)
    printf("%d %llu\n", i, fixedOffset(rbStart[i]));
}

uint64_t readBlock(FILE *inFile, uint64_t bitOffset, BitBuffer *bbOut)
{
  BitStream *bsIn;
  int32_t b, wrBlock, subByteOffset;
  uint64_t bitsRead, i, headerEnd;
  uint32_t buffHi, buffLo;
  
  bsIn = bsOpenReadStream(inFile);
  
  debug("Seeking to %llu", bytes(bitOffset));
  fseeko(bsIn->handle, bytes(bitOffset), SEEK_SET);
  
  subByteOffset = bitOffset & 7;
  debug("subByteOffset: %d", subByteOffset);
  for(i = 0; i < subByteOffset; i++)
    bsGetBit(bsIn);

  bitsRead = 0;
  wrBlock = 0;
  bool inBlock = false;
  
  bbPutUchar(bbOut, BZ_HDR_B);    
  bbPutUchar(bbOut, BZ_HDR_Z);    
  bbPutUchar(bbOut, BZ_HDR_h);    
  bbPutUchar(bbOut, BZ_HDR_0 + 9);
  bbPutUchar(bbOut, 0x31); bbPutUchar(bbOut, 0x41);
  bbPutUchar(bbOut, 0x59); bbPutUchar(bbOut, 0x26);
  bbPutUchar(bbOut, 0x53); bbPutUchar(bbOut, 0x59);
  
  while(true) {
    b = bsGetBit(bsIn);
    bitsRead++;

    if(b == BIT_EOF)
     fatal("EOF encountered after %llu/%llu", bitsRead, bytes(bitsRead));
 
    buffHi = (buffHi << 1) | (buffLo >> 31);
    buffLo = (buffLo << 1) | (b & 1);

    if(bitsRead == headerEnd + 48) {
      bbOut->crc = (buffHi << 16) | (buffLo >> 16);        
      debug("CRC: %d (at pos %llu/%llu)", bbOut->crc, bbOut->pos, bitsRead);
    }
    
    if(inBlock)      
      bbPutBit(bbOut, b);
     
    if (((buffHi & 0x0000ffff) == BLOCK_HEADER_HI && buffLo == BLOCK_HEADER_LO)
       || 
       ((buffHi & 0x0000ffff) == BLOCK_ENDMARK_HI && buffLo == BLOCK_ENDMARK_LO)) {
      if(!inBlock) {
        debug("After %llu bits read, starting block", bitsRead);
        headerEnd = bitsRead;
        inBlock = true;
      } else {
        debug("buffLive: %d", bbOut->buffLive);
        bbOut->pos -= 6; /* we just read the header for the next block -- discard */
        
        /* Take account of byte boundaries -- get us back to state before we read header */
        bbOut->buffer = bbOut->buff[bbOut->pos] >> (8 - bbOut->buffLive);
    
        debug("Finishing block after %llu bits; writing to %llu; buffer: %d; live: %d",
              bitsRead, bbOut->pos, bbOut->buffer, bbOut->buffLive);
        bbPutUchar(bbOut, 0x17); bbPutUchar(bbOut, 0x72);
        bbPutUchar(bbOut, 0x45); bbPutUchar(bbOut, 0x38);
        bbPutUchar(bbOut, 0x50); bbPutUchar(bbOut, 0x90);
        bbPutUint(bbOut, bbOut->crc); bbPutUchar(bbOut, 0x10);
        
        return bitsRead + bitOffset;
      }
    }
  }
}

int decompressBlock(char *src, uint32_t srcLen, char *result, uint32_t *destLen) {  
  return BZ2_bzBuffToBuffDecompress(
    result,
    destLen,
    src,
    srcLen,
    0,
    debug ? 4 : 0);
}

#ifdef INCLUDE_MAIN
void usage(char **argv) {
  fprintf(stderr, "%s -f <file> [-l] [-b <bitoffset>] [-n <blocknumber>] [-d] [-x [-t <truncate>]]\n\n"
                  "  If -b is supplied, data that can be passed to the\n"
                  "  ordinary bzip2 utility will be written to stdout.\n\n"
                  "  If -l is supplied, the bit offsets for the file's\n"
                  "  blocks will be written to stdout.\n",
          argv[0]);
  exit(1);
}

int32_t main(int32_t argc, char **argv)
{
  BitBuffer *bb;
  int ch, whichBlock, truncate;
  uint64_t offset, end;
  bool listBlocks;
  bool useWhichBlock;
  bool doDecompress;
  char inFileName[BZ_MAX_FILENAME];

  whichBlock = -1;
  truncate = 0;
  useWhichBlock = false;
  listBlocks = false;
  doDecompress = false;
  inFileName[0] = 0;

  while ((ch = getopt(argc, argv, "dhxlb:f:n:t:")) != -1) {
    switch (ch) {
    case 'b':
      offset = strtoll(optarg, NULL, 10);
      break;
    case 'd':
      debug = true;
      break;
    case 'f':
      strcpy(inFileName, optarg);
      break;
    case 'l':
      listBlocks = true;
      break;
    case 'n':
      useWhichBlock = true;
      whichBlock = strtol(optarg, NULL, 10);
      break;
    case 't':
      truncate = strtol(optarg, NULL, 10);
      break;
    case 'x':
      doDecompress = true;
      break;
    case 'h':
    default:
      usage(argv);
    }
  }
  
  if(debug)
    debug("Debug enabled");
  
  if(strlen(inFileName) == 0)
    usage(argv);

  if(strlen(inFileName) >= BZ_MAX_FILENAME-20)
    fatal("Supplied filename is suspiciously (>= %zu chars) long", strlen(inFileName));

  if(listBlocks) {
    printBoundaries(computeBoundaries(xfopen(inFileName, "rb")));
  } else {
    if(useWhichBlock) {
      int max;
      fprintf(stderr, "WARNING: Using -n can be slow; must scan file up desired block\n");
      max = computeBoundariesUpto(xfopen(inFileName, "rb"), whichBlock);
      
      if(whichBlock > max)
        fatal("Invalid block specifier: %d", whichBlock);
        
      offset = fixedOffset(rbStart[whichBlock]);
    }
    
    bb = bbOfSize(BZ_MAX_BLOCK);
    end = readBlock(xfopen(inFileName, "r"), offset, bb);
    debug("Res size: %llu; read %llu bits", bb->pos, end);
    
    if(doDecompress) {
      int res;
      char result_buf[BZ_MAX_BLOCK];
      uint32_t destLen = BZ_MAX_BLOCK;
      debug("decompressing %llu bytes", bb->pos);
      if((res = decompressBlock(bb->buff, bb->pos, result_buf, &destLen)) != BZ_OK) {
        debug("bb->buff: 0x%x; bb->pos: %llu; destLen: %d", bb->buff, bb->pos, destLen);
        fatal("error decompressing block: %d", res);
      }
      
      debug("res: %d; destLen: %u", res, destLen);
      
      if(truncate > 0)
        fwrite(result_buf, 1, MIN(destLen, truncate), stdout);
      else
        fwrite(result_buf, 1, destLen, stdout);
      
    } else
      fwrite(bb->buff, 1, bb->pos, stdout);
  }

  debug("finished");
  return 0;
}
#endif
