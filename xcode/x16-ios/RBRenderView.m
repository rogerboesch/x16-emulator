
#import "RBRenderView.h"
#include "platform_virtual_keys.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

extern int platform_main(bool record);

typedef enum _RunMode {
    run_mode_start, run_mode_stop, run_mode_pause, run_mode_resume, run_mode_reset, run_mode_frame, run_mode_step
} RunMode;

RBRenderView* INSTANCE_OF_RENDERVIEW = NULL;

@implementation UIImage (Buffer)

#pragma mark - Image handling

+ (UIImage *)imageWithBuffer:(void *)buffer width:(int)width height:(int)height {
    int length = width*height*4;
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer, length, NULL);
    
    int bitsPerComponent = 8;
    int bitsPerPixel = 4 * bitsPerComponent;
    int bytesPerRow = 4*width;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    CGImageRef imageRef = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
    CGDataProviderRelease(provider);
    
    UIImage* myImage = [UIImage imageWithCGImage:imageRef];
    
    // CGDataProviderRelease(provider);
    // CGColorSpaceRelease(colorSpaceRef);
    // CGImageRelease(imageRef); // TODO: This results in a crash

    return myImage;
}

@end

@interface RBRenderView ()

@property (nonatomic, retain) OSImageView* renderImageView;

@end

@implementation RBRenderView

#pragma mark - Run mode

- (void)runModeX16:(RunMode)mode {
    switch (mode) {
        case run_mode_start:
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                platform_main(false);
            });
            break;
        case run_mode_stop:
            break;
        case run_mode_pause:
            break;
        case run_mode_resume:
            break;
        case run_mode_reset:
            break;
        case run_mode_frame:
            break;
        default:
            break;
    }
}

#pragma mark - Helper

- (void)setBackground:(OSColor *)color {
    SET_BACKGROUND_COLOR(color);
}

#pragma mark - Emulator

- (void)render:(unsigned char*)machine {    
    UIImage *machineImage = [UIImage imageWithBuffer:machine width:SCREEN_WIDTH height:SCREEN_HEIGHT];
    self.renderImageView.image = machineImage;
}

- (void)start {
    [self stop];
    [self runModeX16:run_mode_start];
}

- (void)stop {
    [self runModeX16:run_mode_stop];
}

- (void)pause {
    [self runModeX16:run_mode_pause];
}

- (void)resume {
    [self runModeX16:run_mode_resume];
}

- (void)reset {
    [self runModeX16:run_mode_reset];
}

- (void)nextFrame {
    [self runModeX16:run_mode_step];
}

#pragma mark - UI events

+ (void)sendKey:(int)press code:(int)code ctrlKeyPressed:(BOOL)ctrlKeyPressed {
    int ctrlPressed = 0;
    int shiftPressed = 0;
    int altPressed = 0;
    RBVirtualKey keyCode = RBVK_Unknown;
    char ch = 0;

    if (ctrlKeyPressed) {
        ctrlPressed = 1;
        ch = (char)code;
        char ch2 = _toupper(ch);
        keyCode = character_to_vk(ch2);
        
        if (ch2 == 'Q') {
            ctrlPressed = 0;
            keyCode = RBVK_Escape;
        }
    }
    else {
        // Test modifiers
        switch (code) {
            case RBVK_BackSpace:
                NSLog(@"Backspace pressed");
                ch = '\b';
                keyCode = code;
                break;
            case RBVK_Escape:
                NSLog(@"Escape pressed");
                ch = '\e';
                keyCode = code;
                break;
            case RBVK_Space:
                NSLog(@"Space pressed");
                ch = ' ';
                keyCode = code;
                break;
            case RBVK_Tab:
                NSLog(@"Tab pressed");
                ch = '\t';
                keyCode = code;
                break;
            case RBVK_Return:
                NSLog(@"Return pressed");
                ch = '\r';
                keyCode = code;
                break;
            case RBVK_Left:
            case RBVK_Right:
            case RBVK_Up:
            case RBVK_Down:
                NSLog(@"Cusror pressed");
                ch = '\0';
                keyCode = code;
                break;
            default:
                // Test ascii characters
                if (code > 0) {
                    ch = code;
                    char ch2 = _toupper(ch);
                    keyCode = character_to_vk(ch2);
                }
                break;
        }
    }
    
    if (code == RBVK_Unknown) {
        return;
    }

    /*
    if (press)
        zx_emulator_send_event(RBEVT_KeyPressed, ctrlPressed, shiftPressed, altPressed, keyCode, ch);
    else
        zx_emulator_send_event(RBEVT_KeyReleased, ctrlPressed, shiftPressed, altPressed, keyCode, ch);
     */
}

+ (void)handleKey:(int)code ctrlKeyPressed:(BOOL)ctrlKeyPressed {
    [RBRenderView sendKey:1 code:code ctrlKeyPressed:ctrlKeyPressed];
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
    self.renderImageView.contentMode = UIViewContentModeScaleAspectFill;
    self.renderImageView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self addSubview:self.renderImageView];

	return self;
}

+ (RBRenderView *)shared {
    return INSTANCE_OF_RENDERVIEW;
}

@end

void platform_render_buffer(uint8_t* framebuffer) {
    CFRunLoopRun();
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        [INSTANCE_OF_RENDERVIEW render:framebuffer];
    });
}
