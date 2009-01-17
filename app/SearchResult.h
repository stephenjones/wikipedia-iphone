#import <UIKit/UIKit.h>

@interface SearchResult : UITableCell
{
  NSString *article;
}
-(void) setArticle: (NSString *) title;
-(NSString *) article;
@end
