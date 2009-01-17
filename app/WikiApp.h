#import <UIKit/UIKit.h>
#import "WebKitView.h"
#import "ArticleView.h"
#import "SearchView.h"
#import "SearchResult.h"
#import "SearchBar.h"

#define SEARCH_VIEW 1
#define ARTICLE_VIEW 2

@interface WikiApp : UIApplication {
	WebKitView *wkv;
	UIWindow *window;
  UINavigationBar *navBar;
  SearchBar *tf;
  UIView *mainView;
  UITransitionView *contentView;
  ArticleView *aview;
  SearchView *sview;
  struct CGRect rect;
  int currentView;
  NSString *currentTitle;
}
@end
