
#ifndef FONT_H
#define FONT_H
// ****************************************************************************
//  font.h                                                        DB48X project
// ****************************************************************************
//
//   File Description:
//
//     RPL font objects
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

#include "object.h"
#include "utf8.h"


RECORDER_DECLARE(fonts);
RECORDER_DECLARE(fonts_error);

struct font : object
// ----------------------------------------------------------------------------
//   Shared by all font objects
// ----------------------------------------------------------------------------
{
    typedef int16_t  fint;
    typedef uint16_t fuint;

    font(id type): object(type) { }

    struct glyph_info
    {
        byte_p bitmap;          // Bitmap we get the glyph from
        fint   bx;              // X position in bitmap
        fint   by;              // Y position in bitmap (always 0 today?)
        fuint  bw;              // Width of bitmap
        fuint  bh;              // Height of bitmap
        fint   x;               // X position of glyph when drawing
        fint   y;               // Y position of glyph when drawing
        fuint  w;               // Width of glyph
        fuint  h;               // Height of glyph
        fuint  advance;         // X advance to next character
        fuint  height;          // Y advance to next line
    };
    bool  glyph(unicode codepoint, glyph_info &g) const;
    fuint width(unicode codepoint) const
    {
        glyph_info g;
        if (glyph(codepoint, g))
            return g.advance;
        return 0;
    }
    fuint width(utf8 text) const
    {
        fuint result = 0;
        for (utf8 p = text; *p; p = utf8_next(p))
            result += width(utf8_codepoint(p));
        return result;
    }
    fuint width(utf8 text, size_t len) const
    {
        fuint result = 0;
        utf8 last = text + len;
        for (utf8 p = text; p < last; p = utf8_next(p))
            result += width(utf8_codepoint(p));
        return result;
    }
    fuint height(unicode codepoint) const
    {
        glyph_info g;
        if (glyph(codepoint, g))
            return g.advance;
        return 0;
    }
    fuint height() const;

public:
    SIZE_DECL(font)
    {
        byte_p p = payload(o);
        return ptrdiff(p, o) + leb128<size_t>(p);
    }
};
typedef const font *font_p;


struct sparse_font : font
// ----------------------------------------------------------------------------
//   An object representing a sparse font (one bitmap per character)
// ----------------------------------------------------------------------------
{
    sparse_font(id type = ID_sparse_font): font(type) {}
    OBJECT_DECL(sparse_font);
    bool glyph(unicode codepoint, glyph_info &g) const;
    fuint height();
};
typedef const sparse_font *sparse_font_p;


struct dense_font : font
// ----------------------------------------------------------------------------
//   An object representing a dense font (a single bitmap for all characters)
// ----------------------------------------------------------------------------
{
    dense_font(id type = ID_dense_font): font(type) {}
    OBJECT_DECL(dense_font);
    bool glyph(unicode codepoint, glyph_info &g) const;
    fuint height();
};
typedef const dense_font *dense_font_p;


struct dmcp_font : font
// ----------------------------------------------------------------------------
//   An object accessing the DMCP built-in fonts (and remapping to Unicode)
// ----------------------------------------------------------------------------
{
    dmcp_font(fint index, id type = ID_dense_font): font(type)
    {
        byte_p p = payload(this);
        leb128(p, index);
    }
    static size_t required_memory(id i, fint index)
    {
        return leb128size(i) + leb128size(index);
    }

    OBJECT_DECL(dense_font);
    fint index() const      { byte_p p = payload(this); return leb128<fint>(p); }

    bool glyph(unicode codepoint, glyph_info &g) const;
    fuint height();
};
typedef const dmcp_font *dmcp_font_p;


inline font::fuint font::height() const
// ----------------------------------------------------------------------------
//   Dynamic dispatch to the available font classes
// ----------------------------------------------------------------------------
{
    switch(type())
    {
    case ID_sparse_font: return ((sparse_font *)this)->height();
    case ID_dense_font:  return ((dense_font *)this)->height();
    case ID_dmcp_font:   return ((dmcp_font *)this)->height();
    default:
        record(fonts_error, "Unexpectd font type %d", type());
    }
    return false;
}

// Fonts for various parts of the user interface
extern font_p EditorFont;
extern font_p StackFont;
extern font_p ReducedFont;
extern font_p ErrorFont;
extern font_p MenuFont;
extern font_p HelpFont;
extern font_p HelpBoldFont;
extern font_p HelpItalicFont;
extern font_p HelpCodeFont;
extern font_p HelpTitleFont;
extern font_p HelpSubTitleFont;

void font_defaults();

// In the DM42 DMCP - Not fully Unicode capable
extern const dmcp_font_p LibMonoFont10x17;
extern const dmcp_font_p LibMonoFont11x18;
extern const dmcp_font_p LibMonoFont12x20;
extern const dmcp_font_p LibMonoFont14x22;
extern const dmcp_font_p LibMonoFont17x25;
extern const dmcp_font_p LibMonoFont17x28;
extern const dmcp_font_p SkrMono13x18;
extern const dmcp_font_p SkrMono18x24;
extern const dmcp_font_p Free42Font;

#endif // FONT_H
