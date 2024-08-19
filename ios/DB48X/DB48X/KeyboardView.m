// ****************************************************************************
//  KeyboardView.m                                                DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Handles touches on the virtual keyboard
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms outlined in the LICENSE.txt file
//
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

#import "KeyboardView.h"
#import "ViewController.h"
#import "AppSettings.h"

#include "dmcp.h"
#include "sim-dmcp.h"


@implementation KeyboardView
// ----------------------------------------------------------------------------
//   The keyboard view must keep track of which keys happened last
// ----------------------------------------------------------------------------
{
    UISelectionFeedbackGenerator *feedback;
    int lastKey;
    bool upArrowHeld;
    bool downArrowHeld;
}

- (void)touchesBegan:(NSSet<UITouch*> *)touches withEvent:(UIEvent *)event
// ----------------------------------------------------------------------------
//   A touch happens on the virtual keyboard
// ----------------------------------------------------------------------------
{
    for (UITouch *touch in touches)
    {
        CGPoint pos = [touch locationInView:self];
        CGFloat relx = pos.x / self.frame.size.width;
        CGFloat rely = pos.y / self.frame.size.height;

        for (struct tapmap *ptr = tapMap; ptr->key; ptr++)
        {
            const CGFloat dx = 0.03;
            const CGFloat dy = 0.03;
            if ((relx >= ptr->left - dx) && (relx <= ptr->right + dx) &&
                (rely >= ptr->top - dy) && (rely <= ptr->bot + dy))
            {
                int key = ptr->keynum;
                if (key != lastKey)
                {
                    if (theAppSettings.hapticFeedback)
                    {
                        feedback = [[UISelectionFeedbackGenerator alloc] init];

                        // Prepare the generator when the gesture begins.
                        [feedback selectionChanged];
                    }

                    if (key == KEY_UP)
                        upArrowHeld = true;
                    else if (key == KEY_DOWN)
                        downArrowHeld = true;
                    else if (upArrowHeld)
                        key_push(KEY_UP);
                    else if (downArrowHeld)
                        key_push(KEY_DOWN);
                    else
                        key_push(0);
                    shift_held = upArrowHeld;
                    alt_held = downArrowHeld;
                    key_push(key);
                    lastKey = key;
                }
            }
        }
    }
}


- (void)touchesEnded:(NSSet<UITouch*> *)touches withEvent:(UIEvent *)event
// ----------------------------------------------------------------------------
//   Touches terminated on the virtual keyboard
// ----------------------------------------------------------------------------
{
    for (UITouch *touch in touches)
    {
        CGPoint pos = [touch locationInView:self];
        CGFloat relx = pos.x / self.frame.size.width;
        CGFloat rely = pos.y / self.frame.size.height;

        int key = 0;
        for (struct tapmap *ptr = tapMap; ptr->key; ptr++)
        {
            if ((relx >= ptr->left) && (relx <= ptr->right) &&
                (rely >= ptr->top) && (rely <= ptr->bot))
            {
                key = ptr->keynum;
                if (key == KEY_UP)
                    upArrowHeld = false;
                else if (key == KEY_DOWN)
                    downArrowHeld = false;
            }
        }
    }
    if (lastKey)
    {
        key_push(0);
        lastKey = 0;
        feedback = nil;
    }
}

@end
