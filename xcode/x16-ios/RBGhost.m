//
//  rb_ghost.m
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#import "RBGhost.h"
#import "platform_ios.h"
#import "platform_virtual_keys.h"

extern void x16_send_event(RBEvent evt);

NSOperationQueue* s_ghostQueue = NULL;

@implementation RBGhost

+ (void)sendEvent:(RBEvent)evt {
    if (evt.code == RBVK_Unknown) {
        if (evt.type == RBEVT_KeyPressed || evt.type == RBEVT_KeyReleased) {
            // Test ascii characters
            char ch = toupper(evt.ch);
            
            if ((int)ch > 0) {
                int keyCode = character_to_vk(ch);
                evt.code = keyCode;
                evt.ch = ch;
                evt.shift = 0;
                
                int nonShiftCode = get_non_shift_key(ch);
                if (nonShiftCode != RBVK_Unknown) {
                    evt.code = nonShiftCode;
                }

            }
        }
    }
    
    x16_send_event(evt);
    usleep(20*1000);
}

+ (void)typeInCharacter:(char)ch ctrlPressed:(BOOL)ctrlPressed {
    BOOL sendShiftKey = NO;
    
    if (is_shift_key_used(ch))
        sendShiftKey = YES;

    if (sendShiftKey) {
        [s_ghostQueue addOperationWithBlock:^{
            RBEvent evt;
            evt.type = RBEVT_KeyPressed;
            evt.code = RBVK_LShift;
            evt.ch = 0;
            evt.control = 0;
            evt.shift = 1;
            evt.alt = 0;

            [RBGhost sendEvent:evt];
        }];
    }

    [s_ghostQueue addOperationWithBlock:^{
        RBEvent evt;
        evt.type = RBEVT_KeyPressed;
        evt.code = RBVK_Unknown;
        evt.ch = ch;
        evt.control = ctrlPressed;
        evt.shift = 0;
        evt.alt = 0;

        [RBGhost sendEvent:evt];
    }];
    
    [s_ghostQueue addOperationWithBlock:^{
        RBEvent evt;
        evt.type = RBEVT_KeyReleased;
        evt.ch = ch;
        
        [RBGhost sendEvent:evt];
    }];

    if (sendShiftKey) {
        [s_ghostQueue addOperationWithBlock:^{
            RBEvent evt;
            evt.type = RBEVT_KeyReleased;
            evt.code = RBVK_LShift;
            evt.ch = 0;
            evt.control = 0;
            evt.shift = 1;
            evt.alt = 0;

            [RBGhost sendEvent:evt];
        }];
    }
}

+ (void)typeInCode:(int)code ctrlPressed:(BOOL)ctrlPressed {
    [s_ghostQueue addOperationWithBlock:^{
        RBEvent evt;
        evt.type = RBEVT_KeyPressed;
        evt.code = code;
        evt.ch = '\0';
        evt.control = ctrlPressed;
        evt.shift = 0;
        evt.alt = 0;

        [RBGhost sendEvent:evt];
    }];
    
    [s_ghostQueue addOperationWithBlock:^{
        RBEvent evt;
        evt.type = RBEVT_KeyReleased;
        evt.ch = '\0';
        evt.code = code;

        [RBGhost sendEvent:evt];
    }];
}

+ (void)typeIn:(NSString *)command {
    if (s_ghostQueue == NULL) {
        s_ghostQueue = [NSOperationQueue new];
        s_ghostQueue.maxConcurrentOperationCount = 1;
    }
    
    for (int i = 0; i < command.length; i++) {
        char ch = [command characterAtIndex:i];
        [RBGhost typeInCharacter:ch ctrlPressed:false];
    }

    [RBGhost typeInCharacter:'\r' ctrlPressed:false];
}

+ (void)pressKey:(int)ch code:(int)code ctrlPressed:(BOOL)ctrlPressed {
    if (s_ghostQueue == NULL) {
        s_ghostQueue = [NSOperationQueue new];
        s_ghostQueue.maxConcurrentOperationCount = 1;
    }

    if (code != RBVK_Unknown) {
        [RBGhost typeInCode:code ctrlPressed:ctrlPressed];
    }
    else {
        [RBGhost typeInCharacter:ch ctrlPressed:ctrlPressed];
    }
}

@end
