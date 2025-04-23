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

#include "blitter.h"
#include "dmcp.h"
#include "expression.h"
#include "grob.h"
#include "renderer.h"
#include "runtime.h"
#include "settings.h"
#include "target.h"
#include "tests.h"
#include "user_interface.h"
#include "utf8.h"


stack    Stack;

using coord = blitter::coord;
using size  = blitter::size;


stack::stack()
// ----------------------------------------------------------------------------
//   Constructor does nothing at the moment
// ----------------------------------------------------------------------------
    : interactive(0), interactive_base(0)
#if SIMULATOR
    , history(), writer(0), reader(0)
#endif  // SIMULATOR
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


uint stack::draw_stack()
// ----------------------------------------------------------------------------
//   Draw the stack on screen
// ----------------------------------------------------------------------------
{
    // Do not redraw if there is an error
    if (rt.error())
    {
        error_save errs;
        return draw_stack();
    }

    font_p font = interactive ? Settings.stack_font() : Settings.result_font();
    font_p idxfont    = HelpFont;
    size   lineHeight = font->height();
    size   idxHeight  = idxfont->height();
    size   idxOffset  = (lineHeight - idxHeight) / 2 - 2;
    coord  top        = ui.stack_screen_top();
    coord  bottom     = ui.stack_screen_bottom();
    uint   depth      = rt.depth();
    uint   digits     = countDigits(depth);
    coord  hdrx       = idxfont->width('0') * digits + 2;
    size   avail      = LCD_W - hdrx - 5;

    Screen.fill(0, top, LCD_W, bottom, Settings.StackBackground());
    if (rt.editing())
    {
        bottom--;
        Screen.fill(0, bottom, LCD_W, bottom, Settings.EditorLineForeground());
        bottom--;
    }
    if (!depth)
        return bottom;

    if (interactive)
    {
        uint irows = (bottom - top) / font->height();
        if (interactive < interactive_base + 1)
            interactive_base = interactive - 1;
        else if (interactive > interactive_base + irows)
            interactive_base = interactive - irows;
    }
    else
    {
        interactive_base = 0;
    }

    rect clip      = Screen.clip();
    Screen.fill(0, top, hdrx-1, bottom, Settings.StackLevelBackground());
    Screen.fill(hdrx, top, hdrx, bottom, Settings.StackLineForeground());

    coord    y         = bottom;
    coord    yresult   = y;
    bool     rgraph    = Settings.GraphicResultDisplay();
    bool     sgraph    = Settings.GraphicStackDisplay();
    auto     rfont     = Settings.ResultFont();
    auto     sfont     = Settings.StackFont();
    uint     rtime     = Settings.ResultGraphingTimeLimit();
    uint     stime     = Settings.StackGraphingTimeLimit();
    pattern  rfg       = Settings.ResultForeground();
    pattern  sfg       = Settings.StackForeground();
    pattern  rbg       = Settings.ResultBackground();
    pattern  sbg       = Settings.StackBackground();
    bool     rml       = Settings.MultiLineResult();
    bool     sml       = Settings.MultiLineStack();
    bool     autoScale = Settings.AutoScaleStack();
    grob_g   graph;
    text_g   rendered;
    object_g obj;
    object_g cached;
    char     buf[16];

    // Invalidate cache if settings changed
    static uint settingsHash = 0;
    uint hash = Settings.hash() ^ (interactive ? 0x4242 : 0) ;
    if (hash != settingsHash)
    {
        rt.uncache();
        settingsHash = hash;
    }

    for (uint level = interactive_base; level < depth; level++)
    {
        if (coord(y) <= top)
            break;

        font = (interactive || level != 0) ? Settings.stack_font() : Settings.result_font();
        lineHeight = font->height();

        obj        = rt.stack(level);
        cached     = rt.cached(level == 0, +obj);
        graph      = nullptr;

        size w     = 0;
        if (!interactive && (level ? sgraph : rgraph))
        {
            if (cached)
                if (grob_p gr = cached->as_graph())
                    graph = gr;

            if (graph && !obj->is_graph() &&
                graph->height() > size(bottom - top))
                graph = nullptr;

            if (!graph && !cached)
            {
                grapher g(avail - 2,
                          bottom - top,
                          level == 0 ? rfont : sfont,
                          grob::pattern::black,
                          grob::pattern::white,
                          true);
                g.duration = level == 0 ? rtime : stime;
                do
                {
                    graph = obj->graph(g);
                } while (!graph && !rt.error() && autoScale && g.reduce_font());

                if (graph)
                {
                    rt.cache(level == 0, +obj, +graph);
                    if (rgraph == sgraph && rfont == sfont)
                        rt.cache(level != 0, +obj, +graph);
                }
            }
            if (graph)
            {
                size gh = graph->height();
                if (lineHeight < gh)
                    lineHeight = gh;
                w = graph->width();

#ifdef SIMULATOR
                if (level == 0)
                {
                    extern int last_key;
                    bool     ml = level ? sml : rml;
                    renderer r(nullptr, ~0U, true, ml);
                    size_t   len = obj->render(r);
                    utf8     out = r.text();
                    int      key = last_key;
                    output(key, obj->type(), out, len);
                    record(tests_rpl,
                           "Key %d X-reg %+s size %u %s",
                           key, object::name(obj->type()), len, out);
                }
#endif // SIMULATOR
            }
        }

        y -= lineHeight;
        coord ytop = y < top ? top : y;
        coord yb   = y + lineHeight-1;
        Screen.clip(0, ytop, LCD_W, yb);

        pattern fg = level == 0 ? rfg : sfg;
        pattern bg = level == 0 ? rbg : sbg;
        if (graph)
        {
#if CONFIG_COLOR
            if (pixmap_p pix = graph->as<pixmap>())
            {
                pixmap::surface s = pix->pixels();
                Screen.copy(s, LCD_W - 2 - w, y);
            }
            else
#endif // CONFIG_COLOR
            {
                grob::surface s = graph->pixels();
                Screen.draw(s, LCD_W - 2 - w, y, fg);
                Screen.draw_background(s, LCD_W - 2 - w, y, bg);
            }
        }
        else
        {
            // Text rendering - Check caching
            bool   ml  = !interactive && (level ? sml : rml);
            size_t len = 0;
            utf8   out = utf8("");

            rendered = cached ? cached->as<text>() : nullptr;
            if (rendered)
            {
                out = rendered->value(&len);
            }
            else
            {
                // Text rendering
                renderer r(nullptr, ~0U, true, ml);
                len = obj->render(r);
                out = r.text();
                gcutf8 saveOut = out;
                rendered = text::make(out, len);
                if (rendered)
                {
                    rt.cache(level == 0, +obj, +rendered);
                    if (rml == sml)
                        rt.cache(level != 0, +obj, +rendered);
                }
                out = saveOut;
            }

#ifdef SIMULATOR
            if (level == 0)
            {
                extern int last_key;
                int key = last_key;
                output(key, obj->type(), out, len);
                record(tests_rpl,
                       "Stack key %d X-reg %+s size %u %s",
                       key, object::name(obj->type()), len, out);
            }
#endif
            w = font->width(out, len);

            if (w >= avail || memchr(out, '\n', len))
            {
                uint availRows = (y + lineHeight - 1 - top) / lineHeight;
                bool dots      = !ml || w >= avail * availRows;

                if (!dots)
                {
                    // Try to split into lines
                    size_t rlen[16];
                    uint   rows = 0;
                    utf8   end  = out + len;
                    utf8   rs   = out;
                    size   rw   = 0;
                    size   rx   = 0;
                    for (utf8 p = out; p < end; p = utf8_next(p))
                    {
                        unicode c = utf8_codepoint(p);
                        bool cr = c == '\n';
                        size cw = cr ? 0 : font->width(c);
                        rw += cw;
                        if (cr || rw >= avail)
                        {
                            if (rows >= availRows)
                            {
                                dots = true;
                                break;
                            }
                            rlen[rows++] = p - rs;
                            rs = p;
                            if (rx < rw - cw)
                                rx = rw - cw;
                            rw = cw;
                        }
                    }
                    if (rx < rw)
                        rx = rw;

                    if (!dots)
                    {
                        if (end > rs)
                            rlen[rows++] = end - rs;
                        y -= (rows - 1) * lineHeight;
                        ytop = y < top ? top : y;
                        Screen.clip(0, ytop, LCD_W, yb);
                        rs = out;
                        for (uint r = 0; r < rows; r++)
                        {
                            Screen.text(LCD_W - 2 - rx,
                                        y + r * lineHeight,
                                        rs, rlen[r], font);
                            rs += rlen[r];
                        }
                    }
                }

                if (dots)
                {
                    unicode sep   = L'…';
                    coord   x     = hdrx + 5;
                    coord   split = 200;
                    coord   skip  = font->width(sep) * 3 / 2;
                    size    offs  = lineHeight / 5;

                    Screen.clip(x, ytop, split, yb);
                    Screen.text(x, y, out, len, font, fg);
                    Screen.clip(split, ytop, split + skip, yb);
                    Screen.glyph(split + skip/8, y - offs, sep, font,
                                 pattern::gray50);
                    Screen.clip(split+skip, ytop, LCD_W, yb);
                    Screen.text(LCD_W - 2 - w, y, out, len, font, fg);
                }
            }
            else
            {
                Screen.text(LCD_W - 2 - w, y, out, len, font, fg);
            }

            if (level == 0)
                yresult = y;
        }

        // If there was any error during rendering, draw it on top
        if (utf8 errmsg = rt.error())
        {
            Screen.text(hdrx + 2, ytop, errmsg, HelpFont, bg, fg);
            rt.clear_error();
        }

        // Draw index
        Screen.clip(0, ytop, hdrx, bottom);
        snprintf(buf, sizeof(buf), "%u", level + 1);
        size hw = idxfont->width(utf8(buf));
        if (interactive == level + 1)
        {
            uint half = idxHeight / 2;
            Screen.fill(0, y + idxOffset, hdrx, y + idxOffset + idxHeight,
                        Settings.StackLevelForeground());
            Screen.clip(0, ytop, hdrx + half, bottom);
            for (uint i = 0; i < half; i++)
                Screen.fill(hdrx, y + idxOffset + half - i - 1,
                            hdrx + half - i - 1, y + idxOffset + half + i,
                            Settings.StackLevelForeground());
            Screen.text(hdrx - hw, y + idxOffset, utf8(buf), idxfont,
                        Settings.StackLevelBackground());
        }
        else
        {
            Screen.text(hdrx - hw, y + idxOffset, utf8(buf), idxfont,
                        Settings.StackLevelForeground());
        }
        Screen.clip(clip);
    }

    Screen.clip(clip);

    return yresult;
}
