//
//  Toolbar.m
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#import "Toolbar.h"
#import "RBGhost.h"
#import "platform_ios.h"

extern void machine_reset(void);
extern void machine_toggle_warp(void);
extern void machine_toggle_geos(void);
extern void machine_paste(char*);

@implementation Toolbar

- (void)tap:(UIButton *)sender {
    switch (sender.tag) {
        case 1:
            [RBGhost pressKey:0 code:RBVK_Escape ctrlPressed:NO];
            break;
        case 2:
            machine_toggle_warp();
            break;
        case 3:
            machine_reset();
            break;
        case 4:
            machine_paste(platform_get_from_clipboard());
            break;
        case 5:
            machine_toggle_geos();
            break;
        case 6:
            [RBGhost pressKey:0 code:RBVK_F4 ctrlPressed:NO];
    }
}

- (void)addButton:(NSString *)imageName tag:(int)tag {
    UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
    [button setImage:[UIImage systemImageNamed:imageName] forState:UIControlStateNormal];
    button.frame = CGRectMake(0, 0, 40, 40);
    button.tag = tag;
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

    [self addButton:@"stop" tag:1];
    [self addButton:@"paperclip.circle" tag:4];
    [self addDelimiter:YES];
    [self addButton:@"keyboard" tag:6];
    [self addDelimiter:YES];
    [self addButton:@"display" tag:5];
    [self addButton:@"speedometer" tag:2];
    [self addDelimiter:NO];
    [self addButton:@"restart.circle" tag:3];

    return self;
}

@end
