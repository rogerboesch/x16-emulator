//
//  RBAlert.m
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#import "RBAlert.h"

@implementation RBAlert

+ (void)handleYes {
}

+ (void)handleNo {
}

+ (void)runYesNo:(UIViewController *)parent title:(NSString *)title text:(NSString *)text completion:(void (^)(BOOL))completion {
    UIAlertController* alert = [UIAlertController alertControllerWithTitle:title message:text
                                preferredStyle:UIAlertControllerStyleAlert];

    UIAlertAction* yesButton = [UIAlertAction
                               actionWithTitle:@"Yes"
                               style:UIAlertActionStyleDefault
                               handler:^(UIAlertAction * action) {
                                 completion(YES);
                               }];

    UIAlertAction* noButton = [UIAlertAction
                              actionWithTitle:@"No"
                              style:UIAlertActionStyleDefault
                              handler:^(UIAlertAction * action) {
                                completion(NO);
                              }];

    [alert addAction:yesButton];
    [alert addAction:noButton];

    [parent presentViewController:alert animated:YES completion:nil];
}

+ (void)run:(UIViewController *)parent title:(NSString *)title text:(NSString *)text buttons:(NSArray*)buttons completion:(void (^)(int))completion {
    UIAlertController * alert = [UIAlertController alertControllerWithTitle:title message:text
                                preferredStyle:UIAlertControllerStyleAlert];

    for (int i = 0; i < buttons.count; i++) {
        UIAlertAction* action = [UIAlertAction actionWithTitle:[buttons objectAtIndex:i] style:UIAlertActionStyleDefault
                                   handler:^(UIAlertAction * action) {
                                     completion(i);
                                   }];
        [alert addAction:action];
    }

    [parent presentViewController:alert animated:YES completion:nil];
}

+ (void)error:(UIViewController *)parent text:(NSString *)text {
    UIAlertController * alert = [UIAlertController alertControllerWithTitle:@"Error" message:text
                                preferredStyle:UIAlertControllerStyleAlert];

    UIAlertAction* yesButton = [UIAlertAction
                               actionWithTitle:@"OK"
                               style:UIAlertActionStyleDefault
                               handler:^(UIAlertAction * action) {
                               }];
    [alert addAction:yesButton];

    [parent presentViewController:alert animated:YES completion:nil];
}

@end

