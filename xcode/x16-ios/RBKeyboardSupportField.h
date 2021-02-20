//
//  RBKeyboardSupportField.h
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#import <UIKit/UIKit.h>
#include "platform_virtual_keys.h"

typedef void (^RBKeyboardSupportFieldCallback)(int, int, BOOL);

@interface RBKeyboardSupportField : UITextField <UITextFieldDelegate>

@property (nonatomic, copy) RBKeyboardSupportFieldCallback callback;

@end
