//
//  ViewController.h
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#import "ViewController.h"
#import "Toolbar.h"
#import "video.h"
#import "RBRenderView.h"
#import "RBKeyboardSupportField.h"
#import "RBGhost.h"

extern void x16_send_event(RBEvent evt);

ViewController* INSTANCE_OF_VIEWCONTROLLER = NULL;

@interface ViewController ()

@property (nonatomic, retain) Toolbar* toolbar;
@property (nonatomic, retain) RBRenderView* renderView;
@property (nonatomic, retain) RBKeyboardSupportField* supportField;

@property (nonatomic, retain) NSString* cloudFilename;
@property (nonatomic) BOOL cloudPickerActive;

@end

@implementation ViewController

// MARK: - Cloud support

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller {
    self.cloudPickerActive = NO;
}

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    if (urls.count > 0) {
        NSURL* originalURL = [urls objectAtIndex:0];

        [originalURL startAccessingSecurityScopedResource];
        NSFileCoordinator *fileCoordinator = [[NSFileCoordinator alloc] init];
        NSError *error = nil;
        [fileCoordinator coordinateReadingItemAtURL:originalURL options:NSFileCoordinatorReadingForUploading error:&error byAccessor:^(NSURL *newURL) {
            // Read contant
            NSData* data = [NSData dataWithContentsOfURL:originalURL];
            if (data != nil) {
                // Save to documents
                NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask, YES);
                NSString* documentsDirectory = [paths objectAtIndex:0];
                NSString* path = [NSString stringWithFormat:@"%@/TEMP", documentsDirectory];
                [data writeToFile:path atomically:NO];

                self.cloudFilename = path;
            }
            else {
                self.cloudFilename = @"";
            }
            
            [originalURL stopAccessingSecurityScopedResource];
            
            self.cloudPickerActive = NO;
        }];
    }
    else {
        self.cloudFilename = @"";
        self.cloudPickerActive = NO;
    }
}

- (void)loadFromCloud {
    UIDocumentPickerViewController *documentProvider;
    documentProvider = [[UIDocumentPickerViewController alloc] initWithDocumentTypes:[NSArray arrayWithObjects:@"public.item", nil] inMode: UIDocumentPickerModeOpen];
    documentProvider.delegate = self;
    documentProvider.modalPresentationStyle = UIModalPresentationOverFullScreen;
    
    self.cloudPickerActive = YES;
    [self presentViewController:documentProvider animated:YES completion:NULL];
}

// MARK: - Mouse support

- (void)handleSingleTap:(UITapGestureRecognizer *)recognizer {
    CGPoint location = [recognizer locationInView:recognizer.view];
    int x = SCREEN_WIDTH / self.renderView.frame.size.width * location.x;
    int y = SCREEN_HEIGHT / self.renderView.frame.size.height * location.y;

    RBEvent evt;
    evt.type = RBEVT_MouseMoved;
    evt.code = RBVK_LShift;
    evt.ch = 0;
    evt.control = 0;
    evt.shift = 0;
    evt.alt = 0;
    evt.mouseX = x;
    evt.mouseY = y;
    
    x16_send_event(evt);
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
    rect.origin.y = 70;

    float factor = 640 / rect.size.width;
    rect.size.height = MIN(rect.size.height-70, 480 / factor);
    
    self.renderView.frame = rect;

    rect = self.view.bounds;
    rect.size.height = 30;
    rect.origin.y = 35;
    self.toolbar.frame = rect;
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    
    [self.renderView startUsingWidth:SCREEN_WIDTH height:SCREEN_HEIGHT];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.view.backgroundColor = UIColor.blackColor;
    self.cloudFilename = @"";
    INSTANCE_OF_VIEWCONTROLLER = self;
    
    self.toolbar = [[Toolbar alloc] initWithFrame:CGRectZero];
    [self.view addSubview:self.toolbar];

    self.renderView = [[RBRenderView alloc] initWithFrame:CGRectZero];
    [self.view addSubview:self.renderView];
    self.renderView.backgroundColor = UIColor.blueColor;
    
    self.supportField = [[RBKeyboardSupportField alloc] initWithFrame:CGRectMake(2400, 2400, 100, 20)];
    self.supportField.keyboardAppearance = UIKeyboardAppearanceDark;
    [self.view addSubview:self.supportField];

    self.supportField.callback = ^(int ch, int code, BOOL ctrlPressed) {
        if (ctrlPressed && ch == 'x') {
            [RBGhost pressKey:0 code:RBVK_Escape ctrlPressed: false];
        }
        else {
            [RBGhost pressKey:ch code:code ctrlPressed: ctrlPressed];
        }
    };
    
    UITapGestureRecognizer *singleFingerTap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleSingleTap:)];
    [self.renderView addGestureRecognizer:singleFingerTap];
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
