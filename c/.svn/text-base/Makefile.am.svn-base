bin_PROGRAMS = searcher indexer livesearch bzipreader blocks lsearcher wp \
	process lbigram lcode

AM_CFLAGS = -g -Wall 

searcher_CFLAGS = -fpack-struct
livesearch_CFLAGS = -DDEUG -DWP_INCLUDE
indexer_CFLAGS = -fpack-struct
bzipreader_CFLAGS = -DINCLUDE_MAIN
blocks_CFLAGS = -DINCLUDE_MAIN
lsearcher_CFLAGS = -DINCLUDE_MAIN -DDEBUG

LDADD = safe.o

bzipreader_LDADD = $(LDADD) -lbz2
livesearch_LDADD = $(LDADD) -lncurses

livesearch_SOURCES = lsearcher.c livesearch.c bzipreader.c blocks.c wp.c
wp_SOURCES = lsearcher.c bzipreader.c blocks.c wp.c
searcher_SOURCES = search.c searcher.c

DISTCLEANFILES = AUTHORS README ChangeLog INSTALL NEWS COPYING Makefile.in \
							 configure Makefile install-sh missing

all:
	ln -sf livesearch reader

distclean-local:
	-rm -rf *.dSYM *.log *.status *.cache
