#import <UIKit/UIKit.h>
#import <UIKit/UIScroller.h>
#import <UIKit/UIWebView.h>

@interface WebKitView : UIScroller {
	UIWebView *webView;
  UIScroller *scroller; 
}

-(id)initWithFrame: (CGRect)frame;
-(void)loadURL: (NSURL *)url;
-(void)dealloc;
-(void)loadString: (NSString *)str;

@end
