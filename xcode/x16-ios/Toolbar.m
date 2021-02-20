//
//  Toolbar.m
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#import "Toolbar.h"
#import "RBGhost.h"

extern void machine_reset(void);

@implementation Toolbar

- (void)tap:(UIButton *)sender {
    switch (sender.tag) {
        case 1:
            [RBGhost pressKey:0 code:RBVK_Escape ctrlPressed:NO];
            break;
        case 2:
            machine_reset();
            break;
    }
}

- (void)addButton:(NSString *)imageName {
    int index = (int)self.items.count+1;
    
    UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
    [button setImage:[UIImage systemImageNamed:imageName] forState:UIControlStateNormal];
    button.frame = CGRectMake(0, 0, 40, 40);
    button.tag = index;
    [button addTarget:self action:@selector(tap:) forControlEvents:UIControlEventTouchUpInside];
    UIBarButtonItem* item = [[UIBarButtonItem alloc] initWithCustomView:button];
    
    NSMutableArray* newItems = [NSMutableArray arrayWithArray:self.items];
    [newItems addObject:item];
    self.items = newItems;
}

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];

    self.barTintColor = UIColor.blackColor;

    [self addButton:@"stop"];
    [self addButton:@"restart.circle"];
    
    return self;
}

@end
