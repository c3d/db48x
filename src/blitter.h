#ifndef BLITTER_H
#define BLITTER_H
// ****************************************************************************
//  blitter.h                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Low-level graphic routines (blitter) for the DB48X project
//
//     These routines are designed to be highly-optimizable while being able
//     to deal with 1, 4 and 16 bits per pixel as found on various calculators
//     To achieve that objective, the code is parameterized at compile-time,
//     so it makes relatively heavy use of templates and inlining.
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
//
//   In the code, BPP stands for "Bits per pixels", and BPW for "Bits per word"
//   Pixel buffer words are assumed to be 32-bit as on most calculators today.
//

#include "font.h"
#include "types.h"
#include "utf8.h"

#include "recorder.h"

RECORDER_DECLARE(debug);

#define PACKED __attribute__((packed))


struct blitter
// ----------------------------------------------------------------------------
//    The graphics class managing bitmap operations
// ----------------------------------------------------------------------------
{
    // ========================================================================
    //
    //    Types and constants
    //
    // ========================================================================

    // Basic types
    typedef int32_t  coord;
    typedef uint32_t size;
    typedef size_t   offset;
    typedef uint32_t pixword;
    typedef uint16_t palette_index;
    typedef uint64_t pattern_bits;

    enum
    {
        BPW = 8 * sizeof(pixword) // Bits per pixword
    };


    enum mode
    // ------------------------------------------------------------------------
    //   Graphics mode (including bits per pixel info)
    // ------------------------------------------------------------------------
    {
        MONOCHROME,         // Monochrome bitmap, e.g. fonts
        MONOCHROME_REVERSE, // Monochrome bitmap, reverse X axis (DM42)
        GRAY_4BPP,          // Gray, 4 bits per pixel (HP50G and related)
        RGB_16BPP,          // RGB16 (HP Prime)
    };


    // ========================================================================
    //
    //    Color representation
    //
    // ========================================================================
    //  Colors have a generic RGB-based interface, even on monochrome systems
    //  like the DM42, or on grayscale systems like the HP50G.

    template <mode Mode>
    union color
    // ------------------------------------------------------------------------
    //   The generic color representation
    // ------------------------------------------------------------------------
    {
        // The constructor takes red/green/blue values
        color(uint8_t red, uint8_t green, uint8_t blue);

        // The constructor from a bit pattern
        color(pixword bits);

        // Return the components
        uint8_t red();
        uint8_t green();
        uint8_t blue();

        enum
        {
            BPP = 1
        };
    };


    // ========================================================================
    //
    //   Pattern representation
    //
    // ========================================================================
    //   A pattern is a NxN set of pixels on screen, corresponding to a fixed
    //   number of bits. This is used to simulate gray scales on monochrome
    //   machines like the DM42, but can also create visual effects on grayscale
    //   or color systems.
    //   Patterns are presently always stored as a 64-bit value for efficient
    //   processing during drawing. For 1BPP, patterns represent 8x8 pixel,
    //   for 4BPP they represent 4x4 pixels, and for 16BPP 2x2 pixels.
    //   This means it is always possible to create two-color and four-color
    //   patterns in a way that is independent of the target system.
    //   A pattern can be built from RGB values, which will build a pattern
    //   with the corresponding pixel density on a monochrome system.

    template <mode Mode>
    union pattern
    // ------------------------------------------------------------------------
    //   Pattern representation for fills
    // ------------------------------------------------------------------------
    {
        uint64_t bits;

        enum
        {
            SIZE = 1
        }; // Size of the pattern
        enum : uint64_t
        {
            SOLID = 1
        }; // Multiplier for solids
        enum
        {
            BPP = color<Mode>::BPP
        }; // Bit per pixels for pattern
        using color = blitter::color<Mode>;

    public:
        // Build a solid pattern from a single color
        pattern(color c);

        // Build a checkered pattern for a given RGB level
        pattern(uint8_t red, uint8_t green, uint8_t blue);

        // Build a checkerboard from 2 or 4 colors in an array
        template <uint N>
        pattern(color colors[N]);

        // Build a pattern with two or four alternating colors
        pattern(color a, color b);
        pattern(color a, color b, color c, color d);

        // Pattern from bits
        pattern(uint64_t bits = ~0ULL): bits(bits) {}

        // Some pre-defined shades of gray
        static const pattern black  = pattern(0,     0,   0);
        static const pattern gray10 = pattern(32,   32,  32);
        static const pattern gray25 = pattern(64,   64,  64);
        static const pattern gray50 = pattern(128, 128, 128);
        static const pattern gray75 = pattern(192, 192, 192);
        static const pattern gray90 = pattern(224, 224, 224);
        static const pattern white  = pattern(255, 255, 255);
        static const pattern invert  = pattern();
    };


    // ========================================================================
    //
    //    Points and rectangles
    //
    // ========================================================================

    struct point
    // ------------------------------------------------------------------------
    //    A point holds a pair of coordinates
    // ------------------------------------------------------------------------
    {
        point(coord x = 0, coord y = 0) : x(x), y(y)
        {
        }
        point(const point &o) = default;
        coord x, y;
    };


    struct rect
    // ------------------------------------------------------------------------
    //   A rectangle is a point with a width and a height
    // ------------------------------------------------------------------------
    {
        rect(coord x1 = 0, coord y1 = 0, coord x2 = -1, coord y2 = -1)
            : x1(x1),
              y1(y1),
              x2(x2),
              y2(y2)
        {
        }
        rect(size w, size h) : rect(0, 0, w - 1, h - 1)
        {
        }
        rect(const rect &o)            = default;
        rect &operator=(const rect &o) = default;

        rect &operator&=(const rect &o)
        // --------------------------------------------------------------------
        //   Intersection of two rectangles
        // --------------------------------------------------------------------
        {
            if (x1 < o.x1)
                x1 = o.x1;
            if (x2 > o.x2)
                x2 = o.x2;
            if (y1 < o.y1)
                y1 = o.y1;
            if (y2 > o.y2)
                y2 = o.y2;
            return *this;
        }

        rect &operator|=(const rect &o)
        // --------------------------------------------------------------------
        //   Union of two rectangles
        // --------------------------------------------------------------------
        {
            if (x1 > o.x1)
                x1 = o.x1;
            if (x2 < o.x2)
                x2 = o.x2;
            if (y1 > o.y1)
                y1 = o.y1;
            if (y2 < o.y2)
                y2 = o.y2;
            return *this;
        }

        friend rect operator&(const rect &a, const rect &b)
        // --------------------------------------------------------------------
        //   Intersection of two rectangle
        // --------------------------------------------------------------------
        {
            rect r(a);
            r &= b;
            return r;
        }

        friend rect operator|(const rect &a, const rect &b)
        // --------------------------------------------------------------------
        //   Union of two rectangle
        // --------------------------------------------------------------------
        {
            rect r(a);
            r |= b;
            return r;
        }

        void inset(size dw, size dh)
        // --------------------------------------------------------------------
        //   Inset the rectangle by the given amount
        // --------------------------------------------------------------------
        {
            x1 += dw;
            y1 += dh;
            x2 -= dw;
            y2 -= dh;
        }

        void inset(size d)
        // --------------------------------------------------------------------
        //   Inset the rectangle by the given amount
        // --------------------------------------------------------------------
        {
            inset(d, d);
        }

        void offset(coord dx, coord dy)
        // --------------------------------------------------------------------
        //   Offset a rectangle by the given amounts
        // --------------------------------------------------------------------
        {
            x1 += dx;
            x2 += dx;
            y1 += dy;
            y2 += dy;
        }


        bool empty() const
        // --------------------------------------------------------------------
        //   Check if a rectangle is empty
        // --------------------------------------------------------------------
        {
            return x1 > x2 || y1 > y2;
        }

        size width() const
        // --------------------------------------------------------------------
        //   Return the width of a rectangle
        // --------------------------------------------------------------------
        {
            return x2 - x1 + 1;
        }

        size height() const
        // --------------------------------------------------------------------
        //   Return the height of a rectangle
        // --------------------------------------------------------------------
        {
            return y2 - y1 + 1;
        }

        bool contains(point p) const
        // --------------------------------------------------------------------
        //   Return true if point is inside the rectangle
        // --------------------------------------------------------------------
        {
            return p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2;
        }


      public:
        coord x1, y1, x2, y2;
    };


    // =========================================================================
    //
    //    Core blitting routine
    //
    // =========================================================================

    enum clipping
    // ------------------------------------------------------------------------
    //   Hints to help the compiler drop useless code
    // ------------------------------------------------------------------------
    {
        CLIP_NONE   = 0,
        CLIP_SRC    = 1,
        CLIP_DST    = 2,
        CLIP_ALL    = 3,
        SKIP_SOURCE = 4,
        SKIP_COLOR  = 8,
        OVERLAP     = 16, // Set if src and dest overlap
        FILL_QUICK  = SKIP_SOURCE,
        FILL_SAFE   = SKIP_SOURCE | CLIP_DST,
        COPY        = CLIP_ALL | SKIP_COLOR,
        DRAW        = CLIP_ALL,
    };

    // blitop: a blitting operation
    typedef pixword (*blitop)(pixword dst, pixword src, pixword arg);

    template <clipping Clip, typename Dst, typename Src, mode CMode>
    static void blit(Dst           &dst,
                     const Src     &src,
                     const rect    &drect,
                     const point   &spos,
                     blitop         op,
                     pattern<CMode> colors);


    // ========================================================================
    //
    //   Surface: a bitmap for graphic operations
    //
    // ========================================================================

    template <mode Mode>
    struct surface
    // -------------------------------------------------------------------------
    //   Structure representing a surface on screen
    // -------------------------------------------------------------------------
    {
        surface(pixword *p, size w, size h, size scanline, size swap)
            : pixels(p),
              w(w),
              h(h),
              scanline(scanline),
              swapsize(swap-1),
              drawable(w, h)
        {
        }
        surface(pixword *p, size w, size h, size scanline)
            : surface(p, w, h, scanline, scanline)
        {
        }
        surface(pixword *p, size w, size h) : surface(p, w, h, w)
        {
        }
        surface(const surface &o) = default;

        void horizontal_adjust(coord UNUSED &x1, coord UNUSED &x2) const
        {
        }
        void vertical_adjust(coord UNUSED &x1, coord UNUSED &x2) const
        {
        }
        static bool horizontal_swap()   { return false; }
        static bool vertical_swap()     { return false; }
        static const mode blitmode = Mode;

        // Operations used by the blitting routine
        using color   = blitter::color<Mode>;
        using pattern = blitter::pattern<Mode>;

        enum
        {
            BPP = color::BPP
        };

    public:
        surface &clip(const rect &r)
        // --------------------------------------------------------------------
        //    Limit drawing to the given rectangle
        // --------------------------------------------------------------------
        {
            drawable = r;
            drawable &= rect(w, h);
            return *this;
        }


        surface &clip(coord x1, coord y1, coord x2, coord y2)
        // --------------------------------------------------------------------
        //   Clip an area given in coordinates
        // --------------------------------------------------------------------
        {
            return clip(rect(x1, y1, x2, y2));
        }

        const rect &clip() const
        // --------------------------------------------------------------------
        //   Return the clipping area
        // --------------------------------------------------------------------
        {
            return drawable;
        }

        rect area() const
        // --------------------------------------------------------------------
        //   Return total drawing area
        // --------------------------------------------------------------------
        {
            return rect(w, h);
        }

        size width() const
        // --------------------------------------------------------------------
        //   Total drawing width
        // --------------------------------------------------------------------
        {
            return w;
        }

        size height() const
        // --------------------------------------------------------------------
        //   Total drawing height
        // --------------------------------------------------------------------
        {
            return h;
        }


        color pixel_color(coord x, coord y) const
        // --------------------------------------------------------------------
        //   Return the color for the given pixel
        // --------------------------------------------------------------------
        {
            if (drawable.contains(point(x,y)))
            {
                horizontal_adjust(x,x);
                vertical_adjust(y,y);
                offset   po = pixel_offset(x, y);
                pixword *pa = pixel_address(po);
                offset   ps = pixel_shift(po);
                pixword  pw = *pa;
                pixword  pc = (pw >> ps) & ~(~0U << BPP);
                return color(pc);
            }
            return color(0);
        }


        template <clipping Clip = FILL_SAFE>
        void fill(const rect &r, pattern colors = pattern::black)
        // --------------------------------------------------------------------
        //   Fill a rectangle
        // --------------------------------------------------------------------
        {
            blit<Clip>(*this, *this, r, point(), blitop_set, colors);
        }

        template <clipping Clip = FILL_SAFE>
        void fill(coord   x1,
                  coord   y1,
                  coord   x2,
                  coord   y2,
                  pattern colors = pattern::black)
        // --------------------------------------------------------------------
        //   Fill a rectangle with a color pattern
        // --------------------------------------------------------------------
        {
            fill<Clip>(rect(x1, y1, x2, y2), colors);
        }

        template <clipping Clip = FILL_SAFE>
        void fill(pattern colors = pattern::black)
        // --------------------------------------------------------------------
        //   Fill the entire area with the chosen color
        // --------------------------------------------------------------------
        {
            fill<Clip>(drawable, colors);
        }

        template <clipping Clip = FILL_SAFE>
        void invert(const rect &r, pattern colors = pattern::invert)
        // --------------------------------------------------------------------
        //   Invert a rectangle
        // --------------------------------------------------------------------
        {
            blit<Clip>(*this, *this, r, point(), blitop_xor, colors);
        }

        template <clipping Clip = FILL_SAFE>
        void invert(coord   x1,
                    coord   y1,
                    coord   x2,
                    coord   y2,
                    pattern colors = pattern::invert)
        // --------------------------------------------------------------------
        //   Invert a rectangle with a color pattern
        // --------------------------------------------------------------------
        {
            invert<Clip>(rect(x1, y1, x2, y2), colors);
        }

        template <clipping Clip = FILL_SAFE>
        void invert(pattern colors = pattern::invert)
        // --------------------------------------------------------------------
        //   Invert the entire area with the chosen color
        // --------------------------------------------------------------------
        {
            invert<Clip>(drawable, colors);
        }

        template <clipping Clip = COPY, mode SMode>
        void copy(surface<SMode> &src,
                  const rect     &r,
                  const point    &spos  = point(0, 0))
        // --------------------------------------------------------------------
        //   Copy a rectangular area from the source
        // --------------------------------------------------------------------
        {
            blit<Clip>(*this, src, r, spos, blitop_source, pattern());
        }

        template <clipping Clip = COPY, mode SMode>
        void copy(surface<SMode> &src,
                  const point    &pos   = point(0, 0))
        // --------------------------------------------------------------------
        //   Copy a rectangular area from the source
        // --------------------------------------------------------------------
        {
            return copy(src, pos.x, pos.y, pattern());
        }

        template <clipping Clip = COPY, mode SMode>
        void copy(surface<SMode> &src,
                  coord           x,
                  coord           y)
        // --------------------------------------------------------------------
        //   Copy a rectangular area from the source
        // --------------------------------------------------------------------
        {
            rect dest(x, y, x + src.w - 1, y + src.h - 1);
            blit<Clip>(*this, src, dest, point(), blitop_source, pattern());
        }


        template <clipping Clip = DRAW, mode SMode>
        void draw(surface<SMode> &src,
                  const point    &pos   = point(0, 0),
                  pattern         color = pattern::black,
                  blitop          op    = blitop_draw)
        // --------------------------------------------------------------------
        //   Draw a rectangular area from the source
        // --------------------------------------------------------------------
        {
            return draw(src, pos.x, pos.y, color, op);
        }


        template <clipping Clip = DRAW, mode SMode>
        void draw(surface<SMode> &src,
                  coord           x,
                  coord           y,
                  pattern         color = pattern::black,
                  blitop          op    = blitop_draw)
        // --------------------------------------------------------------------
        //   Draw a rectangular area from the source
        // --------------------------------------------------------------------
        {
            rect dest(x, y, x + src.w - 1, y + src.h - 1);
            blit<Clip>(*this, src, dest, point(), op, color);
        }


        template <clipping Clip = DRAW, mode SMode>
        void draw_background(surface<SMode> &src,
                             const point    &pos   = point(0, 0),
                             pattern         color = pattern::black,
                             blitop          op    = blitop_background)
        // --------------------------------------------------------------------
        //   Draw a rectangular area from the source backgroundo
        // --------------------------------------------------------------------
        {
            draw(src, pos.x, pos.y, color, op);
        }


        template <clipping Clip = DRAW, mode SMode>
        void draw_background(surface<SMode> &src,
                             coord           x,
                             coord           y,
                             pattern         color = pattern::white,
                             blitop          op    = blitop_background)
        // --------------------------------------------------------------------
        //   Draw a rectangular area from the source background
        // --------------------------------------------------------------------
        {
            draw(src, x, y, color, op);
        }


        template <clipping Clip = CLIP_DST>
        coord glyph(coord       x,
                    coord       y,
                    unicode     codepoint,
                    const font *f,
                    pattern     colors = pattern::black,
                    blitop      op     = blitop_draw);
        // --------------------------------------------------------------------
        //   Draw a glyph with the given operation and colors
        // --------------------------------------------------------------------

        template <clipping Clip = CLIP_DST>
        coord glyph(coord       x,
                    coord       y,
                    unicode     codepoint,
                    const font *f,
                    pattern     fg,
                    pattern     bg);
        // --------------------------------------------------------------------
        //   Draw a glyph with a foreground and background
        // --------------------------------------------------------------------

        template <clipping Clip = CLIP_DST>
        coord text(coord       x,
                   coord       y,
                   utf8        text,
                   const font *f,
                   pattern     colors = pattern::black,
                   blitop      op     = blitop_draw);
        // --------------------------------------------------------------------
        //   Draw a text with the given operation and colors
        // --------------------------------------------------------------------

        template <clipping Clip = CLIP_DST>
        coord text(coord       x,
                   coord       y,
                   utf8        text,
                   const font *f,
                   pattern     fg,
                   pattern     bg);
        // --------------------------------------------------------------------
        //   Draw a text with a foreground and background
        // --------------------------------------------------------------------

        template <clipping Clip = CLIP_DST>
        coord text(coord       x,
                   coord       y,
                   utf8        text,
                   size_t      len,
                   const font *f,
                   pattern     colors = pattern::black,
                   blitop      op     = blitop_draw);
        // --------------------------------------------------------------------
        //   Draw a text with the given operation and colors
        // --------------------------------------------------------------------

        template <clipping Clip = CLIP_DST>
        coord text(coord       x,
                   coord       y,
                   utf8        text,
                   size_t      len,
                   const font *f,
                   pattern     fg,
                   pattern     bg);
        // --------------------------------------------------------------------
        //   Draw a text with a foreground and background
        // --------------------------------------------------------------------

        template <clipping Clip = FILL_SAFE>
        void line(coord   x1,
                  coord   y1,
                  coord   x2,
                  coord   y2,
                  size    width,
                  pattern fg);
        // --------------------------------------------------------------------
        //   Draw a line between the given coordinates
        // --------------------------------------------------------------------

        template <clipping Clip = FILL_SAFE>
        void ellipse(coord   x1,
                     coord   y1,
                     coord   x2,
                     coord   y2,
                     size    width,
                     pattern fg);
        // --------------------------------------------------------------------
        //   Draw an ellipse with the given coordinates
        // --------------------------------------------------------------------

        template <clipping Clip = FILL_SAFE>
        void circle(coord x, coord y, size r, size width, pattern fg)
        // --------------------------------------------------------------------
        //   Draw a circle with the given coordinates
        // --------------------------------------------------------------------
        {
            ellipse(x - r / 2,
                    y - r / 2,
                    x + (r + 1) / 2,
                    y + (r + 1) / 2,
                    width,
                    fg);
        }

        template <clipping Clip = FILL_SAFE>
        void rectangle(coord x1, coord y1,
                       coord x2, coord y2,
                       size width, pattern fg)
        // --------------------------------------------------------------------
        //   Draw a rectangle
        // --------------------------------------------------------------------
        {
            rounded_rectangle(x1, y1, x2, y2, 0, width, fg);
        }

        template <clipping Clip = FILL_SAFE>
        void rounded_rectangle(coord x1, coord y1,
                               coord x2, coord y2,
                               size r, size width, pattern fg);
        // --------------------------------------------------------------------
        //   Draw a rounded rectangle between the given coordinates
        // --------------------------------------------------------------------


    protected:
        offset pixel_offset(coord x, coord y) const
        // ---------------------------------------------------------------------
        //   Offset in bits in a given surface for the given coordinates
        // ---------------------------------------------------------------------
        {
            return ((offset) scanline * y + x) * (offset) BPP;
        }

        offset pixel_shift(offset bitoffset) const
        // ---------------------------------------------------------------------
        //   Shift in bits in the word for the given coordinates
        // ---------------------------------------------------------------------
        {
            return bitoffset % BPW;
        }

        pixword *pixel_address(offset bitoffset) const
        // ---------------------------------------------------------------------
        //   Get the address of a word representing the pixel in a surface
        // ---------------------------------------------------------------------
        {
            return pixels + bitoffset / BPW;
        }

    protected:
        friend struct blitter;
        pixword *pixels;        // Word-aligned address of surface buffer
        size     w;             // Pixel width of buffer
        size     h;             // Pixel height of buffer
        size     scanline;      // Scanline for the buffer (can be > width)
        size     swapsize;      // Size used for swapping left/right coords
        rect     drawable;      // Draw area (clipping outside)
    };


  protected:
    // ========================================================================
    //
    //   Helper routines
    //
    // ========================================================================

    static inline pixword shl(pixword value, unsigned shift)
    // -------------------------------------------------------------------------
    //   Shift right guaranteeing a zero for a large shift (even on x86)
    // -------------------------------------------------------------------------
    {
        return shift < BPW ? value << shift : 0;
    }


    static inline pixword shr(pixword value, unsigned shift)
    // -------------------------------------------------------------------------
    //   Shift right guaranteeing a zero for a large shift (even on x86)
    // -------------------------------------------------------------------------
    {
        return shift < BPW ? value >> shift : 0;
    }


    static inline pixword shlc(pixword value, unsigned shift)
    // -------------------------------------------------------------------------
    //   Shift x1 by the complement of the given shift
    // -------------------------------------------------------------------------
    {
        return shl(value, BPW - shift);
    }


    static pixword shrc(pixword value, unsigned shift)
    // -------------------------------------------------------------------------
    //   Shift x2 by the complement of the given shift
    // -------------------------------------------------------------------------
    {
        return shr(value, BPW - shift);
    }


    template <typename Int>
    static inline uint64_t rotate(Int bits, uint shift)
    // --------------------------------------------------------------------
    //   Rotate a 64-bit pattern by the given amount
    // --------------------------------------------------------------------
    {
        enum
        {
            BIT_SIZE = sizeof(bits) * 8
        };
        shift %= BIT_SIZE;
        bits = ((bits >> shift) | (bits << (BIT_SIZE - shift)));
        return bits;
    }


    static inline pixword bitswap(pixword bits)
    // ------------------------------------------------------------------------
    //  Flip left and right in the input bits
    // ------------------------------------------------------------------------
    {
        pixword result = 0;
        for (uint bit = 0; bit < 32; bit++)
            if ((bits >> bit) & 1)
                result |= 1U << bit;
        return result;
    }

    template <mode Dst, mode Src>
    static pixword convert(pixword data);
    // ------------------------------------------------------------------------
    //   Convert data from Src mode to Dst mode (converts the low bits)
    // ------------------------------------------------------------------------



    // =========================================================================
    //
    //   Operators for blit
    //
    // =========================================================================

public:
    static pixword blitop_set(pixword UNUSED dst,
                              pixword UNUSED src,
                              pixword        arg)
    // -------------------------------------------------------------------------
    //   This simply sets the color passed in arg
    // -------------------------------------------------------------------------
    {
        return arg;
    }

    static pixword blitop_source(pixword UNUSED dst,
                                 pixword        src,
                                 pixword UNUSED arg)
    // -------------------------------------------------------------------------
    //   This simly sets the color from the source
    // -------------------------------------------------------------------------
    {
        return src;
    }

    static pixword blitop_xor(pixword dst, pixword src, pixword arg)
    // -------------------------------------------------------------------------
    //   Perform a 'xor' graphical operation (can also be used for inverting)
    // -------------------------------------------------------------------------
    {
        dst ^= src ^ arg;
        return dst;
    }


    static pixword blitop_and(pixword dst, pixword src, pixword arg)
    // -------------------------------------------------------------------------
    //   Perform the 'and' operation
    // -------------------------------------------------------------------------
    {
        dst &= src ^ arg;
        return dst;
    }


    static pixword blitop_or(pixword dst, pixword src, pixword arg)
    // -------------------------------------------------------------------------
    //   Perform a 'or' graphical operation
    // -------------------------------------------------------------------------
    {
        dst |= src ^ arg;
        return dst;
    }


    static pixword blitop_nop(pixword dst, pixword UNUSED s, pixword UNUSED a)
    // ------------------------------------------------------------------------
    //   No graphical operation
    // ------------------------------------------------------------------------
    {
        return dst;
    }


    static pixword blitop_draw(pixword dst, pixword src, pixword arg)
    // ------------------------------------------------------------------------
    //   Colorize based on source
    // ------------------------------------------------------------------------
    {
        return (dst & src) | (arg & ~src);
    }


    static pixword blitop_background(pixword dst, pixword src, pixword arg)
    // ------------------------------------------------------------------------
    //   Colorize based on source
    // ------------------------------------------------------------------------
    {
        return (dst & ~src) | (arg & src);
    }
};



// ============================================================================
//
//   Color template specializations for various BPP
//
// ============================================================================

template <>
union blitter::color<blitter::mode::MONOCHROME>
// ------------------------------------------------------------------------
//  Color representation (1-bit, e.g. DM42)
// ------------------------------------------------------------------------
{
    color(uint8_t red, uint8_t green, uint8_t blue)
        : value((red + green + green + blue) / 4 >= 128)
    { }
    color(pixword pix): value(pix != 0) {}

    uint8_t red()
    {
        return value * 255;
    }
    uint8_t green()
    {
        return value * 255;
    }
    uint8_t blue()
    {
        return value * 255;
    }

    enum
    {
        BPP = 1
    };

  public:
    bool value : 1; // The color value is 0 or 1
};


template <>
union blitter::color<blitter::mode::MONOCHROME_REVERSE>
// ------------------------------------------------------------------------
//  Color representation (1-bit, e.g. DM42)
// ------------------------------------------------------------------------
//  On the DM42. white is 0 and black is 1
{
    color(uint8_t red, uint8_t green, uint8_t blue)
        : value((red + green + green + blue) / 4 < 128)
    {
    }
    color(pixword pix): value(pix == 0) {}

    uint8_t red()
    {
        return !value * 255;
    }
    uint8_t green()
    {
        return !value * 255;
    }
    uint8_t blue()
    {
        return !value * 255;
    }

    enum
    {
        BPP = 1
    };

  public:
    bool value : 1; // The color value is 0 or 1
};


template <>
union blitter::color<blitter::mode::GRAY_4BPP>
// ------------------------------------------------------------------------
//  Color representation (4-bit, e.g. HP50G)
// ------------------------------------------------------------------------
//  On the HP50G, 0xF is black, 0x0 is white
{
    color(uint8_t red, uint8_t green, uint8_t blue)
        : value(0xF - (red + green + green + blue) / 64)
    {}
    color(pixword pix): value(pix & 15) {}

    uint8_t red()
    {
        return (0xF - value) * 0x11;
    }
    uint8_t green()
    {
        return red();
    }
    uint8_t blue()
    {
        return red();
    }

    enum
    {
        BPP = 4
    };

  public:
    uint8_t value : 4;
};


template <>
union blitter::color<blitter::mode::RGB_16BPP>
// ------------------------------------------------------------------------
//  Color representation (16-bit, e.g. HP Prime)
// ------------------------------------------------------------------------
{
    struct rgb16
    {
        rgb16(uint8_t red, uint8_t green, uint8_t blue)
            : blue(blue),
              green(green),
              red(red)
        {
        }
        uint8_t blue  : 5;
        uint8_t green : 6;
        uint8_t red   : 5;
    } PACKED rgb16;
    uint16_t value : 16;

    enum
    {
        BPP = 16
    };

  public:
    // Build a color from normalized RGB values
    color(uint8_t red, uint8_t green, uint8_t blue)
        : rgb16(red >> 3, green >> 2, blue >> 3)
    {}
    color(pixword pix): value(pix & 0xFFFF) {}

    uint8_t red()
    {
        return (rgb16.red << 3) | (rgb16.red & 0x7);
    }
    uint8_t green()
    {
        return (rgb16.green << 2) | (rgb16.green & 0x3);
    }
    uint8_t blue()
    {
        return (rgb16.blue << 3) | (rgb16.blue & 0x7);
    }
} PACKED;


// ============================================================================
//
//   Pattern template specializations for various BPP
//
// ============================================================================

template <blitter::mode Mode>
template <uint N>
inline blitter::pattern<Mode>::pattern(blitter::color<Mode> colors[N])
// ------------------------------------------------------------------------
//  Build a checkerboard from 2 or 4 colors in an array
// ------------------------------------------------------------------------
{
    enum
    {
        BPP = blitter::color<Mode>::BPP
    };
    for (uint shift = 0; shift < 64 / BPP; shift++)
    {
        uint index = (shift + ((shift / SIZE) % N)) % N;
        bits |= uint64_t(colors[index].value) << (shift * BPP);
    }
}

template <blitter::mode Mode>
inline blitter::pattern<Mode>::pattern(color a, color b)
// ------------------------------------------------------------------------
//   Build a pattern from two colors
// ------------------------------------------------------------------------
//   Is there any way to use delegating constructors here?
{
    color colors[2] = { a, b };
    *this           = pattern(colors);
}

template <blitter::mode Mode>
inline blitter::pattern<Mode>::pattern(color a, color b, color c, color d)
// ------------------------------------------------------------------------
//   Build a pattern from four colors
// ------------------------------------------------------------------------
{
    color colors[4] = { a, b, c, d };
    *this           = pattern(colors);
}


template <>
union blitter::pattern<blitter::mode::MONOCHROME>
// ------------------------------------------------------------------------
//   Pattern for 1-bit bitmaps (e.g. fonts)
// ------------------------------------------------------------------------
{
    uint64_t bits;

    enum
    {
        BPP  = 1,
        SIZE = 8
    }; // 64-bit = 8x8 1-bit pattern
    enum : uint64_t
    {
        SOLID = 0xFFFFFFFFFFFFFFFFull
    };
    using color = blitter::color<MONOCHROME>;

  public:
    // Build a solid pattern from a single color
    pattern(color c) : bits(c.value * SOLID)
    {
    }

    // Build a checkered pattern for a given RGB level
    pattern(uint8_t red, uint8_t green, uint8_t blue) : bits(0)
    {
        // Compute a gray value beteen 0 and 64, number of pixels to fill
        uint16_t gray = (red + green + green + blue + 4) / 16;
        if (gray == 32) // Hand tweak 50% gray
            bits = 0xAAAAAAAAAAAAAAAAull;
        else
            // Generate a pattern with "gray" bits lit "at random"
            for (int bit = 0; bit < 64 && gray; bit++, gray--)
                bits |= 1ULL << (79 * bit % 64);
    }

    // Shared constructors
    template <uint N>
    pattern(color colors[N]);
    pattern(color a, color b);
    pattern(color a, color b, color c, color d);

    // Pattern from bits
    pattern(uint64_t bits = ~0ULL): bits(bits) {}

    // Some pre-defined shades of gray
    static const pattern black;
    static const pattern gray10;
    static const pattern gray25;
    static const pattern gray50;
    static const pattern gray75;
    static const pattern gray90;
    static const pattern white;
    static const pattern invert;
};


template <>
union blitter::pattern<blitter::mode::MONOCHROME_REVERSE>
// ------------------------------------------------------------------------
//   Pattern for 1-bit screens (DM42)
// ------------------------------------------------------------------------
{
    uint64_t bits;

    enum
    {
        BPP  = 1,
        SIZE = 8
    }; // 64-bit = 8x8 1-bit pattern
    enum : uint64_t
    {
        SOLID = 0xFFFFFFFFFFFFFFFFull
    };
    using color = blitter::color<MONOCHROME_REVERSE>;

  public:
    // Build a solid pattern from a single color
    pattern(color c) : bits(c.value * SOLID)
    {
    }

    // Build a checkered pattern for a given RGB level
    pattern(uint8_t red, uint8_t green, uint8_t blue) : bits(0)
    {
        // Compute a gray value beteen 0 and 64, number of pixels to fill
        uint16_t gray = (red + green + green + blue + 4) / 16;
        if (gray == 32) // Hand tweak 50% gray
            bits = 0xAAAAAAAAAAAAAAAAull;
        else
            // Generate a pattern with "gray" bits lit "at random"
            for (int bit = 0; bit < 64 && gray; bit++, gray--)
                bits |= 1ULL << (79 * bit % 64);
    }

    // Shared constructors
    template <uint N>
    pattern(color colors[N]);
    pattern(color a, color b);
    pattern(color a, color b, color c, color d);

    // Pattern from bits
    pattern(uint64_t bits = ~0ULL): bits(bits) {}

    // Some pre-defined shades of gray
    static const pattern black;
    static const pattern gray10;
    static const pattern gray25;
    static const pattern gray50;
    static const pattern gray75;
    static const pattern gray90;
    static const pattern white;
    static const pattern invert;
};


template <>
union blitter::pattern<blitter::mode::GRAY_4BPP>
// ------------------------------------------------------------------------
//   Pattern for 4-bit screens (HP50G)
// ------------------------------------------------------------------------
{
    uint64_t bits;

    enum
    {
        BPP  = 4,
        SIZE = 4
    }; // 64-bit = 4x4 4-bit pattern
    enum : uint64_t
    {
        SOLID = 0x1111111111111111ull
    };
    using color = blitter::color<GRAY_4BPP>;

  public:
    // Build a solid pattern from a single color
    pattern(color c) : bits(c.value * SOLID)
    {
    }

    // Build a checkered pattern for a given RGB level
    pattern(uint8_t red, uint8_t green, uint8_t blue) : bits(0)
    {
        // Compute a gray value beteen 0 and 15
        uint16_t gray = (red + green + green + blue + 4) / 64;
        bits          = SOLID * (0xF - gray);
    }

    // Shared constructors
    template <uint N>
    pattern(color colors[N]);
    pattern(color a, color b);
    pattern(color a, color b, color c, color d);

    // Pattern from bits
    pattern(uint64_t bits = 0ULL): bits(bits) {}

    // Some pre-defined shades of gray
    static const pattern black;
    static const pattern gray10;
    static const pattern gray25;
    static const pattern gray50;
    static const pattern gray75;
    static const pattern gray90;
    static const pattern white;
    static const pattern invert;
};


template <>
union blitter::pattern<blitter::mode::RGB_16BPP>
// ------------------------------------------------------------------------
//   Pattern for 16-bit screens (HP Prime)
// ------------------------------------------------------------------------
{
    uint64_t bits;

    enum
    {
        BPP  = 16,
        SIZE = 2
    }; // 64-bit = 4x4 4-bit pattern
    enum : uint64_t
    {
        SOLID = 0x0001000100010001ull
    };
    using color = blitter::color<RGB_16BPP>;

  public:
    // Build a solid pattern from a single color
    pattern(color c) : bits(c.value * SOLID)
    {
    }

    // Build a checkered pattern for a given RGB level
    pattern(uint8_t red, uint8_t green, uint8_t blue) : bits(0)
    {
        // Compute a gray value beteen 0 and 15
        color c(red, green, blue);
        bits = SOLID * c.value;
    }

    // Shared constructors
    template <uint N>
    pattern(color colors[N]);
    pattern(color a, color b);
    pattern(color a, color b, color c, color d);

    // Pattern from bits
    pattern(uint64_t bits = 0): bits(bits) {}

    // Some pre-defined shades of gray
    static const pattern black;
    static const pattern gray10;
    static const pattern gray25;
    static const pattern gray50;
    static const pattern gray75;
    static const pattern gray90;
    static const pattern white;
    static const pattern invert;
};


template <blitter::clipping Clip,
          typename Dst,
          typename Src,
          blitter::mode CMode>
void blitter::blit(Dst           &dst,
                   const Src     &src,
                   const rect    &drect,
                   const point   &spos,
                   blitop         op,
                   pattern<CMode> colors)
// ----------------------------------------------------------------------------
//   Generalized multi-bpp blitting routine
// ----------------------------------------------------------------------------
//   This transfers pixels from 'src' to 'dst' (which can be equal)
//   - targeting a rectangle defined by (x1, x2, y1, y2)
//   - fetching pixels from (x,y) in the source
//   - applying the given operation in 'op'
//
//   Everything is so that the compiler can optimize code away
//
//   The code selects the correct direction for copies within the same
//   surface, so it is safe to use for operations like scrolling.
//
//   We pass the bits-per-pixel as arguments to make it easier for the
//   optimizer to replace multiplications with shifts when we pass a
//   constant. Additional flags can be used to statically disable sections
//   of the code.
//
//   An arbitrary blitop is passed, which can be used to process each set of
//   pixels in turn. That operation is dependent on the respective bits per
//   pixelss, and can be used e.g. to do bit-plane conversions. See how this
//   is used in DrawText to colorize 1bpp bitplanes. The source and color
//   pattern are both aligned to match the destination before the operator
//   is called. So if the source is 1bpp and the destination is 4bpp, you
//   might end up with the 8 low bits of the source being the bit pattern
//   that applies to the 8 nibbles in the destination word.
{
    bool       clip_src = Clip & CLIP_SRC;
    bool       clip_dst = Clip & CLIP_DST;
    bool       skip_src = Clip & SKIP_SOURCE;
    bool       skip_col = Clip & SKIP_COLOR;
    bool       overlap  = Clip & OVERLAP;

    coord      x1       = drect.x1;
    coord      y1       = drect.y1;
    coord      x2       = drect.x2;
    coord      y2       = drect.y2;
    coord      x        = spos.x;
    coord      y        = spos.y;

    const uint SBPP     = Src::BPP;
    const uint DBPP     = Dst::BPP;
    const uint CBPP     = color<CMode>::BPP;
    const mode DMode    = Dst::blitmode;
    const mode SMode    = Src::blitmode;

    if (clip_src)
    {
        coord sx1 = src.drawable.x1;
        coord sx2 = src.drawable.x2;
        coord sy1 = src.drawable.y1;
        coord sy2 = src.drawable.y2;

        if (x < sx1)
        {
            x1 += sx1 - x;
            x = sx1;
        }
        if (x + x2 - x1 > sx2)
            x2 = sx2 - x + x1;
        if (y < sy1)
        {
            y1 += sy1 - y;
            y = sy1;
        }
        if (y + y2 - y1 > sy2)
            y2 = sy2 - y + y1;
    }

    if (clip_dst)
    {
        // Clipping based on target
        coord dx1 = dst.drawable.x1;
        coord dx2 = dst.drawable.x2;
        coord dy1 = dst.drawable.y1;
        coord dy2 = dst.drawable.y2;

        if (x1 < dx1)
        {
            if (dst.horizontal_swap() == src.horizontal_swap())
                x += dx1 - x1;
            x1 = dx1;
        }
        if (x2 > dx2)
        {
            if (dst.horizontal_swap() != src.horizontal_swap())
                x -= dx2 - x2;
            x2 = dx2;
        }
        if (y1 < dy1)
        {
            y += dy1 - y1;
            y1 = dy1;
        }
        if (y2 > dy2)
            y2 = dy2;
    }

    // Some platforms have the weird idea of flipping left and right
    dst.horizontal_adjust(x1, x2);
    dst.vertical_adjust(y1, y2);

    // Bail out if there is no effect
    if (x1 > x2 || y1 > y2)
        return;

    // Source coordinates
    coord sl = x;
    coord sr = sl + x2 - x1;
    coord st = y;
    coord sb = st + y2 - y1;

    src.horizontal_adjust(sl, sr);
    src.vertical_adjust(st, sb);

    // Check whether we need to go forward or backward along X or Y
    bool     xback  = overlap && x < x1;
    bool     yback  = overlap && y < y1;
    int      xdir   = xback ? -1 : 1;
    int      ydir   = yback ? -1 : 1;
    coord    dx1    = xback ? x2 : x1;
    coord    dx2    = xback ? x1 : x2;
    coord    dy1    = yback ? y2 : y1;
    coord    sx1    = xback ? sr : sl;
    coord    sy1    = yback ? sb : st;
    coord    ycount = y2 - y1;

    // Pointers to word containing start and end pixel
    offset   do1    = dst.pixel_offset(dx1, dy1);
    offset   do2    = dst.pixel_offset(dx2, dy1);
    offset   so     = skip_src ? 0 : src.pixel_offset(sx1, sy1);
    offset   dod    = dst.pixel_offset(0, ydir);
    offset   sod    = src.pixel_offset(0, ydir);

    // X1 and x2 pixel shift
    unsigned cshift = surface<CMode>::BPP == 16 ? 48
                    : surface<CMode>::BPP == 4  ? 20
                    : surface<CMode>::BPP == 1  ? 9
                                                : 0;
    unsigned cxs    = xdir * BPW * CBPP / DBPP;

    // Shift adjustment from source to destination
    unsigned dls    = dst.pixel_shift(do1);
    unsigned drs    = dst.pixel_shift(do2);
    unsigned dws    = xback ? drs : dls;
    unsigned sws    = skip_src ? 0 : src.pixel_shift(so);
    unsigned sadj   = (int) (sws * DBPP - dws * SBPP) / (int) DBPP;
    unsigned sxadj  = xdir * (int) (SBPP * BPW / DBPP);

    // X1 and x2 masks
    pixword  ones   = ~0U;
    pixword  lmask  = ones << dls;
    pixword  rmask  = shrc(ones, drs + DBPP);
    pixword  dmask1 = xback ? rmask : lmask;
    pixword  dmask2 = xback ? lmask : rmask;

    // Adjust the color pattern based on starting point
    uint64_t cdata64 = skip_col
                         ? blitter::pattern<CMode>::white.bits
                         : rotate(colors.bits, dx1 * CBPP + dy1 * cshift - dws);

    // Loop on all lines
    while (ycount-- >= 0)
    {
        pixword  dmask = dmask1;
        bool     xdone = false;
        pixword  sdata = 0;
        pixword  cdata = 0;
        pixword *dp1   = dst.pixel_address(do1);
        pixword *dp2   = dst.pixel_address(do2);
        pixword *sp    = skip_src ? dp1 : src.pixel_address(so);
        pixword *dp    = dp1;
        pixword  smem  = sp[0];
        pixword  snew  = smem;

        if (xback)
            sadj -= sxadj;

        do
        {
            xdone = dp == dp2;
            if (xdone)
                dmask &= dmask2;

            if (!skip_src)
            {
                unsigned nextsadj = sadj + sxadj;

                // Check if we change source word
                int      skip     = nextsadj >= BPW;
                if (skip)
                {
                    sp += xdir;
                    smem = snew;
                    ASSERT(sp >= src.pixels);
                    snew = sp[0];
                }

                nextsadj %= BPW;
                sadj %= BPW;
                if (sadj)
                    if (xback)
                        sdata = shlc(smem, nextsadj) | shr(snew, nextsadj);
                    else
                        sdata = shlc(snew, sadj) | shr(smem, sadj);
                else
                    sdata = xback ? snew : smem;

                sadj = nextsadj;
            }
            if (!skip_col)
            {
                cdata   = cdata64;
                cdata64 = rotate(cdata64, cxs);
            }

            ASSERT(dp >= dst.pixels);
            ASSERT(dp <= dst.pixels + (dst.scanline*dst.h*DBPP+(BPW-1))/BPW);
            pixword ddata = dp[0];
            pixword sdc = skip_src ? sdata : convert<DMode, SMode>(sdata);
            pixword cdc = skip_col ? cdata64 : convert<DMode, CMode>(cdata);
            pixword tdata = op(ddata, sdc, cdc);
            *dp           = (tdata & dmask) | (ddata & ~dmask);

            dp += xdir;
            dmask = ~0U;
            smem  = snew;
        } while (!xdone);

        // Move to next line
        dy1 += ydir;
        do1 += dod;
        do2 += dod;
        so += sod;
        sws     = skip_src ? 0 : src.pixel_shift(so);
        dls     = dst.pixel_shift(do1);
        drs     = dst.pixel_shift(do2);
        dws     = xback ? drs : dls;
        lmask   = ones << dls;
        rmask   = shrc(ones, drs + DBPP);
        dmask1  = xback ? rmask : lmask;
        dmask2  = xback ? lmask : rmask;
        cdata64 = rotate(colors.bits, dx1 * CBPP + dy1 * cshift - dws);
        sws     = src.pixel_shift(so);
        sadj    = (int) (sws * DBPP - dws * SBPP) / (int) DBPP;
    }
}


// ============================================================================
//
//   Template specialization for horizontal adjustment
//
// ============================================================================



template <>
inline void blitter::surface<blitter::MONOCHROME_REVERSE>::horizontal_adjust(
    coord &x1,
    coord &x2) const
// ----------------------------------------------------------------------------
//   On the DM42, we need horizontal adjustment for coordinates
// ----------------------------------------------------------------------------
{
    coord ox1 = swapsize - x2;
    x2        = swapsize - x1;
    x1        = ox1;
}


template <>
inline bool blitter::surface<blitter::MONOCHROME_REVERSE>::horizontal_swap()
// ----------------------------------------------------------------------------
//   On the DM42, we need horizontal adjustment for coordinates
// ----------------------------------------------------------------------------
{
    return true;
}


template <>
inline void blitter::surface<blitter::RGB_16BPP>::horizontal_adjust(
    coord &x1,
    coord &x2) const
// ----------------------------------------------------------------------------
//   On the DM42, we need horizontal adjustment for coordinates
// ----------------------------------------------------------------------------
{
    coord ox1 = swapsize - x2;
    x2        = swapsize - x1;
    x1        = ox1;
}


template <>
inline bool blitter::surface<blitter::RGB_16BPP>::horizontal_swap()
// ----------------------------------------------------------------------------
//   On the DM42, we need horizontal adjustment for coordinates
// ----------------------------------------------------------------------------
{
    return true;
}



// ============================================================================
//
//   Template specializations for blitops
//
// ============================================================================

template <blitter::mode Dst, blitter::mode Src>
inline blitter::pixword blitter::convert(pixword data)
// ----------------------------------------------------------------------------
//   Convert low bits of data from Src mode to Dst mode
// ----------------------------------------------------------------------------
{
    return data;
}


#define CONVERT(dmode, smode)                                   \
template <>                                                     \
inline blitter::pixword                                         \
blitter::convert<blitter::mode::dmode,                          \
                 blitter::mode::smode>(pixword UNUSED data)

CONVERT(MONOCHROME, MONOCHROME_REVERSE)
// ----------------------------------------------------------------------------
//   The MONOCHROME_REVERSE mode flips black and white
// ----------------------------------------------------------------------------
{
    return ~data;
}


CONVERT(MONOCHROME_REVERSE, MONOCHROME)
// ----------------------------------------------------------------------------
//   The MONOCHROME_REVERSE mode flips black and white
// ----------------------------------------------------------------------------
{
    return ~data;
}


CONVERT(GRAY_4BPP, MONOCHROME)
// ----------------------------------------------------------------------------
//   Convert a monochrome bitmap to Gray 4 BPP
// ----------------------------------------------------------------------------
{
    pixword cvt = 0;
    for (unsigned shift = 0; shift < 8; shift++)
        if (~data & (1 << shift))
            cvt |= 0xF << (4 * shift);
    return cvt;
}


CONVERT(RGB_16BPP, MONOCHROME)
// ----------------------------------------------------------------------------
//   Convert a monochrome bitmap to RGB_16BPP
// ----------------------------------------------------------------------------
{
    pixword cvt = 0;
    for (unsigned shift = 0; shift < 2; shift++)
        if (~data & (1 << shift))
            cvt |= 0xFFFF << (16 * shift);
    return cvt;
}

CONVERT(GRAY_4BPP, MONOCHROME_REVERSE)
// ----------------------------------------------------------------------------
//   Convert a monochrome bitmap to Gray 4 BPP
// ----------------------------------------------------------------------------
{
    pixword cvt = 0;
    for (unsigned shift = 0; shift < 8; shift++)
        if (data & (1 << shift))
            cvt |= 0xF << (4 * shift);
    return cvt;
}


CONVERT(RGB_16BPP, MONOCHROME_REVERSE)
// ----------------------------------------------------------------------------
//   Convert a monochrome bitmap to RGB_16BPP
// ----------------------------------------------------------------------------
{
    // Expand the low 2 bits from the source into a 32-bit cvt
    pixword cvt = 0;
    for (unsigned shift = 0; shift < 2; shift++)
        if (data & (1 << shift))
            cvt |= 0xFFFF << (16 * shift);
    return cvt;
}

#undef CONVERT



// ============================================================================
//
//    Text rendering operations
//
// ============================================================================

template <blitter::mode Mode>
template <blitter::clipping Clip>
blitter::coord blitter::surface<Mode>::glyph(coord       x,
                                             coord       y,
                                             unicode     codepoint,
                                             const font *f,
                                             pattern     colors,
                                             blitop      op)
// ----------------------------------------------------------------------------
//   Render a glyph on the given surface
// ----------------------------------------------------------------------------
{
    font::glyph_info g;
    if (f->glyph(codepoint, g))
    {
        // Bitmap may be misaligned, if so, fixup
        uintptr_t bma = (uintptr_t) g.bitmap;
        g.bx += 8 * (bma & 3);
        bma &= ~3;
        surface<MONOCHROME> source((pixword *) bma, g.bw, g.bh);
        rect  dest(x + g.x, y + g.y, x + g.x + g.w - 1, y + g.y + g.h - 1);
        point spos(g.bx, g.by);
        blit<Clip>(*this, source, dest, spos, op, colors);
        x += g.advance;
    }
    return x;
}


template <blitter::mode Mode>
template <blitter::clipping Clip>
blitter::coord blitter::surface<Mode>::glyph(coord       x,
                                             coord       y,
                                             unicode     codepoint,
                                             const font *f,
                                             pattern     fg,
                                             pattern     bg)
// ----------------------------------------------------------------------------
//   Render a glyph with a foreground and background
// ----------------------------------------------------------------------------
{
    font::glyph_info g;
    if (f->glyph(codepoint, g))
    {
        // Fill background
        fill<Clip>(x, y, x + g.advance - 1, y + f->height() - 1, bg);

        // Bitmap may be misaligned, if so, fixup
        uintptr_t bma = (uintptr_t) g.bitmap;
        g.bx += 8 * (bma & 3);
        bma &= ~3;
        surface<MONOCHROME> source((pixword *) bma, g.bw, g.bh);
        rect  dest(x + g.x, y + g.y, x + g.x + g.w - 1, y + g.y + g.h - 1);
        point spos(g.bx, g.by);
        blit<Clip>(*this, source, dest, spos, blitop_draw, fg);
        x += g.advance;
    }
    return x;
}


template <blitter::mode Mode>
template <blitter::clipping Clip>
blitter::coord blitter::surface<Mode>::text(coord       x,
                                            coord       y,
                                            utf8        text,
                                            const font *f,
                                            pattern     colors,
                                            blitop      op)
// ----------------------------------------------------------------------------
//   Render a glyph on the given surface
// ----------------------------------------------------------------------------
{
    while (*text)
    {
        unicode cp = utf8_codepoint(text);
        text       = utf8_next(text);
        x          = glyph<Clip>(x, y, cp, f, colors, op);
    }
    return x;
}


template <blitter::mode Mode>
template <blitter::clipping Clip>
blitter::coord blitter::surface<Mode>::text(coord       x,
                                            coord       y,
                                            utf8        text,
                                            const font *f,
                                            pattern     fg,
                                            pattern     bg)
// ----------------------------------------------------------------------------
//   Render a text with a foreground and background
// ----------------------------------------------------------------------------
{
    while (*text)
    {
        unicode cp = utf8_codepoint(text);
        text       = utf8_next(text);
        x          = glyph<Clip>(x, y, cp, f, fg, bg);
    }
    return x;
}

template <blitter::mode Mode>
template <blitter::clipping Clip>
blitter::coord blitter::surface<Mode>::text(coord       x,
                                            coord       y,
                                            utf8        text,
                                            size_t      len,
                                            const font *f,
                                            pattern     colors,
                                            blitop      op)
// ----------------------------------------------------------------------------
//   Render a glyph on the given surface
// ----------------------------------------------------------------------------
{
    while (len)
    {
        unicode cp = utf8_codepoint(text);
        size_t  sz = utf8_size(cp);
        if (sz > len)
            break; // Defensive coding, see #101
        len -= sz;
        x = glyph<Clip>(x, y, cp, f, colors, op);
        text += sz;
    }
    return x;
}


template <blitter::mode Mode>
template <blitter::clipping Clip>
blitter::coord blitter::surface<Mode>::text(coord       x,
                                            coord       y,
                                            utf8        text,
                                            size_t      len,
                                            const font *f,
                                            pattern     fg,
                                            pattern     bg)
// ----------------------------------------------------------------------------
//   Render a text with a foreground and background
// ----------------------------------------------------------------------------
{
    while (len)
    {
        unicode cp = utf8_codepoint(text);
        size_t  sz = utf8_size(cp);
        if (sz > len)
            break; // Defensive coding, see #101
        len -= sz;
        x = glyph<Clip>(x, y, cp, f, fg, bg);
        text += sz;
    }
    return x;
}


template <blitter::mode Mode>
template <blitter::clipping Clip>
void blitter::surface<Mode>::line(coord   x1,
                                  coord   y1,
                                  coord   x2,
                                  coord   y2,
                                  size    width,
                                  pattern fg)
// --------------------------------------------------------------------
//   Draw a line between the given coordinates
// --------------------------------------------------------------------
{
    if (Clip & CLIP_ALL)
    {
        if (x1 < drawable.x1)
        {
            if (x2 <= x1)
                return;
            y1 = y2 + (drawable.x1 - x2) * (y1 - y2) / (x1 - x2);
            x1 = drawable.x1;
        }
        if (x1 > drawable.x2)
        {
            if (x2 >= x1)
                return;
            y1 = y2 + (drawable.x2 - x2) * (y1 - y2) / (x1 - x2);
            x1 = drawable.x2;
        }
        if (x2 < drawable.x1)
        {
            if (x1 <= x2)
                return;
            y2 = y1 + (drawable.x1 - x1) * (y2 - y1) / (x2 - x1);
            x2 = drawable.x1;
        }
        if (x2 > drawable.x2)
        {
            if (x1 >= x2)
                return;
            y2 = y1 + (drawable.x2 - x1) * (y1 - y2) / (x1 - x2);
            x2 = drawable.x2;
        }
        if (y1 < drawable.y1)
        {
            if (y2 <= y1)
                return;
            x1 = x2 + (drawable.y1 - y2) * (x1 - x2) / (y1 - y2);
            y1 = drawable.y1;
        }
        if (y1 > drawable.y2)
        {
            if (y2 >= y1)
                return;
            x1 = x2 + (drawable.y2 - y2) * (x1 - x2) / (y1 - y2);
            y1 = drawable.y2;
        }
        if (y2 < drawable.y1)
        {
            if (y1 <= y2)
                return;
            x2 = x1 + (drawable.y1 - y1) * (x2 - x1) / (y2 - y1);
            y2 = drawable.y1;
        }
        if (y2 > drawable.y2)
        {
            if (y1 >= y2)
                return;
            x2 = x1 + (drawable.y2 - y1) * (x1 - x2) / (y1 - y2);
            y2 = drawable.y2;
        }
    }
    if (!width)
        width = 1;

    size  dx = x1 > x2 ? x1 - x2 : x2 - x1;
    size  dy = y1 > y2 ? y1 - y2 : y2 - y1;
    int   sx = x2 < x1 ? -1 : 1;
    int   sy = y2 < y1 ? -1 : 1;
    coord d  = dx - dy;
    coord x  = x1;
    coord y  = y1;
    size  wn = (width - 1) / 2;
    size  wp = width / 2;

    while (true)
    {
        fill<Clip>(x - wn, y - wn, x + wp, y + wp, fg);
        if (x == x2 && y == y2)
            break;
        if (d >= 0)
        {
            x += sx;
            d -= dy;
        }
        if (d < 0)
        {
            y += sy;
            d += dx;
        }
    }
}


template <blitter::mode Mode>
template <blitter::clipping Clip>
void blitter::surface<Mode>::ellipse(coord   x1,
                                     coord   y1,
                                     coord   x2,
                                     coord   y2,
                                     size    width,
                                     pattern fg)
// ----------------------------------------------------------------------------
//   Draw an ellipse between the given coordinates
// ----------------------------------------------------------------------------
{
    coord xc = (x1 + x2) / 2;
    coord yc = (y1 + y2) / 2;
    size  a  = (x2 > x1 ? x2 - x1 : x1 - x2)/2;
    size  b  = (y2 > y1 ? y2 - y1 : y1 - y2)/2;
    uint  a2 = uint(a) * uint(a);
    uint  b2 = uint(b) * uint(b);
    int   d  = 0;
    coord x  = a;
    coord y  = 0;
    size  wn = width / 2;
    size  wp = (width - 1) / 2;

    do
    {
        if (width)
        {
            fill<Clip>(xc + x - wn, yc + y - wn, xc + x + wp, yc + y + wp, fg);
            fill<Clip>(xc - x - wn, yc + y - wn, xc - x + wp, yc + y + wp, fg);
            fill<Clip>(xc + x - wn, yc - y - wn, xc + x + wp, yc - y + wp, fg);
            fill<Clip>(xc - x - wn, yc - y - wn, xc - x + wp, yc - y + wp, fg);
        }
        else
        {
            fill<Clip>(xc - x, yc - y, xc + x + 1, yc - y + 1, fg);
            fill<Clip>(xc - x, yc + y, xc + x + 1, yc + y + 1, fg);
        }

        int dx = b2 * x;
        int dy = a2 * y;
        if (d <= 0)
        {
            y++;
            d += dy;
        }
        if (d >= 0)
        {
            x--;
            d -= dx;
        }
    }
    while (x >= 0);
}


template <blitter::mode Mode>
template <blitter::clipping Clip>
void blitter::surface<Mode>::rounded_rectangle(coord   x1,
                                               coord   y1,
                                               coord   x2,
                                               coord   y2,
                                               size    r,
                                               size    width,
                                               pattern fg)
// ----------------------------------------------------------------------------
//   Draw a rounded rectangle between the given coordinates
// ----------------------------------------------------------------------------
{
    coord xc = (x1 + x2) / 2;
    coord yc = (y1 + y2) / 2;
    size  a  = (x2 > x1 ? x2 - x1 : x1 - x2)/2;
    size  b  = (y2 > y1 ? y2 - y1 : y1 - y2)/2;
    r /= 2;
    if (r > a)
        r = a;
    if (r > b)
        r = b;
    a -= r;
    b -= r;
    int   d  = r / 2;
    coord x  = r;
    coord y  = 0;
    size  wn = width / 2;
    size  wp = (width - 1) / 2;
    coord xl = xc - a;
    coord xr = xc + a;
    coord yt = yc - b;
    coord yb = yc + b;

    while (x >= y)
    {
        if (width)
        {
            fill<Clip>(xl - x - wn, yt - y - wn, xl - x + wp, yt - y + wp, fg);
            fill<Clip>(xl - y - wn, yt - x - wn, xl - y + wp, yt - x + wp, fg);
            fill<Clip>(xr + x - wn, yt - y - wn, xr + x + wp, yt - y + wp, fg);
            fill<Clip>(xr + y - wn, yt - x - wn, xr + y + wp, yt - x + wp, fg);
            fill<Clip>(xl - x - wn, yb + y - wn, xl - x + wp, yb + y + wp, fg);
            fill<Clip>(xl - y - wn, yb + x - wn, xl - y + wp, yb + x + wp, fg);
            fill<Clip>(xr + x - wn, yb + y - wn, xr + x + wp, yb + y + wp, fg);
            fill<Clip>(xr + y - wn, yb + x - wn, xr + y + wp, yb + x + wp, fg);
        }
        else
        {
            fill<Clip>(xl - x, yt - y, xr + x, yt - y, fg);
            fill<Clip>(xl - y, yt - x, xr + y, yt - x, fg);
            fill<Clip>(xl - x, yb + y, xr + x, yb + y, fg);
            fill<Clip>(xl - y, yb + x, xr + y, yb + x, fg);
        }

        y++;
        d -= y;
        if (d < 0)
        {
            x--;
            d += x;
        }
    }

    if (width)
    {
        fill<Clip>(xl - wn,     yt - r - wn, xr + wp,     yt - r + wp, fg);
        fill<Clip>(xl - wn,     yb + r - wn, xr + wp,     yb + r + wp, fg);
        fill<Clip>(xl - r - wn, yt - wn,     xl - r + wp, yb + wp,     fg);
        fill<Clip>(xr + r - wn, yt - wn,     xr + r + wp, yb + wp,     fg);
    }
    else
    {
        fill<Clip>(xl - r, yt, xr + r, yb, fg);
    }
}


#endif // BLITTER_H
