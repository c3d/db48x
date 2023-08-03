// ****************************************************************************
//  stack.cc                                                      DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Rendering of the objects on the stack
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
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

#include "stack.h"

#include "graphics.h"
#include "user_interface.h"
#include "renderer.h"
#include "runtime.h"
#include "target.h"
#include "settings.h"

#include <dmcp.h>


stack    Stack;

using coord = graphics::coord;
using size  = graphics::size;


RECORDER(tests, 16, "Information about tests");

stack::stack()
// ----------------------------------------------------------------------------
//   Constructor does nothing at the moment
// ----------------------------------------------------------------------------
#if SIMULATOR
    : history(), writer(0), reader(0)
#endif
{
}


static inline uint countDigits(uint value)
// ----------------------------------------------------------------------------
//   Count how many digits we need to display a value
// ----------------------------------------------------------------------------
{
    uint result = 1;
    while (value /= 10)
        result++;
    return result;
}


void stack::draw_stack()
// ----------------------------------------------------------------------------
//   Draw the stack on screen
// ----------------------------------------------------------------------------
{
    font_p font       = Settings.result_font();
    font_p hdrfont    = HeaderFont;
    font_p idxfont    = HelpFont;
    size   lineHeight = font->height();
    size   idxHeight  = idxfont->height();
    coord  top        = hdrfont->height() + 2;
    coord  bottom     = ui.stack_screen_bottom() - 1;
    uint   depth      = rt.depth();
    uint   digits     = countDigits(depth);
    coord  hdrx       = idxfont->width('0') * digits + 2;
    size   avail      = LCD_W - hdrx - 5;

    Screen.fill(0, top, LCD_W, bottom - 1, pattern::white);
    if (!depth)
        return;

    rect clip      = Screen.clip();

    Screen.fill(hdrx, top, hdrx, bottom, pattern::gray50);
    if (rt.editing())
        Screen.fill(0, bottom, LCD_W, bottom, pattern::gray50);

    char buf[8];
    coord y = bottom;
#ifdef SIMULATOR
    extern int last_key;
    if (depth == 0)
        output(last_key, object::ID_object, nullptr, 0);
#endif
    for (uint level = 0; level < depth; level++)
    {
        y -= lineHeight;
        if (y + lineHeight  <= top)
            break;

        coord ytop = y < top ? top : y;
        coord yb   = y + lineHeight-1;
        Screen.clip(0, ytop, LCD_W, yb);

        size idxOffset = (lineHeight - idxHeight) / 2;
        snprintf(buf, sizeof(buf), "%d", level + 1);
        size w = idxfont->width(utf8(buf));
        Screen.text(hdrx - w, y + idxOffset, utf8(buf), idxfont);

        object_g obj = rt.stack(level);
        renderer r(nullptr, ~0U, true);
        size_t   len = obj->render(r);
        utf8     out = r.text();
#ifdef SIMULATOR
        if (level == 0)
        {
            output(last_key, obj->type(), out, len);
            record(tests,
                   "Key %d X-reg %+s size %u %s",
                   last_key, object::name(obj->type()), len, out);
        }
#endif

        w = font->width(out, len);
        if (w > avail)
        {
            unicode sep   = L'…';
            coord   x     = hdrx + 5;
            coord   split = 200;
            coord   skip  = font->width(sep) * 3 / 2;
            size    offs  = lineHeight / 5;

            Screen.clip(x, ytop, split, yb);
            Screen.text(x, y, out, len, font);
            Screen.clip(split, ytop, split + skip, yb);
            Screen.glyph(split + skip/8, y - offs, sep, font, pattern::gray50);
            Screen.clip(split+skip, y, LCD_W, yb);
            Screen.text(LCD_W - w, y, out, len, font);
        }
        else
        {
            Screen.text(LCD_W - w, y, out, len, font);
        }

        font = Settings.stack_font();
        lineHeight = font->height();
    }
    Screen.clip(clip);
}
