#import "SearchBar.h"

@implementation SearchBar

-(SearchBar *) initWithFrame: (struct CGRect) rect {
  self = [super initWithFrame: rect];

  bounds = rect;

  [self setTextFont: @"font-family: Helvetica; font-size: 14px; padding-top: 5px; padding-left: 4px;"];
  [self setAutoCapsType: 0];
  [self setPreferredKeyboardType: 0];
  [self setAutoCorrectionType: 1]; //disabled
  [self setOpaque: 0];
  [self setClearButtonStyle: 1];
  //[self setPlaceholder: @"Search"];
  [self setDelegate: self];
  [self becomeFirstResponder];

  return self;
}

- (void) show
{
  [self setTransform: CGAffineTransformMake(1,0,0,1,0,0)];
  [self setFrame: CGRectMake(-bounds.size.width, bounds.origin.y, bounds.size.width, bounds.size.height)];

  struct CGAffineTransform trans = CGAffineTransformMakeTranslation(bounds.size.width + 10, 0);
  UITransformAnimation *translate = [[UITransformAnimation alloc] initWithTarget: self];
  [translate setStartTransform: CGAffineTransformMake(1,0,0,1,0,0)];
  [translate setEndTransform: trans];

  UIAlphaAnimation *fade = [[UIAlphaAnimation alloc] initWithTarget: self];
  [fade setEndAlpha: 1];
  [fade setStartAlpha: 0];

  UIAnimator *animator = [[UIAnimator alloc] init];
  [animator addAnimation: translate withDuration: .3 start: YES];
  [animator addAnimation: fade withDuration: .3 start: YES];

}

- (void) hide
{
  struct CGRect rect = [UIHardware fullScreenApplicationContentRect];
  rect.origin.x = rect.origin.y = 0;
    
  [self setTransform: CGAffineTransformMake(1,0,0,1,0,0)];
  [self setFrame: CGRectMake(0, bounds.origin.y, bounds.size.width, bounds.size.height)];
    
  struct CGAffineTransform trans = CGAffineTransformMakeTranslation(-(bounds.size.width + 10), 0);

  UITransformAnimation *translate = [[UITransformAnimation alloc] initWithTarget: self];

  [translate setStartTransform: CGAffineTransformMake(1,0,0,1,0,0)];
  [translate setEndTransform: trans];

  UIAlphaAnimation *fade = [[UIAlphaAnimation alloc] initWithTarget: self];
  [fade setEndAlpha: 0];
  [fade setStartAlpha: 1];

  UIAnimator *animator = [[UIAnimator alloc] init];
  [animator addAnimation: translate withDuration: .3 start: YES];
  [animator addAnimation: fade withDuration: .3 start: YES];
}


@end

