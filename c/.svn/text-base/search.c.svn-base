#include "search.h"

#define NODES(n) ((n) * sizeof(struct node))

static uint32_t storepos = 0;
static char needle[MAXLINE];
static char so_far[MAXLINE];
static struct node *root;
static bool done;
static bool csen = true;
static char cmd[MAXLINE];

struct node *load_index(char *file) {
  void *ptr;
  struct index_hdr h;
  FILE *fp = xfopen(file, "r");

  fread(&h, sizeof(struct index_hdr), 1, fp);
  storepos = h.nodes;
  strncpy(cmd, h.cmd, MAXLINE);

  if(h.node_size != sizeof(struct node))
    fatal("Couldn't read index: conflicting node sizes. %s expects %d, " \
          "I've been compiled to expect %d. Did you use -fpack-struct?", \
          file, h.node_size, sizeof(struct node));

  ptr = xmmap(NULL, NODES(storepos), PROT_READ, MAP_PRIVATE, fileno(fp), 0);

  return ptr + sizeof(struct index_hdr);
}

void print_cmd() {
  printf("%s\n", cmd);
}

void load_root(char *file) {
  root = load_index(file);
}

char *matchstr(int depth, struct node *n, char *match) {
  debug("matchstr(%d, 0x%x)", depth, n);

  int i, j = 0;
  bool compressed;

  compressed = n->block & CMASK;

  for(i = 0; i < depth; i++)
    if(so_far[i])
      match[j++] = so_far[i];

  match[j] = '\0';
  debug("so_far: %s", match);
  
  if(compressed) {
    char *s = ((struct cnode *) n)->str;
    debug("compressed str: %s", s);
    strncpy(match + j, s, strlen(s));
    j += strlen(s);
  } else {
    match[j++] = n->c;
  }
  
  match[j] = '\0';

  debug(" -> %s", match);

  return match;
}

bool handle_matches(int startdepth, int depth, struct node *n, resultf r) {
  debug("handle_matches(%d, %d, 0x%x %d %c)", startdepth, depth, n, n - root, n->c);

  bool compressed = n->block & CMASK;
  char match[MAXLINE];

  if(BLOCK_DEMASK(n->block)) {
    if(!r(matchstr(depth, n, match), BLOCK_DEMASK(n->block))) {
      done = true;
      return false;
    }
  }

  if(compressed) return true;

  if(depth == startdepth && n->eq)
    handle_matches(startdepth, depth + 1, root + n->eq, r);
  else {
    if(n->lt)
      if(!handle_matches(startdepth, depth, root + n->lt, r))
        return false;
    if(n->eq) {
      debug("following eq...");
      so_far[depth] = n->c;
      if(!handle_matches(startdepth, depth + 1, root + n->eq, r))
        return false;
    }
    if(n->gt)
      if(!handle_matches(startdepth, depth, root + n->gt, r))
        return false;
  }

  return true;
}

void csen_set(bool b) {
  csen = b;
}

void csearch(char *prefix, struct cnode *n, resultf r, int depth) {
  debug("csearch(%s, (0x%x, %s), %d)", prefix, n, n->str, depth);
  if(!strncmp(prefix, n->str, strlen(prefix)))
    handle_matches(depth, depth, (struct node *) n, r);
}

void search(char *prefix, struct node *n, resultf r, int depth) {
  debug("0x%x search(%s, (%d, %c), %d)", n, prefix, n - root, n->c, depth);

  if(n->block & CMASK) {
    csearch(prefix, (struct cnode *) n, r, depth);
    return;
  }

  if(((!csen && tolower(n->c) == tolower(prefix[0])) || \
     (csen && n->c == prefix[0])) && !done) { 
    so_far[depth] = n->c;
    debug("0x%x match", n);
    if(prefix[1]) {
      if(n->eq)
        search(prefix + 1, root + n->eq, r, depth + 1);
    } else 
      handle_matches(depth, depth, n, r);

  }
  
  if(!done) {
    if((n->c > prefix[0] || (isascii(prefix[0]) && !csen && n->c > (prefix[0]^0x20))) \
        && n->lt) 
      search(prefix, root + n->lt, r, depth);
    
    if((n->c < prefix[0] || (isascii(prefix[0]) && !csen && n->c < (prefix[0]^0x20))) \
        && n->gt) 
      search(prefix, root + n->gt, r, depth);
  }
}

void root_search(char *prefix, resultf r) {
  done = false;
  strcpy(needle, prefix);
  search(prefix, root, r, 0);
}

void print_tree() {
  int i;
  for(i = 0; i < storepos; i++) {
    struct node *n = root + i;

    debug("--");
    debug("%d", i);

    if(n->block == -1) {
      debug("obsolete");
      continue;
    }

    debug("block: %d", BLOCK_DEMASK(n->block));
    if(n->block & CMASK)
      debug("COMPRESSED: %s", ((struct cnode *) n)->str);
    else {
      debug("c: %c", n->c);  
      debug("lt: %d | eq: %d | gt: %d", n->lt, n->eq, n->gt);
    }
  }
}
