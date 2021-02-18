#import <UIKit/UIKit.h>

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

+ (void)sendKey:(int)press code:(int)code ctrlKeyPressed:(BOOL)ctrlKeyPressed;
+ (void)handleKey:(int)code ctrlKeyPressed:(BOOL)ctrlKeyPressed;

+ (RBRenderView *)shared;

@end
