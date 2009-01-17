#include <ncurses.h>
#include <sys/time.h>
#include "lsearcher.h"
#include "safe.h"
#include "wp.h"

#define MAXRES 40
#define MAXSTR 1024

char needle[MAXSTR];
char results[MAXRES][MAXSTR];
uint32_t blocks[MAXRES];
int nresults;

char *article_file_str;
int article_pos;
int pos = 0;
void (*handler)(int c);

time_t last_sec = 0;
suseconds_t last_usec = 0;

wp_dump dump = {0};

void search_handler(int c);
void article_handler(int c);
void draw_results();
void maybe_draw_results();
void handle_finish();

bool handle_result(char *s) {
  char buf[MAXSTR] = {0};
  strcpy(buf, s);
  char *blockpos = strrchr(buf, ' ');
  int c;

  if(blockpos) {
    *(blockpos - 1) = '\0'; 
    blockpos++;

    blocks[nresults] = atoi(blockpos);
    strncpy(results[nresults++], buf, MAXSTR);

    maybe_draw_results();
  } else fatal("invalid: '%s'", buf);

  if((c = getch()) != ERR) {
    ungetch(c);
    return false;
  } else if (nresults >= MAXRES) {
    handle_finish();
    return false;
  } else
    return true;
}

void handle_finish() {
  nodelay(stdscr, FALSE);
}

void draw_results() {
  int i = 0, j = 0;
  char block[20], index[20];

  for(; i < nresults; i++) {
    move(i + 2, j);
    j += sprintf(index, "%c ", i == article_pos ? '*' : ' ');
    j += sprintf(index + j, "%6d", i + 1); 
    addstr(index);

    move(i + 2, j + 1);

    if(i == article_pos)
      attron(A_UNDERLINE);

    addstr(results[i]);

    if(i == article_pos)
      attroff(A_UNDERLINE);

    move(i + 2, 60);
    sprintf(block, "%d", blocks[i]);
    addstr(block);

    j = 0;
  }
}

void maybe_draw_results() {
  struct timeval t;
  gettimeofday(&t, NULL);

  if(t.tv_sec > last_sec || t.tv_usec > last_usec + 100)
    search_redraw();

  last_sec = t.tv_sec;
  last_usec = t.tv_usec;
}

void research() {
  nresults = 0;
  nodelay(stdscr, TRUE);
  search(&dump.index, needle, handle_result, handle_finish, true, true);
}

void search_redraw() {
  clear();
  move(0, 0);
  needle[pos] = '\0';
  addstr(needle);

  draw_results();

  refresh();
}

void draw_article() {
  size_t asize = BZ_MAX_BLOCK;
  int len;
  char *start = NULL; 
  char tbuf[60];
  wp_article article;

  init_article(&article);

  len = block_load_article(&dump, results[article_pos], blocks[article_pos], &article);

  if(len < 0) {
    move(1, 0);
    addstr("There was an error retrieving this article");
    refresh();
    return;
  }

  clear();

  move(1, 0);
  attron(A_UNDERLINE);
  addstr(results[article_pos]);
  attroff(A_UNDERLINE);

  move(2, 0);
  addnstr(article.text, len); 

  move(LINES - 2, 0);
  hline(0, COLS);

  move(LINES - 1, 0);
  clrtoeol();
  sprintf(tbuf, "Block %d (%d bytes)", blocks[article_pos], len);
  addstr(tbuf);

  sprintf(tbuf, "%59s", article_file_str);
  move(LINES - 1, COLS - 59);
  addstr(tbuf);

  refresh(); 
}

void article_handler(int c) {
  switch(c) {
    case KEY_ENTER:
    case 13:
      handle_finish();
      handler = search_handler;
      search_redraw();
      return;
  }
}

void search_handler(int c) {
  switch(c) {
    case KEY_BACKSPACE:
    case 127:
      if(pos > 0) {
        pos--;
        article_pos = 0;
        research();
      }
      break;
    case KEY_DOWN:
      if(article_pos + 1 < nresults) article_pos++;
      break;
    case KEY_UP:
      if(article_pos > 0) article_pos--;
      break;
    case KEY_ENTER:
    case 13:
      if(nresults > 0) {
        draw_article();
        handler = article_handler;
        return;
      }
      break;  
    default:
      article_pos = 0;
      needle[pos++] = c;
      research();
  }

  search_redraw();
}

void curses_loop() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nonl();
  intrflush(stdscr, FALSE);

  while(true) {
    int c = getch();
    if(c != ERR) handler(c);
  }
}

void reader_loop() {
  debug("reader_loop");

  wp_article a;
  char line[MAXSTR];

  init_article(&a);

  while(fgets(line, MAXSTR, stdin)) {
    line[strlen(line) - 1] = '\0';
    debug("retrieving %s", line);
    load_article(&dump, line, &a);
    if(a.block)
      puts(a.text);
    else
      printf("Couldn't retrieve %s", line);
  }
}

int main(int argc, char **argv) {
  bool noCurses = false;
  char *slash = strrchr(argv[0], '/');

  debug = true;

  if(!strcmp(slash ? slash + 1 : argv[0], "reader"))
    noCurses = true;
  
  load_dump(&dump, argv[1], argv[2], argv[3], argv[4]);
  article_file_str = argv[1];

  if(!noCurses) {
    handler = search_handler;
    curses_loop();
  } else {
    debug("calling reader_loop");
    reader_loop(); 
  }

  return 0;
}
