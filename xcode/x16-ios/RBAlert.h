//
//  RBAlert.h
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#import <UIKit/UIKit.h>

@interface RBAlert : NSObject

+ (void)run:(UIViewController *)parent title:(NSString *)title text:(NSString *)text buttons:(NSArray *)buttons completion:(void (^)(int))completion;
+ (void)runYesNo:(UIViewController *)parent title:(NSString *)title text:(NSString *)text completion:(void (^)(BOOL))completion;

@end
