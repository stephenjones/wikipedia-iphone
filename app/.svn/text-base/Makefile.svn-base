# all a bit dirty really

CC=arm-apple-darwin-gcc
LD=$(CC)
LDFLAGS=-lobjc -framework CoreFoundation -framework CoreGraphics -framework WebKit -framework Foundation -framework UIKit -framework LayerKit -lbz2
CFLAGS=-I/usr/local/arm-apple-darwin/include -DWP_INCLUDE -DDEBUG -g 
SEARCHVIEW=SearchView.o

all:    WikiApp

WikiApp:  WikiApp.o $(SEARCHVIEW) WebKitView.o main.o ArticleView.o SearchResult.o SearchBar.o WPParser.o ReadStream.o ../c/wp.o ../c/bzipreader.o ../c/safe.o ../c/lsearcher.o ../c/blocks.o
	$(LD) $(LDFLAGS) -o $@ $^

%.o:    %.m
		$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%.do:		%.m
		cat $< | ../rb/objcproc.rb > _debug_$< && $(CC) -c $(CFLAGS) $(CPPFLAGS) _debug_$< -o $@

clean:
		rm -f *.o WikiApp

install:
		scp WikiApp root@iph:/Applications/Wikipedia.app/Wikipedia

# parser test command-line app
parser-test:
		gcc ParserTest.m WPParser.m ReadStream.m -o ParserTest -lobjc -framework CoreFoundation -framework Foundation

iph-parser-test:
		$(CC) $(CFLAGS) $(LDFLAGS) ParserTest.m WPParser.m ReadStream.m -o ParserTest -lobjc -framework CoreFoundation -framework Foundation