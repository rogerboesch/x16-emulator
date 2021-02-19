
#import <UIKit/UIKit.h>
#include "platform_virtual_keys.h"

typedef void (^RBKeyboardSupportFieldCallback)(int, int, BOOL);

@interface RBKeyboardSupportField : UITextField <UITextFieldDelegate>

@property (nonatomic, copy) RBKeyboardSupportFieldCallback callback;

@end
