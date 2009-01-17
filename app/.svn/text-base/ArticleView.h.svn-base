#import <UIKit/UIKit.h>
#import "WebKitView.h"
#import "WPParser.h"
#import "../c/wp.h"

@interface ArticleView : UIView {
	WebKitView *wkv;
  NSString *currentTitle;

  wp_dump *dump;
  wp_article *article;
}
-(void) setPolicyDelegate: (id) obj;
-(void) updateHTML: (NSString *) str;
-(void) setDump: (wp_dump *) d;
-(void) setWikiApp: (id) w;
@end
