//
//  Toolbar.m
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#import "Toolbar.h"
#import "RBAlert.h"
#import "RBGhost.h"
#import "platform_ios.h"

extern void machine_reset(void);
extern void machine_toggle_warp(void);
extern void machine_toggle_geos(void);
extern void machine_paste(char*);
extern void machine_copy(void);
extern void machine_set_sd_card(char* path);
extern void machine_attach_sdcard(void);
extern void machine_deattach_sdcard(void);

typedef enum : NSUInteger {
    ToolBreak,
    ToolWarp,
    ToolReset,
    ToolPaste,
    ToolGEOS,
    ToolTextLine,
    ToolSDCard,
    ToolCopy
} Tool;

@interface Toolbar()
@property (nonatomic) BOOL hasSDCard;
@end

@implementation Toolbar

// MARK: - SD card functionality

- (void)loadSDcard{
    self.hasSDCard = [[NSUserDefaults standardUserDefaults] boolForKey:@"sd_card"];
}

- (void)saveSDcardPath {
    [[NSUserDefaults standardUserDefaults] setBool:self.hasSDCard forKey:@"sd_card"];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)mapNewCard {
    // Need to load/copy first
    platform_load_from_cloud_async();
    platform_wait_for_cloud_filename();
    
    // rename to sdcard.img
    NSString* fromPath = [NSString stringWithFormat:@"%s/TEMP", platform_get_documents_path()];
    NSString* toPath = [NSString stringWithFormat:@"%s", platform_get_sdcard_path("sdcard.img")];
    NSError* error = nil;
    [[NSFileManager defaultManager] moveItemAtPath:fromPath toPath:toPath error:&error];
    
    self.hasSDCard = YES;
}

- (void)mapCurrentSDcard {
    machine_set_sd_card(platform_get_sdcard_path("sdcard.img"));
    machine_attach_sdcard();
    
    [self saveSDcardPath];
}

- (void)unmapCurrentSDcard {
    machine_deattach_sdcard();
}

- (void)attachSDcard {
    if (self.hasSDCard) {
        // Ask to load or re-map
        [RBAlert run:self.viewController title:@"SD Card" text:@"You want map another card, or use/unmap the existing?" buttons:@[@"Map new card", @"Use current card", @"Unmap current card"] completion:^(int tag) {
            switch (tag) {
                case 0:
                    // Call again with disabled flag
                    self.hasSDCard = NO;
                    [self attachSDcard];
                    break;
                case 1:
                    [self mapCurrentSDcard];
                    break;
                case 2:
                    [self unmapCurrentSDcard];
                    break;
            }
        }];
        
        return;
    }
    else {
        // Need to load/copy first
        platform_load_from_cloud_async();
        platform_wait_for_cloud_filename();
        
        // rename to sdcard.img
        NSString* fromPath = [NSString stringWithFormat:@"%s/TEMP", platform_get_documents_path()];
        NSString* toPath = [NSString stringWithFormat:@"%s", platform_get_sdcard_path("sdcard.img")];
        NSError* error = nil;
        [[NSFileManager defaultManager] moveItemAtPath:fromPath toPath:toPath error:&error];
        
        self.hasSDCard = YES;
    }

    machine_set_sd_card(platform_get_sdcard_path("sdcard.img"));
    machine_attach_sdcard();
    
    [self saveSDcardPath];
}

// MARK: - Toolbar UI

- (void)tap:(UIButton *)sender {
    switch (sender.tag) {
        case ToolBreak:
            [RBGhost pressKey:0 code:RBVK_Escape ctrlPressed:NO];
            break;
        case ToolWarp:
            machine_toggle_warp();
            break;
        case ToolReset:
            machine_reset();
            break;
        case ToolPaste:
            machine_paste(platform_get_from_clipboard());
            break;
        case ToolCopy:
            machine_copy();
            break;
        case ToolGEOS:
            machine_toggle_geos();
            break;
        case ToolTextLine:
            [RBGhost pressKey:0 code:RBVK_F4 ctrlPressed:NO];
            break;
        case ToolSDCard:
            [self attachSDcard];
            break;
    }
}

- (void)addButton:(NSString *)imageName tool:(Tool)tool {
    UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
    [button setImage:[UIImage systemImageNamed:imageName] forState:UIControlStateNormal];
    button.frame = CGRectMake(0, 0, 40, 40);
    button.tag = tool;
    [button addTarget:self action:@selector(tap:) forControlEvents:UIControlEventTouchUpInside];
    UIBarButtonItem* item = [[UIBarButtonItem alloc] initWithCustomView:button];
    
    NSMutableArray* newItems = [NSMutableArray arrayWithArray:self.items];
    [newItems addObject:item];
    self.items = newItems;
}

- (void)addDelimiter:(BOOL)flexible {
    UIBarButtonItem* item = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:(flexible ? UIBarButtonSystemItemFlexibleSpace : UIBarButtonSystemItemFixedSpace) menu:nil];
    
    NSMutableArray* newItems = [NSMutableArray arrayWithArray:self.items];
    [newItems addObject:item];
    self.items = newItems;
}

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];

    self.barTintColor = UIColor.blackColor;

    [self loadSDcard];
    
    [self addButton:@"stop" tool:ToolBreak];
    [self addDelimiter:YES];
    [self addButton:@"doc.on.clipboard" tool:ToolPaste];
    [self addButton:@"arrow.right.doc.on.clipboard" tool:ToolCopy];
    [self addDelimiter:YES];
    [self addButton:@"keyboard" tool:ToolTextLine];
    [self addButton:@"sdcard" tool:ToolSDCard];
//    [self addButton:@"display" tool:ToolGEOS];
    [self addButton:@"speedometer" tool:ToolWarp];
    [self addDelimiter:YES];
    [self addButton:@"restart.circle" tool:ToolReset];

    return self;
}

@end
