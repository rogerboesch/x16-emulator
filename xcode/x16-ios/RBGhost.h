//
//  rb_ghost.h
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import "TargetConditionals.h"

#import <UIKit/UIKit.h>

@interface RBGhost : NSObject

+ (void)typeIn:(NSString *)command;
+ (void)pressKey:(int)ch code:(int)code ctrlPressed:(BOOL)ctrlPressed;

@end
