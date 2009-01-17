#import "WebKitView.h"
#import <UIKit/UIView-Geometry.h>

@implementation WebKitView

-(void)loadString: (NSString *)str
{
  [webView loadHTMLString: str baseURL: [NSURL URLWithString: @"http://foo.com"]];
  [scroller setOffset: CGPointMake(0, 0)];
}

-(void)loadURL: (NSURL *)url
{
  NSURLRequest* urlRequest = [NSURLRequest requestWithURL: url];
	[webView loadRequest: urlRequest];
}

-(void)dealloc
{
	[webView release];
	[super dealloc];
}

-(void)setPolicyDelegate: (id) obj {
  [[webView webView] setPolicyDelegate: obj];
}

-(void)view:(id)v didDrawInRect:(CGRect)f duration:(float)d
{
	if(v == webView) 
		[scroller setContentSize: CGSizeMake(320, [webView bounds].size.height)];
}

-(id)initWithFrame: (CGRect)frame
{
	[super initWithFrame: frame];

	scroller = [[UIScroller alloc] initWithFrame: frame];
	[scroller setScrollingEnabled: YES];
	[scroller setAdjustForContentSizeChange: YES];
	[scroller setClipsSubviews: YES];
	[scroller setAllowsRubberBanding: YES];
	[scroller setDelegate: self];


  webView = [[UIWebView alloc] initWithFrame: frame]; //[scroller bounds]];
  [webView setAutoresizes: YES];
	[webView setDelegate: self];
	//[webView setFrame: frame];
  //[webView setAutoresizingMask: 2]; //don't reflow text... or something
	//[webView setSmoothsFonts: true];
  //[webView setClipsSubviews: YES];
	//[webView setEnabledGestures: YES];
  [webView setTilingEnabled: YES];
  [webView setTileSize: CGSizeMake(320.0f,1000.0f)];
	[scroller addSubview: webView];
	[self addSubview: scroller];

  
	return self;
}

@end


