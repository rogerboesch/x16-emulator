//
//  RBRenderView.h
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#import <UIKit/UIKit.h>
#import "platform_virtual_keys.h"

#define OSView UIView
#define OSColor UIColor
#define OSImageView UIImageView
#define OSRect CGRect
#define OSSize CGSize

#define SET_BACKGROUND_COLOR(color) self.backgroundColor = color

@interface RBRenderView : OSView

@property (nonatomic, retain) OSView* toolbar;

- (void)start;
- (void)stop;
- (void)pause;
- (void)resume;
- (void)reset;
- (void)nextFrame;

+ (RBRenderView *)shared;

@end
