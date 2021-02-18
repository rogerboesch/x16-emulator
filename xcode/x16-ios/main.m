
#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int platform_argc;
char** platform_argv;

int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;

    platform_argc = argc;
    platform_argv = argv;
    
    @autoreleasepool {
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }

    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
