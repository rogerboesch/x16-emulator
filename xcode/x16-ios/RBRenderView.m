//
//  RBRenderView.h
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#import "RBRenderView.h"
#include "platform_virtual_keys.h"

extern int platform_main(bool record);

RBRenderView* INSTANCE_OF_RENDERVIEW = NULL;

@interface RBRenderView ()

@property (nonatomic, retain) OSImageView* renderImageView;
@property (nonatomic) int screenWidth;
@property (nonatomic) int screenHeight;

@end

@implementation RBRenderView

#pragma mark - Helper

- (void)setBackground:(OSColor *)color {
    SET_BACKGROUND_COLOR(color);
}

#pragma mark - Emulator

- (void)render:(uint8_t*)buffer {
    int length = self.screenWidth*self.screenHeight*4;
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer, length, NULL);
    
    int bitsPerComponent = 8;
    int bitsPerPixel = 4*bitsPerComponent;
    int bytesPerRow = 4*self.screenWidth;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    CGImageRef imageRef = CGImageCreate(self.screenWidth, self.screenHeight, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
    CGDataProviderRelease(provider);
    
    UIImage* myImage = [UIImage imageWithCGImage:imageRef];
    self.renderImageView.image = myImage;

    CGImageRelease(imageRef); 
    CGColorSpaceRelease(colorSpaceRef);
}

- (void)startUsingWidth:(int)width height:(int)height {
    self.screenWidth = width;
    self.screenHeight = height;

    [self stop];
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        platform_main(false);
    });
}

- (void)stop {
}

- (void)pause {
}

- (void)resume {
}

- (void)reset {
}

- (void)nextFrame {
}

#pragma mark - View

- (void)resizeSubviewsWithOldSize:(OSSize)oldSize {
	self.renderImageView.frame = self.bounds;
}

- (void)setFrame:(OSRect)frame {
	[super setFrame:frame];

    self.renderImageView.frame = self.bounds;
}

#pragma mark - Initialisation

- (id)initWithFrame:(OSRect)frame {
	self = [super initWithFrame:frame];
	
    INSTANCE_OF_RENDERVIEW = self;
    
    self.renderImageView = [[UIImageView alloc] initWithFrame:CGRectZero];
    self.renderImageView.contentMode = UIViewContentModeScaleAspectFit;
    self.renderImageView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self addSubview:self.renderImageView];

	return self;
}

+ (RBRenderView *)shared {
    return INSTANCE_OF_RENDERVIEW;
}

@end

void platform_render_buffer(uint8_t* framebuffer) {
    dispatch_sync(dispatch_get_main_queue(), ^{
        [INSTANCE_OF_RENDERVIEW render:framebuffer];
    });
}
