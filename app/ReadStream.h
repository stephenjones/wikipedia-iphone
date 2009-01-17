#import <Foundation/Foundation.h>

@interface ReadStream : NSObject {
  NSString *str;
  int pos;
}

-(ReadStream *) initWithString: (NSString *) str;
-(unichar) peek;
-(unichar) peekTwo;
-(unichar) read;
-(unichar) expect: (unichar) c;
-(BOOL) nextIs: (unichar) c;
-(BOOL) nextTwoAre: (unichar) c;
-(BOOL) nextAre: (unichar) c1 and: (unichar) c2;
-(BOOL) isAtEnd;
-(BOOL) isAtStart;
-(int) position;
-(void) setPosition: (int) i;
-(NSString *) readUpto: (unichar) c;
-(NSString *) readLineUpto: (unichar) c;
-(NSString *) readLineUptoSingleBar;

@end
