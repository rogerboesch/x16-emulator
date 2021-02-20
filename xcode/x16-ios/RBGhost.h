//
//  RBGhost.h
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#import "TargetConditionals.h"
#import <UIKit/UIKit.h>
#import "platform_virtual_keys.h"

@interface RBGhost : NSObject

+ (void)typeIn:(NSString *)command;
+ (void)pressKey:(int)ch code:(int)code ctrlPressed:(BOOL)ctrlPressed;

@end
