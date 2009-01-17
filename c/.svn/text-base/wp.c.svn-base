#include "wp.h"

#define MAXSTR 1024
#define BOUNDS_HIT_END -2
#define BOUNDS_NO_START -3
#define BOUNDS_NO_TEXT -4
#define BOUNDS_FAIL -10

char *exact_match;
int exact_block;

void load_dump(wp_dump *d, char *dump, char *loc, char *ploc, char *blocks) {
  d->fp = xfopen(dump, "r");
  d->block_map = load_block_map(blocks);

  load_index(&d->index, loc, ploc);
}

void load_dump_from_conf(wp_dump *d, char *conf) {
  char path[MAXSTR];
  char locs[4][MAXSTR];
  FILE *cf = xfopen(conf, "r");
  fgets(path, MAXSTR, cf);
  path[strlen(path) - 1] = '\0';
  xfclose(cf);

  snprintf(locs[0], MAXSTR, "%s/processed", path);
  snprintf(locs[1], MAXSTR, "%s/locate.db", path);
  snprintf(locs[2], MAXSTR, "%s/locate.prefixdb", path);
  snprintf(locs[3], MAXSTR, "%s/blocks.db", path);

  load_dump(d, locs[0], locs[1], locs[2], locs[3]);
}

void init_article(wp_article *a) {
  a->text = malloc(BZ_MAX_BLOCK);
}

bool handle_exact_match(char *s) {
  char buf[MAXSTR], *end;
  strncpy(buf, s, MAXSTR);
  
  debug("handle_exact_match(%s)", s);

  end = strrchr(buf, ' ') - 1;
  *end = '\0';

  if(strcasecmp(buf, exact_match)) return true;
  else {
    exact_block = atoi(end + 1);
    return false;
  }
}

int block_load(wp_dump *d, int block, char *text, int *text_len) {
  uint32_t bzres = 0;
  uint64_t size;

  BitBuffer *bb = bbOfSize(BZ_MAX_BLOCK);
  size = readBlock(d->fp, (d->block_map)[block], bb);   

  if((bzres = decompressBlock(bb->buff, bb->pos, text, text_len)) != BZ_OK)
    fatal("error decompressing block: %d. article_file: 0x%x", bzres, d->fp);

  bbClose(bb);
}

int article_bounds(char *name, char *buf, char **start, int bufsize) {
  debug("find article start: %s, 0x%x, 0x%x, %d", name, buf, *start, bufsize);

  int len = 0;
  int nl = strlen(name);
  char *bufstart = buf;

  /* start of article */
  while(*start = strcasestr(buf, name))
    if(*start && *(*start - 2) == START_HEADING && *(*start + nl) == '\n')
      break;
    else
      buf = *start + 1;

  if(*start == NULL)
    return BOUNDS_NO_START;

  /* start of text */
  while(*(*start)++ != START_TEXT)
    if(**start == START_HEADING)
      return BOUNDS_NO_TEXT;

  /* end of text */
  while(((*start - bufstart) + len) < bufsize && (*(*start + len++) != END_TEXT));

  int size = ((*start - bufstart) + len);
  debug("bufsize: %d, size: %d, len: %d, *start: %d, bufstart: %d, char: %d", bufsize, size, len, *start, bufstart, *(*start + len - 1));
  if(size == bufsize && *(*start + len) != END_TEXT) {
    debug("Hit the end early");
    return BOUNDS_HIT_END;  
  }

  return len - 1;
}

int load_article(wp_dump *d, char *name, wp_article *a) {
  exact_match = name;
  exact_block = -1;
  
  debug("load_article(0x%x, %s, 0x%x)", d, name, a);

  search(&d->index, name, handle_exact_match, NULL, false, true);

  if(exact_block < 0)
    return -1;
  else {
    return block_load_article(d, name, exact_block, a);
  }
}

int block_load_article(wp_dump *d, char *name, int block, wp_article *a) {
  char *text = xalloc(BZ_MAX_BLOCK);
  int text_len = BZ_MAX_BLOCK, article_len;
  char *start;

  debug("opening %s from block %d (%llu)", name, block, (d->block_map)[block]);
  block_load(d, block, text, &text_len);

  if((article_len = article_bounds(name, text, &start, text_len)) < 0) {
    if(article_len == BOUNDS_HIT_END) {
      /* the start was in the block, but we hit the end of the block before finding
       * the end of the article text. Load the next block and re-enter...
       * We only consider the case of loading one extra block; no Wikipedia article will span
       * more than two (it'd have to be >900kb for it to be possible). 
       */

      if(!d->block_map[block + 1])
        fatal("trying to load another block, but don't have any blocks left. "
              "This probably indicates a malformed dump.");

      debug("Loading another block");
      if(!(text = realloc(text, BZ_MAX_BLOCK * 2))) fatal("realloc");
      int n_text_len = BZ_MAX_BLOCK;

      block_load(d, block + 1, text + text_len, &n_text_len);

      if((article_len = article_bounds(name, text, &start, n_text_len + text_len)) < 0)
        fatal("additional block loading failure: %d", article_len);
    } else
      fatal("couldn't find %s in block %d", name, block);
  }

  debug("start: 0x%x, len: %d", start, article_len);
  debug("a->text: 0x%x", a->text);

  *(start + article_len) = '\0';
  strncpy(a->text, start, article_len + 1);
  a->block = block;
  free(text);
  return article_len;
}

#ifndef WP_INCLUDE
int main(int argc, char **argv) {
  debug = true;

  wp_dump d;
  wp_article a = {0};

  load_dump(&d, argv[1], argv[2], argv[3], argv[4]);
  load_article(&d, argv[5], &a);
  printf("%s\n", a.text);
  
  return 0;
}
#endif
