
#import "ViewController.h"
#import "RBRenderView.h"
#import "RBKeyboardSupportField.h"
#import "RBGhost.h"

@interface ViewController ()

@property (nonatomic, retain) RBRenderView* renderView;
@property (nonatomic, retain) RBKeyboardSupportField* supportField;

@end

@implementation ViewController

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    [self.supportField becomeFirstResponder];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];

    CGRect rect = self.view.bounds;
    rect.origin.x += 10;
    rect.size.width -= 20;
    rect.origin.y = 100;

    float factor = 640 / rect.size.width;
    rect.size.height = 400 / factor;
    
    self.renderView.frame = rect;
    
    [self.renderView start];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.view.backgroundColor = UIColor.blackColor;
    
    self.renderView = [[RBRenderView alloc] initWithFrame:CGRectZero];
    [self.view addSubview:self.renderView];

    self.supportField = [[RBKeyboardSupportField alloc] initWithFrame:CGRectMake(2400, 2400, 100, 20)];
    self.supportField.keyboardAppearance = UIKeyboardAppearanceDark;
    [self.view addSubview:self.supportField];

    self.supportField.callback = ^(int ch, int code, BOOL ctrlPressed) {
        [RBGhost pressKey:ch code:code ctrlPressed: ctrlPressed];
    };
}

@end
