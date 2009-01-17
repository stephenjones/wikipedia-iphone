#include "search.h"

int returned;
int maxres = 10;
char needle[MAXLINE];

bool print_match(char *s, uint32_t block) {
  returned++;
  printf("%s %d\n", s, block);
  if(returned < maxres) return true;
  else return false;
}

void usage(char *name) {
  fprintf(stderr, "%s -f <indexFile> -s <search> [-m <maxres>] [-d]\n", name);
  exit(-1);
}

int main(int argc, char **argv) {
  char ch, indexFile[MAXLINE];
  bool printTree = false, printCmd = false;

  while ((ch = getopt(argc, argv, "cm:s:idf:hp")) != -1) {
    switch (ch) {
    case 'c':
      printCmd = true;
      break;  
    case 's':
      strcpy(needle, optarg);
      break;
    case 'd':
      debug = true;
      break;
    case 'f':
      strcpy(indexFile, optarg);
      break;
    case 'm':
      maxres = atoi(optarg);
      break;
    case 'i':
      csen_set(false);
      break;
    case 'p':
      printTree = true;
      break;
    case 'h':
    default:
      usage(argv[0]);
    }
  }

  load_root(indexFile);
  if(printTree) print_tree();
  else if(printCmd) print_cmd();
  else if(!needle[0] || !indexFile[0])
    usage(argv[0]);
  else {
    root_search(needle, print_match);
    printf("%d matches\n", returned);
  }

  return 0;
}
