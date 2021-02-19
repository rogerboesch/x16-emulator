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

- (void)render:(unsigned char *)machine;

+ (RBRenderView *)shared;

@end
