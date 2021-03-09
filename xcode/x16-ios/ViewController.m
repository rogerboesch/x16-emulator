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
#import <UIKit/UIGestureRecognizerSubclass.h>

extern void x16_send_event(RBEvent evt);

ViewController* INSTANCE_OF_VIEWCONTROLLER = NULL;

@interface TouchDownGestureRecognizer : UIGestureRecognizer
@end

@implementation TouchDownGestureRecognizer

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    self.state = UIGestureRecognizerStateBegan;
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    self.state = UIGestureRecognizerStateChanged;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    self.state = UIGestureRecognizerStateEnded;
}

@end

@interface ViewController ()

@property (nonatomic, retain) Toolbar* toolbar;
@property (nonatomic, retain) RBRenderView* renderView;
@property (nonatomic, retain) RBKeyboardSupportField* supportField;

@property (nonatomic, retain) NSString* externalFilename;
@property (nonatomic) BOOL documentPickerActive;

@end

@implementation ViewController

#ifndef APP_STORE

// MARK: - Cloud support

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller {
    self.documentPickerActive = NO;
}

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    if (urls.count > 0) {
        NSURL* originalURL = [urls objectAtIndex:0];

        [originalURL startAccessingSecurityScopedResource];
        NSFileCoordinator *fileCoordinator = [[NSFileCoordinator alloc] init];
        NSError *error = nil;
        [fileCoordinator coordinateReadingItemAtURL:originalURL options:NSFileCoordinatorReadingForUploading error:&error byAccessor:^(NSURL *newURL) {
            // Read contant
            NSError* error = NULL;
            NSString* str = [NSString stringWithContentsOfURL:originalURL encoding:NSUTF8StringEncoding error:&error];
            if (str != NULL && error == NULL) {
                NSData* data = [NSData dataWithContentsOfURL:originalURL];
                if (data != nil) {
                    // Save to documents
                    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask, YES);
                    NSString* documentsDirectory = [paths objectAtIndex:0];
                    NSString* path = [NSString stringWithFormat:@"%@/TEMP", documentsDirectory];
                    [data writeToFile:path atomically:NO];

                    self.externalFilename = path;
                }
            }
            else {
                self.externalFilename = @"";
            }
            
            [originalURL stopAccessingSecurityScopedResource];
            
            self.documentPickerActive = NO;
        }];
    }
    else {
        self.externalFilename = @"";
        self.documentPickerActive = NO;
    }
}

- (void)loadExternalFile {
    UIDocumentPickerViewController *documentProvider;
    documentProvider = [[UIDocumentPickerViewController alloc] initWithDocumentTypes:[NSArray arrayWithObjects:@"public.item", nil] inMode: UIDocumentPickerModeOpen];
    documentProvider.delegate = self;
    documentProvider.modalPresentationStyle = UIModalPresentationOverFullScreen;
    
    self.documentPickerActive = YES;
    [self presentViewController:documentProvider animated:YES completion:NULL];
}

#endif

// MARK: - Mouse support

- (void)handleTouch:(TouchDownGestureRecognizer *)recognizer {
    CGPoint location = [recognizer locationInView:recognizer.view];
    int x = SCREEN_WIDTH / self.renderView.frame.size.width * location.x;
    int y = SCREEN_HEIGHT / self.renderView.frame.size.height * location.y;
    
    RBEvent evt;
    evt.type = RBEVT_None;
    evt.code = RBVK_LShift;
    evt.ch = 0;
    evt.control = 0;
    evt.shift = 0;
    evt.alt = 0;
    evt.mouseX = x;
    evt.mouseY = y;
    evt.mouseBtn = 0;

    switch (recognizer.state) {
        case UIGestureRecognizerStateBegan:
            evt.type = RBEVT_MouseButtonPressed;
            break;
        case UIGestureRecognizerStateChanged:
            evt.type = RBEVT_MouseMoved;
            break;
        case UIGestureRecognizerStateEnded:
            evt.type = RBEVT_MouseButtonReleased;
            break;
            
        default:
            return;
    }

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
    self.externalFilename = @"";
    INSTANCE_OF_VIEWCONTROLLER = self;
    
    self.toolbar = [[Toolbar alloc] initWithFrame:CGRectZero];
    self.toolbar.viewController = self;
    [self.view addSubview:self.toolbar];

    self.renderView = [[RBRenderView alloc] initWithFrame:CGRectZero];
    [self.view addSubview:self.renderView];
    self.renderView.backgroundColor = UIColor.blueColor;
    
    self.supportField = [[RBKeyboardSupportField alloc] initWithFrame:CGRectMake(2400, 2400, 100, 20)];
    self.supportField.keyboardAppearance = UIKeyboardAppearanceDark;
    [self.view addSubview:self.supportField];

    self.supportField.callback = ^(int ch, int code, BOOL ctrlPressed) {
        if (ctrlPressed && ch == 'c') {
            [RBGhost pressKey:0 code:RBVK_Escape ctrlPressed: false];
        }
        else {
            [RBGhost pressKey:ch code:code ctrlPressed: ctrlPressed];
        }
    };
    
    TouchDownGestureRecognizer *touch = [[TouchDownGestureRecognizer alloc] initWithTarget:self action:@selector(handleTouch:)];
    [self.renderView addGestureRecognizer:touch];
}

@end

// TODO: This is a hacky implementation, use later pipes o.ae.

#ifndef APP_STORE

char* platform_wait_for_external_filename() {
    while (INSTANCE_OF_VIEWCONTROLLER.documentPickerActive) {
        usleep(100);
    }

    return (char *)[INSTANCE_OF_VIEWCONTROLLER.externalFilename UTF8String];
}

void platform_load_external_file() {
    dispatch_sync(dispatch_get_main_queue(), ^{
        [INSTANCE_OF_VIEWCONTROLLER loadExternalFile];
    });
}

void platform_load_external_file_async() {
    dispatch_async(dispatch_get_main_queue(), ^{
        [INSTANCE_OF_VIEWCONTROLLER loadExternalFile];
    });
}

#endif

