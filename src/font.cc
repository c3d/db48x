// ****************************************************************************
//  font.cc                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     RPL Font objects
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

#include "font.h"

#include "dmcp.h"
#include "parser.h"
#include "recorder.h"
#include "renderer.h"
#include "runtime.h"

#include <stdio.h>
#include <stdlib.h>

RECORDER(fonts,         16, "Information about fonts");
RECORDER(sparse_fonts,  16, "Information about sparse fonts");
RECORDER(dense_fonts,   16, "Information about dense fonts");
RECORDER(dmcp_fonts,    16, "Information about DMCP fonts");
RECORDER(fonts_error,   16, "Information about fonts");
RECORDER(font_cache,    16, "Font cache data");


static const byte dmcpFontRPL[]
// ----------------------------------------------------------------------------
//   RPL object representing the various DMCP fonts
// ----------------------------------------------------------------------------
{
#define LEB128(id, fnt)      ((id) & 0x7F) | 0x80, ((id) >> 7) & 0x7F, (fnt)

    LEB128(object::ID_dmcp_font, 0),            // lib_mono
    LEB128(object::ID_dmcp_font, 1),
    LEB128(object::ID_dmcp_font, 2),
    LEB128(object::ID_dmcp_font, 3),
    LEB128(object::ID_dmcp_font, 4),
    LEB128(object::ID_dmcp_font, 5),

    LEB128(object::ID_dmcp_font, 10),           // Free42 (fixed size, very small)

    LEB128(object::ID_dmcp_font, 18),           // skr_mono
    LEB128(object::ID_dmcp_font, 21),           // skr_mono

};

// In the DM42 DMCP - Not fully Unicode capable
const dmcp_font_p LibMonoFont10x17 = (dmcp_font_p) (dmcpFontRPL +  0);
const dmcp_font_p LibMonoFont11x18 = (dmcp_font_p) (dmcpFontRPL +  3);
const dmcp_font_p LibMonoFont12x20 = (dmcp_font_p) (dmcpFontRPL +  6);
const dmcp_font_p LibMonoFont14x22 = (dmcp_font_p) (dmcpFontRPL +  9);
const dmcp_font_p LibMonoFont17x25 = (dmcp_font_p) (dmcpFontRPL + 12);
const dmcp_font_p LibMonoFont17x28 = (dmcp_font_p) (dmcpFontRPL + 15);
const dmcp_font_p Free42Font       = (dmcp_font_p) (dmcpFontRPL + 18);
const dmcp_font_p SkrMono13x18     = (dmcp_font_p) (dmcpFontRPL + 21);
const dmcp_font_p SkrMono18x24     = (dmcp_font_p) (dmcpFontRPL + 24);


font_p EditorFont;
font_p StackFont;
font_p ReducedFont;
font_p ErrorFont;
font_p MenuFont;
font_p HelpFont;
font_p HelpBoldFont;
font_p HelpItalicFont;
font_p HelpCodeFont;
font_p HelpTitleFont;
font_p HelpSubTitleFont;


void font_defaults()
// ----------------------------------------------------------------------------
//    Initialize the fonts for the user interface
// ----------------------------------------------------------------------------
{
#define GENERATED_FONT(name)                    \
    extern byte name##_sparse_font_data[];      \
    name = (font_p) name##_sparse_font_data;

    GENERATED_FONT(HelpFont);
    GENERATED_FONT(ReducedFont);
    GENERATED_FONT(StackFont);
    GENERATED_FONT(EditorFont);

    ErrorFont        = SkrMono13x18;
    MenuFont         = HelpFont;

    HelpBoldFont     = HelpFont;
    HelpItalicFont   = HelpFont;
    HelpCodeFont     = LibMonoFont11x18;
    HelpTitleFont    = StackFont;
    HelpSubTitleFont = ReducedFont;
}


struct font_cache
// ----------------------------------------------------------------------------
//   A data structure to accelerate access to font offsets for a given font
// ----------------------------------------------------------------------------
{
    // Use same size as font data
    using fint  = font::fint;
    using fuint = font::fuint;

    enum { MAX_GLYPHS = 128 };

    font_cache(): cache((data *) malloc(sizeof(data) * MAX_GLYPHS)), size(0) { }
    ~font_cache() { free(cache); }


    struct data
    // ------------------------------------------------------------------------
    //   Data in the cache
    // ------------------------------------------------------------------------
    {
        void set(font_p  font,
                 unicode codepoint,
                 byte_p  bitmap,
                 fint    x,
                 fint    y,
                 fuint   w,
                 fuint   h,
                 fuint   advance)
        {
            this->font      = font;
            this->codepoint = codepoint;
            this->bitmap    = bitmap;
            this->x         = x;
            this->y         = y;
            this->w         = w;
            this->h         = h;
            this->advance   = advance;
        }

        font_p  font;      // Font being cached
        byte_p  bitmap;    // Bitmap data for glyph
        unicode codepoint; // Codepoint in that font
        fint    x;         // X position (meaning depends on font type)
        fint    y;         // Y position (meaning depends on font type)
        fuint   w;         // Width (meaning depends on font type)
        fuint   h;         // Height (meaning depends on font type)
        fuint   advance;   // Advance to next character
    } __attribute((packed))__;


    data *lookup(font_p font, unicode codepoint)
    // ------------------------------------------------------------------------
    //   Lookup data in the LRU cache
    // ------------------------------------------------------------------------
    {
        if (size)
        {
            size_t count = size < MAX_GLYPHS ? size : size_t(MAX_GLYPHS);
            data  *last  = cache + (size + ~0) % MAX_GLYPHS;
            data  *d     = last;
            for (size_t i = 0; i < count; i++)
            {
                if (d->font == font && d->codepoint == codepoint)
                {
                    // Bring it back to front for faster lookup next
                    if (i > 0)
                        std::swap(*d, *last);
                    return last;
                }
                if (d-- < cache)
                    d = cache + count - 1;
            }
        }
        return nullptr;
    }


    data *insert(font_p  font,
                 unicode codepoint,
                 byte_p  bitmap,
                 fint    x,
                 fint    y,
                 fuint   w,
                 fuint   h,
                 fuint   advance)
    // ------------------------------------------------------------------------
    //   Insert a new entry in the cache
    // ------------------------------------------------------------------------
    {
        data *last = cache + (size++ % MAX_GLYPHS);
        last->set(font, codepoint, bitmap, x, y, w, h, advance);
        return last;
    }

private:
    data  *cache;
    size_t size;
} FontCache;


bool font::glyph(unicode codepoint, glyph_info &g) const
// ----------------------------------------------------------------------------
//   Dynamic dispatch to the available font classes
// ----------------------------------------------------------------------------
{
    if (codepoint == '\t')
    {
        bool result = glyph(' ', g);
        g.advance = Settings.TabWidth();
        return result;
    }

    switch(type())
    {
    case ID_sparse_font: return ((sparse_font *)this)->glyph(codepoint, g);
    case ID_dense_font:  return ((dense_font *)this)->glyph(codepoint, g);
    case ID_dmcp_font:   return ((dmcp_font *)this)->glyph(codepoint, g);
    default:
        record(fonts_error, "Unexpectd font type %d", type());
    }
    return false;
}


font::fuint sparse_font::height()
// ----------------------------------------------------------------------------
//   Return the font height from its data
// ----------------------------------------------------------------------------
{
    // Scan the font data
    byte_p        p      = payload();
    size_t UNUSED size   = leb128<size_t>(p);
    fuint         height = leb128<fuint>(p);
    return height;
}


bool sparse_font::glyph(unicode codepoint, glyph_info &g) const
// ----------------------------------------------------------------------------
//   Return the bitmap address and update coordinate info for a sparse font
// ----------------------------------------------------------------------------
{
    // Scan the font data
    byte_p            p      = payload();
    size_t UNUSED     size   = leb128<size_t>(p);
    fuint             height = leb128<fuint>(p);
    bool              fixed  = (codepoint <= '9' && codepoint > '0' &&
                                Settings.FixedWidthDigits());
    fuint             digitWidth = 0;

    // Check if cached
    font_cache::data *data = FontCache.lookup(this, codepoint);

    record(sparse_fonts, "Looking up %u, got cache %p", codepoint, data);
    while (!data)
    {
        // Check code point range
        fuint firstCP = leb128<fuint>(p);
        fuint numCPs  = leb128<fuint>(p);
        record(sparse_fonts,
               "  Range %u-%u (%u codepoints)",
               firstCP, firstCP + numCPs, numCPs);

        // Check end of font ranges, or if past current codepoint
        if ((!firstCP && !numCPs) || firstCP > codepoint)
        {
            record(sparse_fonts, "Code point %u not found", codepoint);
            return false;
        }

        // Initialize cache for range of current code point
        fuint lastCP = firstCP + numCPs;
        for (fuint cp = firstCP; cp < lastCP; cp++)
        {
            fint  x = leb128<fint>(p);
            fint  y = leb128<fint>(p);
            fuint w = leb128<fuint>(p);
            fuint h = leb128<fuint>(p);
            fuint a = leb128<fuint>(p);
            if (fixed && cp >= '1' && cp <= '9')
                a = digitWidth;
            if (cp == codepoint)
                data = FontCache.insert(this, codepoint, p, x, y, w, h, a);
            else if (fixed && cp == '0')
                digitWidth = a;

            size_t sparseBitmapBits = w * h;
            size_t sparseBitmapBytes = (sparseBitmapBits + 7) / 8;
            p += sparseBitmapBytes;

            // This write is not useless (workaround for bug #304)
            memset(&g, (byte) (intptr_t) p, sizeof(g));

            record(sparse_fonts,
                   "  cp %u x=%u y=%u w=%u h=%u bitmap=%p %u bytes",
                   cp, x, y, w, h, p - sparseBitmapBytes, sparseBitmapBytes);
        }
    }

    g.bitmap  = data->bitmap;
    g.bx      = 0;
    g.by      = 0;
    g.bw      = data->w;
    g.bh      = data->h;
    g.x       = data->x;
    g.y       = data->y;
    g.w       = data->w;
    g.h       = data->h;
    g.advance = data->advance;
    g.height  = height;
    record(sparse_fonts,
           "For glyph %u, x=%u y=%u w=%u h=%u bw=%u bh=%u adv=%u hgh=%u",
           codepoint, g.x, g.y, g.w, g.h, g.bw, g.bh, g.advance, g.height);
    return true;
}


font::fuint dense_font::height()
// ----------------------------------------------------------------------------
//   Return the font height from its data
// ----------------------------------------------------------------------------
{
    // Scan the font data
    byte_p        p      = payload();
    size_t UNUSED size   = leb128<size_t>(p);
    fuint         height = leb128<fuint>(p);
    return height;
}


bool dense_font::glyph(unicode codepoint, glyph_info &g) const
// ----------------------------------------------------------------------------
//   Return the bitmap address and update coordinate info for a dense font
// ----------------------------------------------------------------------------
{
    // Scan the font data
    byte_p            p          = payload();
    size_t UNUSED     size       = leb128<size_t>(p);
    fuint             height     = leb128<fuint>(p);
    fuint             width      = leb128<fuint>(p);
    byte_p            bitmap     = p;
    bool              fixed  = (codepoint <= '9' && codepoint > '0' &&
                                Settings.FixedWidthDigits());
    fuint             digitWidth = 0;

    // Check if cached
    font_cache::data *data = FontCache.lookup(this, codepoint);

    // Scan the font data
    fint   x          = 0;
    size_t bitmapSize = (height * width + 7) / 8;
    p += bitmapSize;
    while (!data)
    {
        // Check code point range
        fuint firstCP = leb128<fuint>(p);
        fuint numCPs  = leb128<fuint>(p);

        // Check end of font ranges, or if past current codepoint
        if ((!firstCP && !numCPs) || firstCP > codepoint)
        {
            record(dense_fonts, "Code point %u not found", codepoint);
            return false;
        }

        fuint lastCP = firstCP + numCPs;
        for (fuint cp = firstCP; cp < lastCP; cp++)
        {
            fuint cw  = leb128<fuint>(p);
            fuint adv = fixed && cp >= '1' && cp <= '9' ? digitWidth : cw;
            if (cp == codepoint)
                data = FontCache.insert(this, cp,
                                        bitmap, x, 0, cw, height, adv);
            else if (fixed && cp == '0')
                digitWidth = cw;
            x += cw;
        }
    }
    g.bitmap  = bitmap;
    g.bx      = data->x;
    g.by      = data->y;
    g.bw      = width;
    g.bh      = height;
    g.x       = 0;
    g.y       = 0;
    g.w       = data->w;
    g.h       = height;
    g.advance = data->advance;
    g.height  = height;
    return true;
}


font::fuint dmcp_font::height()
// ----------------------------------------------------------------------------
//   Return the font height from its data
// ----------------------------------------------------------------------------
{
    // Switch to the correct DMCP font
    int fontnr = index();
    if (fontnr >= 11 && fontnr <= 16) // Use special DMCP index for Free42 fonts
        fontnr = -(fontnr - 10);
    lcd_switchFont(fReg, fontnr);

    // Check if codepoint is within font range
    const line_font_t *f        = fReg->f;
    return f->height;
}


bool dmcp_font::glyph(unicode utf8cp, glyph_info &g) const
// ----------------------------------------------------------------------------
//   Return the bitmap address and update coordinate info for a DMCP font
// ----------------------------------------------------------------------------
//   On the DM42, DMCP font numbering is a bit wild.
//   There are three font sets, with lcd_nextFontNr and lcd_prevFontNr switching
//   only within a given set, and lcd_toggleFontT switching between sets
//   The lib_mono set has 6 font sizes with the following numbers:
//      0: lib_mono_10x17
//      1: lib_mono_11x18
//      2: lib_mono_12x20
//      3: lib_mono_14x22
//      4: lib_mono_17x25
//      5: lib_mono_17x28
//   lib_mono fonts are the only fonts the simulator has at the moment.
//   The free42 family contains four "HP-style" fonts:
//     -1: free42_2x2   (encoded as dcmp_font::index 11)
//     -3: free42_2x3   (encoded as dmcp_font::index 13)
//     -5: free42_3x3   (encoded as dmcp_font::index 15)
//     -6: free42_3x4   (encoded as dmcp_font::index 16)
//   The skr_mono family has two "bold" fonts with larger size:
//     18: skr_mono_13x18
//     21: skr_mono_18x24
//   The lcd_toggleFontT can sometimes return 16, but that appears to select the
//   same font as index 18 when passed to lcd_switchFont
//
//   Furthermore, the DMCP fonts are not Unicode-compliant.
//   This function automatically performs the remapping of relevant code points
//   prior to caching (in order to keep correct cache locality)
//
//   Finally, the fonts lack a few important characters for RPL, like the
//   program brackets « and ». Those are "synthesized" by this routine.
//
//   The DMCP font does not spoil the cache, keeping it for the other two types.
//   That's because it has a single range and a direct access already.
{
    // Map Unicode code points to corresonding entry in DMCP charset
    unicode codepoint = utf8cp;
    switch(codepoint)
    {
    case L'÷': codepoint = 0x80; break;
    case L'×': codepoint = 0x81; break;
    case L'√': codepoint = 0x82; break;
    case L'∫': codepoint = 0x83; break;
    case L'░': codepoint = 0x84; break;
    case L'Σ': codepoint = 0x85; break;
        // case L'▶': codepoint = 0x86; break;
    case L'π': codepoint = 0x87; break;
    case L'¿': codepoint = 0x88; break;
    case L'≤': codepoint = 0x89; break;
    case L'␊': codepoint = 0x8A; break;
    case L'≥': codepoint = 0x8B; break;
    case L'≠': codepoint = 0x8C; break;
    case L'↲': codepoint = 0x8D; break;
    case L'↓': codepoint = 0x8E; break;
    case L'→': codepoint = 0x8F; break;
    case L'←': codepoint = 0x90; break;
    case L'μ': codepoint = 0x91; break;
    case L'£': codepoint = 0x92; break;
    case L'°': codepoint = 0x93; break;
    case L'Å': codepoint = 0x94; break;
    case L'Ñ': codepoint = 0x95; break;
    case L'Ä': codepoint = 0x96; break;
    case L'∡': codepoint = 0x97; break;
    case L'ᴇ': codepoint = 0x98; break;
    case L'Æ': codepoint = 0x99; break;
    case L'…': codepoint = 0x9A; break;
    case L'␛': codepoint = 0x9B; break;
    case L'Ö': codepoint = 0x9C; break;
    case L'Ü': codepoint = 0x9D; break;
    case L'▒': codepoint = 0x9E; break;
    case L'■': codepoint = 0x9F; break;
    case L'▼': codepoint = 0xA0; break;
    case L'▲': codepoint = 0xA1; break;
    default:
        break;
    }

    // Switch to the correct DMCP font
    int fontnr = index();
    if (fontnr >= 11 && fontnr <= 16) // Use special DMCP index for Free42 fonts
        fontnr = -(fontnr - 10);
    lcd_switchFont(fReg, fontnr);

    // Check if codepoint is within font range
    const line_font_t *f        = fReg->f;
    uint               first    = f->first_char;
    uint               count    = f->char_cnt;
    uint               last     = first + count;
    if (codepoint < first || codepoint >= last)
    {
        font_p alternate = HelpFont;
        switch (fontnr)
        {
        case 2:
        case 3:
        case 4:
        case 5:
            alternate = StackFont;
            break;
        case 18:
        case 21:
            alternate = HelpFont;
            break;
        case 24:
            alternate = StackFont;
            break;
        }
        record(dmcp_fonts, "Code point %u not found (utf8 %u), using alternate",
               codepoint, utf8cp);
        return alternate->glyph(codepoint, g);
    }

    // Get font and glyph properties
    uint           height = f->height;
    const byte    *data   = f->data;
    fint           off    = f->offs[codepoint - first];
    const uint8_t *dp     = data + off;
    fint           cx     = *dp++;
    fint           cy     = *dp++;
    fint           cols   = *dp++;
    fint           rows   = *dp++;

    g.bitmap = dp;
    g.bx = 0;
    g.by = 0;
    g.bw = (cols + 7) / 8 * 8;
    g.bh = rows;
    g.x = cx;
    g.y = cy;
    g.w = cols;
    g.h = rows;
    g.advance = cx + cols;
    g.height = height;

    return true;
}
