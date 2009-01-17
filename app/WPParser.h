#import <Foundation/Foundation.h>
#import "ReadStream.h"

@interface WPParser : NSObject {
  ReadStream *raw;
  NSMutableString *html;
  BOOL italic;
  BOOL bold;
  int tmplLevel;
  int linkLevel;
  BOOL inTable, inTableRow, inTableCaption, inTableCell, inTableHeadingCell;
  NSMutableArray* indents;
}

-(WPParser *) initWithMarkup: (NSString *) markup;
-(void) parse;
-(NSString *) parsed;

@end
