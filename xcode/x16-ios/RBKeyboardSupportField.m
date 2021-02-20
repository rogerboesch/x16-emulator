//
//  RBKeyboardSupportField.m
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#import "RBKeyboardSupportField.h"
#import "platform_virtual_keys.h"

#define CHARACTERS @"abcdefghijklmnopqrstuvwxyz"

@implementation RBKeyboardSupportField

// -----------------------------------------------------------------------------
#pragma mark - Callback

- (void)pressKey:(int)asciiCode keyCode:(int)keyCode ctrlKeyPressed:(BOOL)ctrlKeyPressed {
    if (self.callback != NULL) {
        self.callback(asciiCode, keyCode, ctrlKeyPressed);
    }
}

// -----------------------------------------------------------------------------
#pragma mark - Field delegate

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    [self pressKey:0 keyCode:RBVK_Return ctrlKeyPressed:false];

    return NO;
}

- (BOOL)textFieldShouldEndEditing:(UITextField *)textField {
    return YES;
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string {
    int asciiCode = -1;
    
    if (string.length == 0) {
        const char* _ch = [string cStringUsingEncoding:NSUTF8StringEncoding];
        int isBackSpace = strcmp(_ch, "\b");

        if (isBackSpace == -8) {
            NSLog(@"Backspace was pressed");
            asciiCode = RBVK_BackSpace;
        }
    }
    else {
        asciiCode = [string characterAtIndex:0];
    }

    if (asciiCode == RBVK_BackSpace) {
        [self pressKey:0 keyCode:RBVK_BackSpace ctrlKeyPressed:false];
    }
    else {
        [self pressKey:asciiCode keyCode:RBVK_Unknown ctrlKeyPressed:false];
    }

    if (asciiCode == RBVK_BackSpace && textField.text.length < 2) {
        return NO;
    }

    return YES;
}

// -----------------------------------------------------------------------------
#pragma mark - Special key support

- (NSArray *)keyCommands {
    UIKeyCommand *keyLeft = [UIKeyCommand keyCommandWithInput:UIKeyInputLeftArrow modifierFlags:0 action:@selector(keyLeft:)];
    UIKeyCommand *keyRight = [UIKeyCommand keyCommandWithInput:UIKeyInputRightArrow modifierFlags:0 action:@selector(keyRight:)];
    UIKeyCommand *keyUp = [UIKeyCommand keyCommandWithInput:UIKeyInputUpArrow modifierFlags:0 action:@selector(keyUp:)];
    UIKeyCommand *keyDown = [UIKeyCommand keyCommandWithInput:UIKeyInputDownArrow modifierFlags:0 action:@selector(keyDown:)];
    UIKeyCommand *keyPageUp = [UIKeyCommand keyCommandWithInput:UIKeyInputPageUp modifierFlags:0 action:@selector(keyPageUp:)];
    UIKeyCommand *keyPageDown = [UIKeyCommand keyCommandWithInput:UIKeyInputPageDown modifierFlags:0 action:@selector(keyPageDown:)];
    UIKeyCommand *esc = [UIKeyCommand keyCommandWithInput:UIKeyInputEscape modifierFlags:0 action:@selector(keyEsc:)];
    
    NSMutableArray* keys = [[NSMutableArray alloc] initWithObjects: esc, keyLeft, keyRight, keyUp, keyDown, keyPageDown, keyPageUp, nil];

    for (int i = 0; i < CHARACTERS.length; ++i) {
        NSString *ch = [NSString stringWithFormat:@"%c", [CHARACTERS characterAtIndex:i]];

        UIKeyCommand *keyCtrl = [UIKeyCommand keyCommandWithInput:ch modifierFlags:UIKeyModifierControl action:@selector(keyCtrl:)];
        [keys addObject:keyCtrl];
    }

    return [[NSArray alloc] initWithArray:keys];
}

- (void)keyLeft:(UIKeyCommand *)keyCommand {
    [self pressKey:0 keyCode:RBVK_Left ctrlKeyPressed:false];
}

- (void)keyRight:(UIKeyCommand *)keyCommand {
    [self pressKey:0 keyCode:RBVK_Right ctrlKeyPressed:false];
}

- (void)keyUp:(UIKeyCommand *)keyCommand {
    [self pressKey:0 keyCode:RBVK_Up ctrlKeyPressed:false];
}

- (void)keyDown:(UIKeyCommand *)keyCommand {
    [self pressKey:0 keyCode:RBVK_Down ctrlKeyPressed:false];
}

- (void)keyPageUp:(UIKeyCommand *)keyCommand {
    [self pressKey:0 keyCode:RBVK_PageUp ctrlKeyPressed:false];
}

- (void)keyPageDown:(UIKeyCommand *)keyCommand {
    [self pressKey:0 keyCode:RBVK_PageDown ctrlKeyPressed:false];
}

- (void)keyEsc:(UIKeyCommand *)keyCommand {
    [self pressKey:0 keyCode:RBVK_Escape ctrlKeyPressed:false];
}

- (void)keyCtrl:(UIKeyCommand *)keyCommand {
    NSString *ch = [keyCommand input];
    int pos = (int)[CHARACTERS rangeOfString:ch].location;
    if (pos == NSNotFound) {
        return;
    }
    
    [self pressKey:[CHARACTERS characterAtIndex:pos] keyCode:RBVK_Unknown ctrlKeyPressed:true];
}

// -----------------------------------------------------------------------------
#pragma mark - Init

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];

    self.delegate = self;
    self.autocorrectionType = UITextAutocorrectionTypeNo;
    self.text = @"-"; // Need minium of 1 character to support backspace
    
    return self;
}

@end
