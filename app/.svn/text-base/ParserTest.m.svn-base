#import "WPParser.h"
#import <Foundation/Foundation.h>

#define BUF 1024

int main(int argc, char **argv) {
  char buf[BUF];
  NSAutoreleasePool* pool = [NSAutoreleasePool new];

  NSMutableString *str = [[NSMutableString alloc] init];
  
  while(!feof(stdin)) {
    size_t read = fread(buf, sizeof(char), BUF - 1, stdin);
    buf[read] = '\0';
    [str appendString: [NSString stringWithUTF8String: buf]];
  }

  WPParser *wp = [[WPParser alloc] initWithMarkup: str];
  printf("%s", [[wp parsed] UTF8String]);
  [pool release];
}
