#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/CDStructures.h>
#import <UIKit/UIPushButton.h>
#import <UIKit/UIThreePartButton.h>
#import <UIKit/UINavigationBar.h>
#import <UIKit/UIWindow.h>
#import <UIKit/UIView-Hierarchy.h>
#import <UIKit/UIHardware.h>

#import "WebKitView.h"
#import "WikiApp.h"

@implementation WikiApp

-(UINavigationBar *) createNavBar: (struct CGRect) rect {
  struct CGSize s = [UINavigationBar defaultSize];
  UINavigationBar *bar = [[UINavigationBar alloc] 
                            initWithFrame: CGRectMake(0, 0, s.width, s.height)];
  [bar setDelegate: self];
  [bar enableAnimation];

  return bar;
}

-(void) updateNavBar {
  switch(currentView) {
    case ARTICLE_VIEW:
      [navBar showButtonsWithLeftTitle: @"Search" rightTitle: NULL leftBack: YES];
      break;
    case SEARCH_VIEW:
      [navBar showButtonsWithLeftTitle: NULL rightTitle: @"Article"];
      break;
  }
}

-(void) switchToArticle {
  currentView = ARTICLE_VIEW;
  [self updateNavBar];
  [tf hide];
  [contentView transition: 1 toView: aview];
}

-(void) switchToSearch {
  currentView = SEARCH_VIEW;
  [self updateNavBar];
  [contentView transition: 2 toView: sview];
  [tf show];
}

-(void) navigationBar: (UINavigationBar *) bar buttonClicked: (int) button {
  switch(button) {
    case 0:
      [self switchToArticle];
      break;
    case 1:
      [self switchToSearch];
      break;
  }
}

-(void) logText: (id) not {
  NSString *str = [[not object] text];
 
  if(str != nil)
    [sview setNeedle: str];
}

-(void) createSearchField {
  tf = [[SearchBar alloc] initWithFrame: (CGRectMake(5, 6, 240, 30))];

  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(logText:)
    name: UITextFieldTextDidChangeNotification 
    object: tf];

  [navBar addSubview: tf];
}

-(void) createArticleView: (struct CGRect) r {
  aview = [[ArticleView alloc] initWithFrame: r];
}

-(void) createSearchView: (struct CGRect) r {
  sview = [[SearchView alloc] initWithFrame: r];
  [sview setParent: self];
}

-(void) setupUI {
  currentView = SEARCH_VIEW;

  rect = [UIHardware fullScreenApplicationContentRect];
  rect.origin.x = rect.origin.y = 0.0f;

  window = [[UIWindow alloc] initWithContentRect: [UIHardware fullScreenApplicationContentRect]];

  mainView = [[UITransitionView alloc] initWithFrame: rect];

  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  float bgColor[4] = {0.2, 0.2, 0.2, 1};
  CGColorRef bgRef = CGColorCreate(colorSpace, bgColor);
  [mainView setBackgroundColor: bgRef];

  navBar = [self createNavBar: rect];
  [mainView addSubview: navBar];

  struct CGSize s = [UINavigationBar defaultSize];
  struct CGRect r = CGRectMake(0, 0, rect.size.width, rect.size.height - s.height);
  contentView = [[UITransitionView alloc] 
                  initWithFrame: CGRectMake(0, s.height, rect.size.width, rect.size.height - s.height)];
  [mainView addSubview: contentView];

  [self createSearchField];

  [self updateNavBar];
  [window orderFront: self];
  [window makeKey: self];
  [window _setHidden: NO];
  [window setContentView: mainView];

  [self createSearchView: r];
  [self createArticleView: r];
  [sview setArticleView: aview];
  [contentView addSubview: sview];
}

- (void) applicationDidFinishLaunching: (id) unused
{
  [self setupUI];

  [aview updateHTML: @"<html><body></body></html>"];
}

@end
