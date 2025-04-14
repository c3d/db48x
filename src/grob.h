#ifndef GROB_H
#  define GROB_H
// ****************************************************************************
//  grob.h                                                        DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Graphic objects, representing a bitmap in memory
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
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

#include "blitter.h"
#include "list.h"
#include "object.h"
#include "runtime.h"
#include "target.h"
#include "renderer.h"
#include "settings.h"


GCP(grob);
GCP(bitmap);
GCP(pixmap);

struct grob : object
// ----------------------------------------------------------------------------
//   Representation of a graphic object
// ----------------------------------------------------------------------------
{
    using pixsize = blitter::size;
    using surface = blitter::surface<blitter::mode::MONOCHROME_REVERSE>;
    using pattern = blitter::pattern<blitter::mode::MONOCHROME_REVERSE>;


    grob(id type, pixsize w, pixsize h, gcbytes bits): object(type)
    // ------------------------------------------------------------------------
    //   Graphic object constructor
    // ------------------------------------------------------------------------
    {
        byte *p = (byte *) payload();
        p = leb128(p, w);
        p = leb128(p, h);
        size_t ds = datasize(type, w, h);
        byte_p s = bits;
        while (ds--)
            *p++ = *s++;
    }



    grob(id type, pixsize w, pixsize h): object(type)
    // ------------------------------------------------------------------------
    //   Graphic object constructor
    // ------------------------------------------------------------------------
    {
        byte *p = (byte *) payload();
        p = leb128(p, w);
        p = leb128(p, h);
        size_t ds = datasize(type, w, h);
        while (ds--)
            *p++ = 0;
    }


    static size_t required_memory(id type, pixsize w, pixsize h)
    // ------------------------------------------------------------------------
    //   Compute required grob memory for the given parameters
    // ------------------------------------------------------------------------
    {
        size_t bodysize = bytesize(type, w, h);
        return leb128size(type) + bodysize;
    }


    static size_t required_memory(id            type,
                                  pixsize       w,
                                  pixsize       h,
                                  gcutf8 UNUSED bytes)
    // ------------------------------------------------------------------------
    //   Compute required grob memory for the given parameters
    // ------------------------------------------------------------------------
    {
        size_t bs = bytesize(type, w, h);
        return leb128size(type) + bs;
    }


    static grob_p make(pixsize w, pixsize h)
    // ------------------------------------------------------------------------
    //   Build a grob from the given parameters
    // ------------------------------------------------------------------------
    {
        return rt.make<grob>(w, h);
    }


    static grob_p make(pixsize w, pixsize h, byte_p bits)
    // ------------------------------------------------------------------------
    //   Build a grob from the given parameters
    // ------------------------------------------------------------------------
    {
        return rt.make<grob>(w, h, bits);
    }


    static size_t bytesize(id type, pixsize w, pixsize h)
    // ------------------------------------------------------------------------
    //   Compute the number of bytes required for a bitmap
    // ------------------------------------------------------------------------
    {
        size_t ds = datasize(type, w, h);
        return leb128size(w) + leb128size(h) + ds;
    }


    static size_t datasize(id type, pixsize w, pixsize h)
    // ------------------------------------------------------------------------
    //   Compute the number of bytes required for a bitmap
    // ------------------------------------------------------------------------
    {
        return type == ID_grob ? (w + 7) / 8 * h : (w * h + 7) / 8;
    }


    pixsize width() const
    // ------------------------------------------------------------------------
    //   Return the width of a grob
    // ------------------------------------------------------------------------
    {
        byte_p p = payload();
        return leb128<pixsize>(p);
    }


    pixsize height() const
    // ------------------------------------------------------------------------
    //   Return the height of a grob
    // ------------------------------------------------------------------------
    {
        byte_p p = payload();
        p = leb128skip(p);      // Skip width
        return leb128<pixsize>(p);
    }


    byte_p pixels(pixsize *width, pixsize *height, size_t *datalen = 0) const
    // ------------------------------------------------------------------------
    //   Return the byte pointer to the data in the grob
    // ------------------------------------------------------------------------
    {
        byte_p  p   = payload();
        pixsize w   = leb128<pixsize>(p);
        pixsize h   = leb128<pixsize>(p);
        if (width)
            *width = w;
        if (height)
            *height = h;
        if (datalen)
            *datalen = datasize(type(), w, h);
        return p;
    }


    surface pixels() const
    // ------------------------------------------------------------------------
    //   Return a blitter surface for the grob
    // ------------------------------------------------------------------------
    {
        pixsize w        = 0;
        pixsize h        = 0;
        byte_p  bitmap   = pixels(&w, &h);
        pixsize scanline = type() == ID_grob ? (w + 7) / 8 * 8 : w;
        return surface((pixword *) bitmap, w, h, scanline);
    }


    using blitop = blitter::blitop;

    static object::result command(blitop op);
    // ------------------------------------------------------------------------
    //  Shared code for GXor, GOr, GAnd
    // ------------------------------------------------------------------------

    typedef grob_p(*grob1_fn)(grob_r x);
    typedef grob_p(*grob2_fn)(grob_r y, grob_r x);
    typedef grob_p(*grobop_fn)(pixsize height);
    static object::result command(grob1_fn gfn);
    static object::result command(grob2_fn gfn);
    static object::result command(grobop_fn gfn);
    // ------------------------------------------------------------------------
    //   Shared code for GraphicAppend, GraphicStack, etc
    // ------------------------------------------------------------------------


    // Extract a subimage
    grob_p extract(object_r first, object_r last) const;
    grob_p extract(coord x1, coord y1, coord x2, coord y2) const;


public:
    OBJECT_DECL(grob);
    PARSE_DECL(grob);
    SIZE_DECL(grob);
    RENDER_DECL(grob);
    GRAPH_DECL(grob);
};


struct bitmap : grob
// ----------------------------------------------------------------------------
//   DB48X optimized bitmap representation
// ----------------------------------------------------------------------------
{
    bitmap(id ty, pixsize w, pixsize h, gcbytes bits) : grob(ty, w, h, bits) {}
    bitmap(id type, pixsize w, pixsize h): grob(type, w, h) {}

    static grob_p make(pixsize w, pixsize h)
    // ------------------------------------------------------------------------
    //   Build a grob from the given parameters
    // ------------------------------------------------------------------------
    {
        return rt.make<bitmap>(w, h);
    }


    static grob_p make(pixsize w, pixsize h, byte_p bits)
    // ------------------------------------------------------------------------
    //   Build a grob from the given parameters
    // ------------------------------------------------------------------------
    {
        return rt.make<bitmap>(w, h, bits);
    }

public:
    OBJECT_DECL(bitmap);
    // PARSE_DECL(bitmap) is managed by grob class
    SIZE_DECL(bitmap);
    RENDER_DECL(bitmap);
};


struct pixmap : object
// ----------------------------------------------------------------------------
//   Representation of a pixel map, with any number of bits per pixel
// ----------------------------------------------------------------------------
{
    using pixsize = blitter::size;

    pixmap(id type, pixsize w, pixsize h, uint bpp, gcbytes bits): object(type)
    // ------------------------------------------------------------------------
    //   Pixmap constructor
    // ------------------------------------------------------------------------
    {
        byte *p = (byte *) payload();
        p = leb128(p, w);
        p = leb128(p, h);
        p = leb128(p, bpp);
        size_t ds = datasize(type, w, h, bpp);
        byte_p s = bits;
        while (ds--)
            *p++ = *s++;
    }



    pixmap(id type, pixsize w, pixsize h, uint bpp): object(type)
    // ------------------------------------------------------------------------
    //   Pixmap constructor
    // ------------------------------------------------------------------------
    {
        byte *p = (byte *) payload();
        p = leb128(p, w);
        p = leb128(p, h);
        p = leb128(p, bpp);
        size_t ds = datasize(type, w, h, bpp);
        while (ds--)
            *p++ = 0;
    }


    static size_t required_memory(id type, pixsize w, pixsize h, uint bpp)
    // ------------------------------------------------------------------------
    //   Compute required pixmap memory for the given parameters
    // ------------------------------------------------------------------------
    {
        size_t bodysize = bytesize(type, w, h, bpp);
        return leb128size(type) + bodysize;
    }


    static size_t required_memory(id            type,
                                  pixsize       w,
                                  pixsize       h,
                                  uint          bpp,
                                  gcutf8 UNUSED bytes)
    // ------------------------------------------------------------------------
    //   Compute required pixmap memory for the given parameters
    // ------------------------------------------------------------------------
    {
        size_t bs = bytesize(type, w, h, bpp);
        return leb128size(type) + bs;
    }


    static pixmap_p make(pixsize w, pixsize h, uint bpp)
    // ------------------------------------------------------------------------
    //   Build a pixmap from the given parameters
    // ------------------------------------------------------------------------
    {
        return rt.make<pixmap>(w, h, bpp);
    }


    static pixmap_p make(pixsize w, pixsize h, uint bpp, byte_p bits)
    // ------------------------------------------------------------------------
    //   Build a pixmap from the given parameters
    // ------------------------------------------------------------------------
    {
        return rt.make<pixmap>(w, h, bpp, bits);
    }


    static size_t bytesize(id type, pixsize w, pixsize h, uint bpp)
    // ------------------------------------------------------------------------
    //   Compute the number of bytes required for a bitmap
    // ------------------------------------------------------------------------
    {
        size_t ds = datasize(type, w, h, bpp);
        return leb128size(w) + leb128size(h) + leb128size(bpp) + ds;
    }


    static size_t datasize(id type, pixsize w, pixsize h, uint bpp)
    // ------------------------------------------------------------------------
    //   Compute the number of bytes required for a bitmap
    // ------------------------------------------------------------------------
    {
        return (w * h * bpp + 7) / 8;
    }


    pixsize width() const
    // ------------------------------------------------------------------------
    //   Return the width of a pixmap
    // ------------------------------------------------------------------------
    {
        byte_p p = payload();
        return leb128<pixsize>(p);
    }


    pixsize height() const
    // ------------------------------------------------------------------------
    //   Return the height of a pixmap
    // ------------------------------------------------------------------------
    {
        byte_p p = payload();
        p = leb128skip(p);      // Skip width
        return leb128<pixsize>(p);
    }


    uint depth() const
    // ------------------------------------------------------------------------
    //   Return the bit depth of a pixmap (number of bits per pixel)
    // ------------------------------------------------------------------------
    {
        byte_p p = payload();
        p = leb128skip(p);      // Skip width
        p = leb128skip(p);      // Skip height
        return leb128<uint>(p);
    }


    byte_p pixels(pixsize *width, pixsize *height, uint *bpp,
                  size_t *datalen = nullptr) const
    // ------------------------------------------------------------------------
    //   Return the byte pointer to the data in the pixmap
    // ------------------------------------------------------------------------
    {
        byte_p  p   = payload();
        pixsize w   = leb128<pixsize>(p);
        pixsize h   = leb128<pixsize>(p);
        uint    b   = leb128<uint>(p);
        if (width)
            *width = w;
        if (height)
            *height = h;
        if (bpp)
            *bpp = b;
        if (datalen)
            *datalen = datasize(type(), w, h, b);
        return p;
    }


    surface pixels() const
    // ------------------------------------------------------------------------
    //   Return a blitter surface for the pixmap
    // ------------------------------------------------------------------------
    {
        pixsize w        = 0;
        pixsize h        = 0;
        uint    bpp      = 0;
        byte_p  bitmap   = pixels(&w, &h, &bpp);
        pixsize scanline = w;
        return bpp == surface::BPP
            ? surface((pixword *) bitmap, w, h, scanline)
            : surface(nullptr, 0, 0, 0);
    }


    // Extract a subimage
    pixmap_p extract(object_r first, object_r last) const;
    pixmap_p extract(coord x1, coord y1, coord x2, coord y2) const;


public:
    OBJECT_DECL(pixmap);
    PARSE_DECL(pixmap);
    SIZE_DECL(pixmap);
    RENDER_DECL(pixmap);
    GRAPH_DECL(pixmap);
};


struct grapher
// ----------------------------------------------------------------------------
//   Information about graphing environment
// ----------------------------------------------------------------------------
{
    using font_id = settings::font_id;

    grapher(size          w     = LCD_W,
            size          h     = LCD_H,
            font_id       f     = Settings.ResultFont(),
            grob::pattern fg    = Settings.Foreground(),
            grob::pattern bg    = Settings.Background(),
            bool          stack = false,
            bool          expr  = false,
            bool          graph = false)
        : maxw(w),
          maxh(h),
          start(sys_current_ms()),
          duration(Settings.GraphingTimeLimit()),
          voffset(0),
          font(f),
          foreground(fg),
          background(bg),
          stack(stack),
          expression(expr),
          graph(graph)
    {}

    grapher(const grapher &other) = default;

    grob_p grob(size w, size h)
    {
        if (w <= maxw && h <= maxh && sys_current_ms() - start <= duration)
            return grob::make(w, h);
        return nullptr;
    }

    bool reduce_font()
    {
        if (sys_current_ms() - start > duration)
            return false;
        font_id next = settings::smaller_font(font);
        if (next == font)
            return false;
        font = next;
        return true;
    }

    size          maxw;
    size          maxh;
    uint          start;
    uint          duration;
    coord         voffset;
    font_id       font;
    grob::pattern foreground;
    grob::pattern background;
    bool          stack;
    bool          expression;
    bool          graph;
};

#endif // GROB_H
