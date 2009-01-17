#include "ternary.h"

static uint32_t storepos = 0;
struct node store[INDEX_LEN];
#define NODE(node) (store + (node))
char line[MAXLINE];

void insert(struct node *node, char *str, uint32_t block) {
  int cmp = CMP(str[0], node->c);
  debug("insert %d, %s, %d", node - store, str, block);

  if(!node->c) {
    bzero(node, sizeof(node));
    node->c = str[0];
    if(str[1]) {
      node->eq = ++storepos;
      insert(NODE(storepos), str + 1, block);
    } else node->block = BLOCK_MASK(block);
  } else if(str[0]) {
    if(NEXT_NODE(node, cmp)) {
      if(!cmp) {
        if(str[1])
          insert(NODE(NEXT_NODE(node, cmp)), str + 1, block);
        else
          node->block = BLOCK_MASK(block);
      } else
        insert(NODE(NEXT_NODE(node, cmp)), str, block);
    } else if(!(cmp == 0 && !str[1])) { 
      /* unless at end of string, and all matches */
      NEXT_NODE(node, cmp) = ++storepos;
      insert(NODE(storepos), cmp ? str : str + 1, block);
    } else {
      node->block = BLOCK_MASK(block); 
    }    
  }
}

void collapse(struct node *node) {
  compact_str s;
  struct node *ptr;
  uint32_t offset;
  struct cnode *cnode;
  int i;
  uint32_t last_block;

  offset = node - store;
  i = 0;
  do {
    ptr = NODE(offset);
    s[i++] = ptr->c;
    last_block = ptr->block;
    if(ptr != node) ptr->block = -1;
  } while(offset = ptr->eq);
  s[i] = '\0';

  cnode = (struct cnode *) node;
  cnode->block = last_block | CMASK;
  strncpy(cnode->str, s, sizeof(compact_str));
}

int compact(struct node *node, bool eager) {
  int r;

  if(node->lt || node->gt) {
    if(node->lt) compact(NODE(node->lt), true);
    if(node->gt) compact(NODE(node->gt), true);
  }
  
  if(node->eq) {
    r = compact(NODE(node->eq), node->lt || node->gt);

    if(r > 0 && (r + 2 == sizeof(compact_str) || eager)) {
      collapse(NODE(node->eq));
      return -1;
    }

    if(node->lt || node->gt || (node->eq && node->block))
      return -1;
    else
      return r >= 0 ? r + 1 : r;
  } else {
    if(!node->eq && !node->lt && !node->gt)
      return 0;
    else
      return -1;
  }
}

void dump_index(char *file, int argc, char **argv) {
  struct index_hdr h;
  int i, j = 0;

  h.nodes = storepos + 1;
  h.node_size = sizeof(struct node);

  for(i = 0; i < argc; i++) {
    j += snprintf(h.cmd + j, MAXLINE - j, "%s", argv[i]);
    if(i + 1 < argc) j += snprintf(h.cmd + j, MAXLINE - j, " ");
  }

  FILE *fp = xfopen(file, "w");

  fwrite(&h, sizeof(struct index_hdr), 1, fp);
  fwrite(store, sizeof(struct node), storepos + 1, fp);
  fclose(fp);
}

int slots_in_use() {
  int i, in_use = 0;
  for(i = 0; i < storepos; i++) {
    struct node *n = store + i;
    if(n->block != -1)
      in_use++;
  }
  return in_use;
}

void usage(char *name) {
  fprintf(stderr, "%s -f <indexFile> [-c] [-d]\n", name);
  exit(-1);
}

int main(int argc, char **argv) {
  char str[PREFIX_LEN];
  char *foo, indexFile[MAXLINE], ch;
  uint32_t block = 0;
  uint32_t inserted = 0;
  bool doCompact = false;

  while ((ch = getopt(argc, argv, "cdf:h")) != -1) {
    switch (ch) {
    case 'c':
      doCompact = true;
      break;
    case 'd':
      debug = true;
      break;
    case 'f':
      strcpy(indexFile, optarg);
      break;
    case 'h':
    default:
      usage(argv[0]);
    }
  }

  if(!indexFile)
    usage(argv[0]);

  while(fgets(line, MAXLINE, stdin)) {
    if((storepos + 1) >= INDEX_LEN)
      fatal("no more index space");

    foo = strchr(line, '|');
    foo[0] = '\0';
    foo++;

    strncpy(str, line, PREFIX_LEN - 1);
    str[PREFIX_LEN] = '\0';
    block = atoi(foo);

    debug("inserting '%s' with block %d", str, block);
    insert(store, str, block);
    inserted++;
  }

  if(doCompact) compact(store, true);

  dump_index(indexFile, argc, argv);

  printf("Inserted %d entries using %d slots\n", inserted, storepos);

  if(doCompact)
    printf("After compacting, using %d slots\n", slots_in_use());

  return 0;
}
