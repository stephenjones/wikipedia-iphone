#import <UIKit/UIKit.h>
#import <UIKit/UICompletionTable.h>
#import <UIKit/UIProgressBar.h>
#import "WebKitView.h"
#import "SearchResult.h"
#import "ArticleView.h"
#import "../c/wp.h"

@interface SearchView : UIView {
  NSMutableArray *results;
  ArticleView *aview;
  UIKeyboard *kbrd;
  UIProgressBar *prog;
  id parent;
  UICompletionTable *table;
  NSString *needle;
  NSThread *sthread;
  NSTimer *updater;
  NSLock *lock;
  int curResults;
  bool needRefresh;

  int nsearch;
  char *curNeedle;

  wp_dump *dump;
  wp_article *article;
}

-(void) setArticleView: (ArticleView *) aview;
-(void) setParent: (id) parent;
-(void) setNeedle: (NSString *) needle;
-(void) runSearch: (char *) needle;
-(void) refreshResults;
@end
