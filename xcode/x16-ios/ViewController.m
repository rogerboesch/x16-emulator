
#import "ViewController.h"
#import "RBRenderView.h"
#import "RBKeyboardSupportField.h"
#import "RBGhost.h"


ViewController* INSTANCE_OF_VIEWCONTROLLER = NULL;

@interface ViewController ()

@property (nonatomic, retain) RBRenderView* renderView;
@property (nonatomic, retain) RBKeyboardSupportField* supportField;

@property (nonatomic, retain) NSString* cloudFilename;
@property (nonatomic) BOOL cloudPickerActive;

@end

@implementation ViewController

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller {
    self.cloudPickerActive = NO;
}

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    if (urls.count > 0) {
        self.cloudFilename = [[urls objectAtIndex:0] path];
        NSLog(@"Clod file selecetd: %@", self.cloudFilename);
    }
    else {
        self.cloudFilename = @"";
    }
    
    self.cloudPickerActive = NO;
}

- (void)loadFromCloud {
    UIDocumentPickerViewController *documentProvider;
    documentProvider = [[UIDocumentPickerViewController alloc] initWithDocumentTypes:[NSArray arrayWithObjects:@"public.item", nil] inMode: UIDocumentPickerModeOpen];
    documentProvider.delegate = self;
    documentProvider.modalPresentationStyle = UIModalPresentationOverFullScreen;
    
    self.cloudPickerActive = YES;
    [self presentViewController:documentProvider animated:YES completion:NULL];
}

- (BOOL)cloudPickerIsACtive {
    return self.cloudPickerActive;
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    [self.supportField becomeFirstResponder];
}

- (void)viewWillLayoutSubviews {
    [super viewWillLayoutSubviews];

    CGRect rect = self.view.bounds;
    rect.origin.x += 10;
    rect.size.width -= 20;
    rect.origin.y = 100;

    float factor = 640 / rect.size.width;
    rect.size.height = 400 / factor;
    
    self.renderView.frame = rect;
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    
    [self.renderView start];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.cloudFilename = @"";
    
    INSTANCE_OF_VIEWCONTROLLER = self;
    
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

// TODO: This is a hacky implementation, use later pipes o.ae.

char* platform_wait_for_cloud_filename() {
    while (INSTANCE_OF_VIEWCONTROLLER.cloudPickerActive) {
        usleep(100);
    }

    return (char *)[INSTANCE_OF_VIEWCONTROLLER.cloudFilename UTF8String];
}

void platform_load_from_cloud() {
    dispatch_sync(dispatch_get_main_queue(), ^{
        [INSTANCE_OF_VIEWCONTROLLER loadFromCloud];
    });
}
