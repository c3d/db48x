// ****************************************************************************
//  grob.cc                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2023 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms outlined in the LICENSE.txt file
//
//   DB48X is distributed in the hope that it will be //useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

#include "grob.h"

#include "graphics.h"
#include "parser.h"
#include "renderer.h"
#include "sysmenu.h"
#include "user_interface.h"

#include <cstdlib>
#include <cstring>



// ============================================================================
//
//   GROB: Graphic object, with stride, similar to HP48 GROB
//
// ============================================================================

rect grob::clip = { 0, 0, 1<<30, 1<<30 };

SIZE_BODY(grob)
// ----------------------------------------------------------------------------
//   Compute the size of a graphic object
// ----------------------------------------------------------------------------
{
    byte_p  p = o->payload();
    pixsize w = leb128<pixsize>(p);
    pixsize h = leb128<pixsize>(p);
    p += (w + 7) / 8 * h;
    return ptrdiff(p, o);
}


static inline byte hex(byte c)
// ----------------------------------------------------------------------------
//   A quick and dirty hex-to-number conversion
// ----------------------------------------------------------------------------
{
    return c >= '0' && c <= '9' ? c - '0'
        : c >= 'A' && c <= 'F' ? c - 'A' + 10
        : c >= 'a' && c <= 'f' ? c - 'a' + 10
        : 0xFF;
}


static void bitflip(byte *start, uint width, uint height, bool forward)
// ----------------------------------------------------------------------------
//    Invert the bits from left to right for HP48 compatibility
// ----------------------------------------------------------------------------
//    REVERSE_GROBS is set by target.h if the adjustments are necessary
{
#ifdef REVERSE_GROBS
    uint scan = (width + 7) / 8;
    uint scanline = scan * 8;
    for (uint y = 0; y < height; y++)
    {
        byte *row = start + scan * y;
        if (forward)
            for (uint r = 0; r < scan; r++)
                row[r] = ~((row[r] >> 4) | (row[r] << 4));
        for (uint x = 0; x < scanline / 2; x++)
        {
            uint r = scanline - 1 - x;
            uint lo = x / 8;
            uint ro = r / 8;
            uint lb = x % 8;
            uint rb = r % 8;
            bool ls = (row[lo] & (1 << lb));
            bool rs = (row[ro] & (1 << rb));
            row[lo] = (row[lo] & ~(1 << lb)) | (rs << lb);
            row[ro] = (row[ro] & ~(1 << rb)) | (ls << rb);
        }
        if (!forward)
            for (uint r = 0; r < scan; r++)
                row[r] = ~((row[r] >> 4) | (row[r] << 4));
    }
#endif
}


PARSE_BODY(grob)
// ----------------------------------------------------------------------------
//  Parse a graphic object
// ----------------------------------------------------------------------------
{
    utf8    src    = p.source;
    cstring s      = cstring(src);
    cstring e      = s + p.length;
    bool    grob   = strncasecmp(s, "grob ", 5) == 0;
    bool    bitmap = strncasecmp(s, "bitmap ", 7) == 0;
    bool    pixmap = strncasecmp(s, "pixmap ", 7) == 0;
    if (!grob && !bitmap && !pixmap)
        return SKIP;
    s += grob ? 5 : 7;

#ifndef CONFIG_COLOR
    if (pixmap)
    {
        rt.invalid_pixmap_error();
        return ERROR;
    }
#endif // CONFIG_COLOR

    pixsize w = strtoul(s, (char **) &s, 10);
    pixsize h = strtoul(s, (char **) &s, 10);

    while (s < e && isspace(*s))
        s++;

    grob_g g = bitmap ? bitmap::make(w, h)
#ifdef CONFIG_COLOR
             : pixmap ? pixmap::make(w, h)
#endif // CONFIG_COLOR
                      : grob::make(w, h);

    if (!g)
        return ERROR;

    id     ty    = g->type();
    size_t len   = grob::datasize(ty, w, h);
    byte   b     = 0;
    byte  *d     = (byte *) g->pixels(nullptr, nullptr, nullptr);
    byte  *d0    = d;
    bool   write = true;

    while (len && s < e)
    {
        byte c = hex(*s++);
        if (c == 0xFF)
            break;
        b = (b << 4) | c;
        write = !write;
        if (write)
        {
            *d++ = b;
            b = 0;
            len--;
        }
    }

    // Flip the bits from right to left for HP48 compatibility
    if (grob)
        bitflip(d0, w, h, true);

    p.length = s - cstring(src);
    p.out    = +g;

    return OK;
}


RENDER_BODY(grob)
// ----------------------------------------------------------------------------
//  Render the graphic object
// ----------------------------------------------------------------------------
{
    pixsize w = 0;
    pixsize h = 0;
    byte_p data = o->pixels(&w, &h);
    if (r.stack())
    {
        r.printf("Graphic %u x %u", w, h);
    }
    else
    {
        r.put(Settings.CommandDisplayMode(), utf8("grob"));
        r.printf(" %u %u ", w, h);

        // Flip the bits from right to left for HP48 compatibility
        byte * data0 = (byte *) data;
        bitflip(data0, w, h, false);

        size_t len = (w + 7) / 8 * h;
        while(len--)
            r.printf("%02X", *data++);

        // Flip the bits from right to left again for HP48 compatibility
        bitflip(data0, w, h, true);
    }
    return r.size();
}


GRAPH_BODY(grob)
// ----------------------------------------------------------------------------
//   Return a grob as itself in graphical stack rendering
// ----------------------------------------------------------------------------
{
    // If not rendering for the stack, just return object as is
    if (!g.stack || g.graph)
         return o;

     using pixsize  = blitter::size;
     pixsize width = o->width() + 4;
     pixsize height = o->height() + 4;
#if CONFIG_COLOR
     if (pixmap_g pix = o->as<pixmap>())
     {
         if (!Settings.CompatibleGROBs())
         {
             pixmap_g  result = g.pixmap(width, height);
             if (!result)
                 return nullptr;
             pixmap::surface dst    = result->pixels();
             pixmap::surface src    = pix->pixels();
             rect            inside = dst.area();
             inside.inset(2, 2);
             dst.fill(pixmap::surface::pattern::gray50);
             dst.fill(inside, g.background.bits);
             dst.copy(src, inside);
             return result;
         }
     }
#endif // CONFIG_COLOR

     grob_g  gobj   = o;
     grob_g  result = g.grob(width, height);
     if (!result)
         return nullptr;
     grob::surface dst    = result->pixels();
     grob::surface src    = gobj->pixels();
     rect          inside = dst.area();
     inside.inset(2, 2);
     dst.fill(pattern::gray50);
     dst.fill(inside, g.background);
     dst.copy(src, inside);

     return result;
}



// ============================================================================
//
//   Graphic commands
//
// ============================================================================

object::result grob::command(grob::blitop op)
// ----------------------------------------------------------------------------
//   The shared code for GXor, GOr and GAnd
// ----------------------------------------------------------------------------
{
    if (object_p coords = rt.stack(1))
    {
        PlotParametersAccess ppar;
        coord x = ppar.pair_pixel_x(coords);
        coord y = ppar.pair_pixel_y(coords);
        object_p src = rt.stack(0);
        object_p dst = rt.stack(2);

        if (!rt.error())
        {
            id   srcty = src->type();
            id   dstty = dst->type();
            bool drawn = false;
            rect drect;
            if (srcty == ID_grob || srcty == ID_bitmap)
            {
                grob_p sg = grob_p(src);
                grob::surface srcs = sg->pixels();
                point p(0,0);
                drect = srcs.area();
                drect.offset(x,y);
                if (dstty == ID_Pict)
                {
                    if (grob_p pict = user_display())
                    {
                        dst = pict;
                        dstty = pict->type();
                        rt.drop();
                    }
                    else
                    {
                        ui.draw_graphics();
                        rt.drop(3);
                        blitter::blit<blitter::CLIP_ALL>(Screen, srcs,
                                                         drect, p,
                                                         op, pattern::white);
                        drawn = true;
                    }
                }
                if (dstty == ID_grob || dstty == ID_bitmap)
                {
                    grob_p dg = grob_p(dst);
                    grob::surface dsts = dg->pixels();
                    rt.drop(2);
                    blitter::blit<blitter::CLIP_ALL>(dsts, srcs,
                                                     drect, p,
                                                     op, pattern::white);
                    drawn = true;
                }
#if CONFIG_COLOR
                else if (dstty == ID_pixmap)
                {
                    pixmap_p dg = pixmap_p(dst);
                    pixmap::surface dsts = dg->pixels();
                    rt.drop(2);
                    blitter::blit<blitter::CLIP_ALL>(dsts, srcs,
                                                     drect, p,
                                                     op, pattern::white);
                    drawn = true;
                }
#endif // CONFIG_COLOR
            }
#if CONFIG_COLOR
            else if (srcty == ID_pixmap)
            {
                pixmap_p sg = src->as<pixmap>();
                pixmap::surface srcs = sg->pixels();
                point p(0,0);
                drect = srcs.area();
                drect.offset(x,y);
                if (dstty == ID_Pict)
                {
                    if (grob_p pict = user_display())
                    {
                        dst = pict;
                        dstty = pict->type();
                        rt.drop();
                    }
                    else
                    {
                        ui.draw_graphics();
                        rt.drop(3);
                        blitter::blit<blitter::CLIP_ALL>(Screen, srcs,
                                                         drect, p,
                                                         op, pattern::white);
                        drawn = true;
                    }
                }
                if (dstty == ID_pixmap)
                {
                    pixmap_p dg = pixmap_p(dst);
                    pixmap::surface dsts = dg->pixels();
                    rt.drop(2);
                    blitter::blit<blitter::CLIP_ALL>(dsts, srcs,
                                                     drect, p,
                                                     op, pattern::white);
                    drawn = true;
                }
            }
#endif // CONFIG_COLOR
            if (drawn)
            {
                ui.draw_dirty(drect);
                refresh_dirty();
                return OK;
            }
            rt.type_error();
        }
    }
    return ERROR;
}


object::result grob::command(grob::grob1_fn gfn)
// ----------------------------------------------------------------------------
//   The shared code for GraphicAppend, GraphicStack, etc
// ----------------------------------------------------------------------------
{
    if (object_p x = rt.top())
    {
        grob_g gx = x->as_monochrome();
        if (gx)
        {
            gx = gfn(gx);
            if (gx)
                if (rt.top(+gx))
                    return OK;
        }
        else
        {
            rt.type_error();
        }
    }
    return ERROR;
}


object::result grob::command(grob::grob2_fn gfn)
// ----------------------------------------------------------------------------
//   The shared code for GraphicAppend, GraphicStack, etc
// ----------------------------------------------------------------------------
{
    if (object_p x = rt.stack(0))
    {
        if (object_p y = rt.stack(1))
        {
            grob_g gx = x->as_monochrome();
            grob_g gy = y->as_monochrome();
            if (gx && gy)
            {
                gx = gfn(gy, gx);
                if (gx)
                    if (rt.drop())
                        if (rt.top(+gx))
                            return OK;
                if (!rt.error())
                    rt.graph_does_not_fit_error();
            }
            else
            {
                rt.type_error();
            }
        }
    }
    return ERROR;
}


object::result grob::command(grob::grobop_fn gfn)
// ----------------------------------------------------------------------------
//   The shared code for GraphicSum, GraphicProduct
// ----------------------------------------------------------------------------
{
    if (object_p x = rt.top())
    {
        blitter::size h = x->as_uint32(0, true);
        if (!rt.error())
            if (grob_p gx = gfn(h))
                if (rt.top(+gx))
                    return OK;
        if (!rt.error())
            rt.graph_does_not_fit_error();
    }
    return ERROR;
}




// ============================================================================
//
//   Extracting a sub-image
//
// ============================================================================

grob_p grob::extract(object_r first, object_r last) const
// ----------------------------------------------------------------------------
//   Extract a rectangle from a grob
// ----------------------------------------------------------------------------
{
    PlotParametersAccess ppar;
    coord xf = ppar.pair_pixel_x(first);
    coord yf = ppar.pair_pixel_y(first);
    coord xl = ppar.pair_pixel_x(last);
    coord yl = ppar.pair_pixel_y(last);
    if (rt.error())
        return nullptr;
    return extract(xf, yf, xl, yl);
}


grob_p grob::extract(coord x1, coord y1, coord x2, coord y2) const
// ----------------------------------------------------------------------------
//   Extract the given region
// ----------------------------------------------------------------------------
{
    id ty = type();
    if (x1 < 0)
        x1 = 0;
    if (y1 < 0)
        y1 = 0;
    pixsize w = width();
    pixsize h = height();
    if (pixsize(x2) > w)
        x2 = w;
    if (pixsize(y2) > h)
        y2 = h;
    w = x2 - x1;
    h = y2 - y1;

    grob_g g = this;

#if CONFIG_COLOR
    if (ty == ID_pixmap)
    {
        pixmap_g result = pixmap::make(w, h);
        if (result)
        {
            pixmap_p pix = pixmap_p(+g); // May have moved away from this if GC
            pixmap::surface src = pix->pixels();
            pixmap::surface dst = result->pixels();
            dst.copy(src, -x1, -y1);
        }
        return result;
    }
#endif
    grob_p result = ty == ID_bitmap ? bitmap::make(w, h) : grob::make(w, h);
    if (result)
    {
        grob::surface src = g->pixels();
        grob::surface dst = result->pixels();
        dst.copy(src, -x1, -y1);
    }
    return result;
}



// ============================================================================
//
//   Bitmap: Packed bitmap - Like a GROB, but memory optimized for DB48x
//
// ============================================================================

SIZE_BODY(bitmap)
// ----------------------------------------------------------------------------
//   Compute the size of a packed bitmap
// ----------------------------------------------------------------------------
{
    byte_p  p = o->payload();
    pixsize w = leb128<pixsize>(p);
    pixsize h = leb128<pixsize>(p);
    p += (w * h + 7) / 8;
    return ptrdiff(p, o);
}


RENDER_BODY(bitmap)
// ----------------------------------------------------------------------------
//  Render the graphic object
// ----------------------------------------------------------------------------
{
    pixsize w = 0;
    pixsize h = 0;
    byte_p data = o->pixels(&w, &h);
    if (r.stack())
    {
        r.printf("Bitmap %u x %u", w, h);
    }
    else
    {
        r.put(Settings.CommandDisplayMode(), utf8("bitmap"));
        r.printf(" %u %u ", w, h);

        size_t len = (w * h + 7) / 8;
        while(len--)
            r.printf("%02X", *data++);
    }
    return r.size();
}



#ifdef CONFIG_COLOR
// ============================================================================
//
//   Pixmap: Pixel map: 16BPP color pixel map
//
// ============================================================================

SIZE_BODY(pixmap)
// ----------------------------------------------------------------------------
//   Compute the size of a packed pixmap
// ----------------------------------------------------------------------------
{
    byte_p  p = o->payload();
    pixsize w = leb128<pixsize>(p);
    pixsize h = leb128<pixsize>(p);
    p += (w * h * 16 + 7) / 8;
    return ptrdiff(p, o);
}


RENDER_BODY(pixmap)
// ----------------------------------------------------------------------------
//  Render the graphic object
// ----------------------------------------------------------------------------
{
    pixsize w = 0;
    pixsize h = 0;
    byte_p data = o->pixels(&w, &h);
    if (r.stack())
    {
        r.printf("Pixmap %u x %u", w, h);
    }
    else
    {
        r.put(Settings.CommandDisplayMode(), utf8("pixmap"));
        r.printf(" %u %u ", w, h);

        size_t len = (w * h * 16 + 7) / 8;
        while(len--)
            r.printf("%02X", *data++);
    }
    return r.size();
}


// ============================================================================
//
//   Black and white patterns
//
// ============================================================================

// Pre-built grob::patterns for shades of grey
const grob::pattern grob::pattern::black   = grob::pattern(0, 0, 0);
const grob::pattern grob::pattern::gray10  = grob::pattern(32, 32, 32);
const grob::pattern grob::pattern::gray25  = grob::pattern(64, 64, 64);
const grob::pattern grob::pattern::gray50  = grob::pattern(128, 128, 128);
const grob::pattern grob::pattern::gray75  = grob::pattern(192, 192, 192);
const grob::pattern grob::pattern::gray90  = grob::pattern(224, 224, 224);
const grob::pattern grob::pattern::white   = grob::pattern(255, 255, 255);
const grob::pattern grob::pattern::invert  = grob::pattern();
#endif
