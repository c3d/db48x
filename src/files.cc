// ****************************************************************************
//  files.cc                                                      DB48X project
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
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

#include "files.h"

#include "array.h"
#include "dmcp.h"
#include "file.h"
#include "grob.h"
#include "list.h"
#include "program.h"
#include "runtime.h"
#include "settings.h"


// ============================================================================
//
//   Storing an object to disk
//
// ============================================================================

bool files::store(text_p name, object_p value, cstring defext) const
// ----------------------------------------------------------------------------
//   Check if the name ends in 'B' for binary, otherwise use source format
// ----------------------------------------------------------------------------
{
    files_g fs = this;

    size_t len  = 0;
    utf8   path = name->value(&len);
    if (!len || !path)
    {
        rt.invalid_file_name_error();
        return false;
    }

    // Select format based on extension
    size_t extpos = len;
    while (extpos > 0 && path[extpos-1] != '.')
        extpos--;
    if (!extpos)
    {
        text_g nmg = text_g(name)
            + text_g(text::make(".")) + text_g(text::make(defext));
        return fs->store(nmg, value, "48s");
    }

    // Check how to save
    cstring ext = cstring(path + extpos);

    // Save as binary?
    if (strncasecmp(ext, "48b", 3) == 0)
        return fs->store_binary(name, value);

    // Save as text?
    if (strncasecmp(ext, "txt", 3) == 0)
    {
        if (text_p txt = value->as<text>())
            return fs->store_text(name, txt);
        return fs->store_text(name, value->as_text(true));
    }

    // Save as comma-separated value
    if (strncasecmp(ext, "csv", 3) == 0)
    {
        id ty = value->type();
        if (is_array_or_list(ty))
            return fs->store_list(name, list_p(value));
    }

    // Save as BMP
    if (strncasecmp(ext, "bmp", 3) == 0)
    {
        if (value->is_graph())
            return fs->store_grob(name, grob_p(value));
        text_g fname = name;
        if (grob_p grob = value->graph())
            return fs->store_grob(fname, grob);
    }

    // Default to saving as source
    return fs->store_source(name, value);
}


// The checksum we use for all commands
static uint32_t computed_checksum = 0;
static byte     file_magic[]      = FILE_MAGIC;


static uint32_t id_checksum()
// ----------------------------------------------------------------------------
//   A checksum of all ID names, used to identify changes in binary format
// ----------------------------------------------------------------------------
{
    if (computed_checksum == 0)
    {
        for (uint i = 0; i < object::NUM_IDS; i++)
            for (utf8 p = object::fancy(object::id(i)); *p; p++)
                computed_checksum = 0x1081 * computed_checksum ^ *p;
    }
    return computed_checksum;
}


bool files::store_binary(text_p name, object_p value) const
// ----------------------------------------------------------------------------
//  Store object in binary format
// ----------------------------------------------------------------------------
{
    if (value)
    {
        file f(filename(name, true), true);
        if (f.valid())
        {
            uint32_t checksum = id_checksum();
            if (f.write(cstring(file_magic), sizeof(file_magic))    &&
                f.write(cstring(&checksum), sizeof(checksum))       &&
                f.write(cstring(value), value->size()))
                return true;
        }
        rt.error(f.error());
    }
    return false;
}


bool files::store_source(text_p name, object_p value) const
// ----------------------------------------------------------------------------
//   Store object in source format
// ----------------------------------------------------------------------------
{
    if (value)
    {
        file f(filename(name, true), true);
        if (f.valid())
        {
            renderer r(f);
            value->render(r);
            return true;
        }
        rt.error(f.error());
    }
    return false;
}


bool files::store_text(text_p name, text_p value) const
// ----------------------------------------------------------------------------
//   Stores a text value directly
// ----------------------------------------------------------------------------
{
    if (value)
    {
        file f(filename(name, true), true);
        if (f.valid())
        {
            size_t len = 0;
            utf8   txt    = value->value(&len);
            if (f.write(cstring(txt), len))
                return true;
        }
        rt.error(f.error());
    }
    return false;
}


bool files::store_list(text_p name, list_p value) const
// ----------------------------------------------------------------------------
//   Store a list or array in CSV format, using ';' as separator
// ----------------------------------------------------------------------------
{
    if (value)
    {
        file f(filename(name, true), true);
        if (f.valid())
        {
            renderer r(f);
            bool ok = true;
            for (object_p row : *value)
            {
                if (list_p li = row->as_array_or_list())
                {
                    bool first = true;
                    for (object_p col : *li)
                    {
                        if (!first)
                            ok = f.write(";", 1);
                        if (ok)
                            col->render(r);
                        first = false;
                        if (!ok)
                            break;
                    }
                    if (ok)
                        ok = f.write("\n", 1);
                }
                else
                {
                    row->render(r);
                    if (ok)
                        ok = f.write("\n", 1);
                }
                if (!ok)
                    break;
            }
            if (ok)
                return true;
        }
        rt.error(f.error());
    }
    return false;

}


object_p files::recall(text_p name, cstring defext) const
// ----------------------------------------------------------------------------
//    Recall an object from disk
// ----------------------------------------------------------------------------
{
    files_g fs = this;

    size_t len  = 0;
    utf8   path = name->value(&len);
    if (!len || !path)
    {
        rt.invalid_file_name_error();
        return nullptr;
    }

    // Select format based on extension
    size_t extpos = len;
    while (extpos > 0 && path[extpos-1] != '.')
        extpos--;
    if (!extpos)
    {
        text_g nmg = text_g(name)
            + text_g(text::make(".")) + text_g(text::make(defext));
        return fs->recall(nmg, "48s");
    }

    // Check the format of the file
    cstring ext = cstring(path + extpos);

    // Load from binary?
    if (strncasecmp(ext, "48b", 3) == 0)
        return fs->recall_binary(name);

    // Load from text?
    if (strncasecmp(ext, "txt", 3) == 0)
        return fs->recall_text(name);

    // Load from comma-separated value
    if (strncasecmp(ext, "csv", 3) == 0)
        return fs->recall_list(name, true);

    // Load from BMP
    if (strncasecmp(ext, "bmp", 3) == 0)
        return fs->recall_grob(name);

    // Default to loading from source
    return fs->recall_source(name);
}


object_p files::recall_binary(text_p name) const
// ----------------------------------------------------------------------------
//   Recall an object from binary file
// ----------------------------------------------------------------------------
{
    if (rt.allocated())
    {
        rt.unable_to_allocate_error();
        return nullptr;
    }

    file f(filename(name), false);
    if (f.valid())
    {
        scribble scr;
        uint32_t checksum = id_checksum();
        char     buf[sizeof(file_magic)];
        uint32_t check = 0;
        char     c;
        size_t   sz;
        object_p result;
        if (!f.read(buf, sizeof(file_magic)))
            goto err;
        if (memcmp(buf, file_magic, sizeof(file_magic)) != 0)
        {
            rt.invalid_magic_number_error();
            return nullptr;
        }
        if (!f.read((char *) &check, sizeof(checksum)))
            goto err;
        if (check != checksum)
        {
            rt.incompatible_binary_error();
            return nullptr;
        }

        while (true)
        {
            if (!f.read(&c, 1))
                break;
            byte *ptr = rt.allocate(1);
            *ptr = c;
        }

        sz = rt.allocated();
        result = rt.temporary();
        if (result->type() >= NUM_IDS || result->size() != sz)
        {
            rt.invalid_object_in_file_error();
            return nullptr;
        }

        return result;

    err:
        rt.error(f.error());
    }
    return nullptr;
}


object_p files::recall_source(text_p name) const
// ----------------------------------------------------------------------------
//   Recall an object from source file
// ----------------------------------------------------------------------------
{
    {
        file prog(filename(name), false);
        if (!prog.valid())
        {
            rt.error(prog.error());
            return nullptr;
        }

        // Loop on the input file and process it as if it was being typed
        uint bytes = 0;
        rt.clear();

        for (unicode c = prog.get(); c; c = prog.get())
        {
            byte buffer[4];
            size_t count = utf8_encode(c, buffer);
            rt.insert(bytes, buffer, count);
            bytes += count;
        }
    }

    // End of file: execute the command we typed
    size_t edlen = rt.editing();
    if (edlen)
    {
        text_g edstr = rt.close_editor(true);
        if (edstr)
        {
            gcutf8 editor = edstr->value();
            bool dc = Settings.DecimalComma();
            Settings.DecimalComma(false);
            object_p obj = object::parse(editor, edlen);
            Settings.DecimalComma(dc);
            return obj;
        }
    }

    rt.invalid_object_error();
    return nullptr;
}


text_p files::recall_text(text_p name) const
// ----------------------------------------------------------------------------
//   Recall text from a text file
// ----------------------------------------------------------------------------
{
    file f(filename(name), false);
    if (!f.valid())
    {
        rt.error(f.error());
        return nullptr;
    }

    // Loop on the input file and process it as if it was being typed
    size_t bytes = 0;
    rt.clear();
    for (unicode c = f.get(); c; c = f.get())
    {
        byte buffer[4];
        size_t count = utf8_encode(c, buffer);
        rt.insert(bytes, buffer, count);
        bytes += count;
    }

    // End of file: execute the command we typed
    return rt.close_editor(true, false);
}


list_p files::recall_list(text_p name, bool as_array) const
// ----------------------------------------------------------------------------
//  Recall list from a CSV file
// ----------------------------------------------------------------------------
{
    file f(filename(name), false);
    if (!f.valid())
    {
        if (!rt.error())
            rt.error(f.error());
        return nullptr;
    }

    id       ty     = as_array ? ID_array : ID_list;
    list_g   result = list::make(ty, nullptr, 0);
    object_g item   = nullptr;
    list_g   row    = nullptr;
    int      cols   = 0;
    int      kcols  = -1;
    bool     intxt  = false;
    bool     ineqn  = false;
    uint     paren  = 0;
    uint     brack  = 0;
    uint     curly  = 0;
    uint     nonsp  = 0;

    // Loop on the input file and process it as if it was being typed
    size_t   bytes  = 0;
    rt.clear();
    for (unicode c = f.get(); c; c = f.get())
    {
        switch(c)
        {
        case '(':       paren++; break;
        case ')':       paren--; break;
        case '[':       brack++; break;
        case ']':       brack--; break;
        case '{':       curly++; break;
        case '}':       curly--; break;
        case '"':       intxt = !intxt; break;
        case '\'':      ineqn = !ineqn; break;
        }
        bool sepok = !paren && !brack && !curly && !intxt && !ineqn;

        if (!isspace(c))
            nonsp++;
        if (sepok && (c == ',' || c == ';' || c == '\n'))
        {
            text_p parsed = rt.close_editor(true);
            size_t len    = 0;
            utf8   txt    = parsed->value(&len);
            if (nonsp)
                item = object::parse(txt, len);
            else
                item = +symbol::make("");;
            nonsp = 0;
            if (!item)
                break;
            byte_p b  = byte_p(+item);
            size_t sz = item->size();
            list_g li = rt.make<list>(ty, b, sz);

            if (row || c == ';' || c == ',')
            {
                row = row ? row + li : li;
                if (c == ';' || c == ',')
                    cols++;
            }
            if (c == '\n')
            {
                // Check if we have a rectangular input
                if (kcols < 0)
                    kcols = cols;
                if (cols != kcols)
                {
                    if (ty != ID_list)
                    {
                        ty = ID_list;
                        list_g copy = list::make(ty, nullptr, 0);
                        for (object_p obj : *result)
                        {
                            list_g ci;
                            if (list_p li = obj->as_array_or_list())
                            {
                                b = byte_p(li->objects(&sz));
                                ci = list::make(b, sz);
                                obj = ci;
                            }
                            b = byte_p(obj);
                            ci = list::make(b, obj->size());
                            copy = copy + ci;
                        }
                        if (row)
                        {
                            b = byte_p(row->objects(&sz));
                            row = list::make(b, sz);
                        }
                        result = copy;
                    }
                }
                if (row)
                {
                    b = byte_p(+row);
                    sz = row->size();
                    li = rt.make<list>(ty, b, sz);
                }
                result = result + li;
                row = nullptr;
                cols = 0;
            }
            rt.clear();
            bytes = 0;
        }
        else
        {
            byte buffer[4];
            size_t count = utf8_encode(c, buffer);
            rt.insert(bytes, buffer, count);
            bytes += count;
        }
    }
    if (row)
        result = result + row;

    return result;
}


bool files::purge(text_p name) const
// ----------------------------------------------------------------------------
//   Purge a file (unlink it)
// ----------------------------------------------------------------------------
{
    text_p path = filename(name);
    return file::unlink(path);
}


static inline size_t find_colon(utf8 txt, size_t len)
// ----------------------------------------------------------------------------
//   Check if there is a colon after letters/digits, e.g. A: or SDCARD:
// ----------------------------------------------------------------------------
{
    utf8 start = txt;
    while (len--)
    {
        char c = *txt++;
        if (c == ':')
            return txt - start;
        if (!isalnum(c))
            break;
    }
    return 0;
}


static inline bool is_path_separator(char c)
// ----------------------------------------------------------------------------
//   Check path separator
// ----------------------------------------------------------------------------
{
    return c == '/' || c == '\\';
}


text_p files::filename(text_p fname, bool writing) const
// ----------------------------------------------------------------------------
//   Build a filename from given input
// ----------------------------------------------------------------------------
// In order to work in the simulator case, absolute paths are treated as
// being relative to current working directory.
// We also accept "pools", like C: or SD:, which are turned to directories
{
    text_g path = this;
    text_g name = fname;

    // If name is an absolute path, use it directly
    size_t len  = 0;
    utf8   txt  = name->value(&len);

    // Check if we have C: or SDCARD:, if so, turn it to a base path
    bool   in_pool = false;
    if (size_t colon = find_colon(txt, len))
    {
        if (colon + 1 < len)
        {
            path = text::make(txt, colon - 1);
            txt += colon;
            len -= colon;
            in_pool = true;
            name = text::make(txt, len);
            txt = name->value(&len);
        }
    }

    // Check if we have an absolute path
    bool absolute = len > 0 && is_path_separator(*txt);
    if (absolute)
    {
        txt++;
        len--;

        // Turn config:/constants.csv into config/constants.csv
        if (in_pool)
            absolute = false;
        name = text::make(txt, len);
    }

    // Check if length of this one is zero or if it's just '/'
    size_t plen = 0;
    utf8   ptxt = path->value(&plen);
    if (plen == 0 || (plen == 1 && is_path_separator(*ptxt)))
        absolute = true;

    // Build the path if necessary
    if (!absolute)
    {
        text_g sep = text::make("/", 1);
        name = path + sep + name;
    }

    // Make sure that we do not evade the sandbox
    uint depth = 0;
    char last = 0;
    txt = name->value(&len);
    for (utf8 p = txt; len--; p++)
    {
        byte c = *p;
        if (c == '.')
        {
            if (last == '.')
            {
                if (!depth)
                {
                    rt.invalid_path_error();
                    return nullptr;
                }
                depth--;
            }
        }
        if (is_path_separator(c))
        {
            if (last != '.' && !is_path_separator(last))
            {
                depth++;

                // If we want to write, make sure all directories exist
                if (writing)
                {
                    // Hack: overwrite '/' with 0 to null-terminate string
                    byte *term = (byte *) p;
                    save<byte> ssep(*term, 0);
                    check_create_dir(cstring(txt));
                }
            }
        }
        last = c;
    }

    return name;
}



// ============================================================================
//
//    BMP file management
//
// ============================================================================

template <typename Int>
struct le
// ----------------------------------------------------------------------------
//   Little endian representation for a type
// ----------------------------------------------------------------------------
{
    static constexpr union bytes
    {
        byte bytes[sizeof(Int)];
        Int  value;
    } byte_order = { .value = Int(0x0706050403020100ULL) };

    le() : value(0)
    {
    }

    le(Int v) : value(swap(v))
    {
    }

    operator Int()
    {
        return swap(value);
    }

    le &operator=(Int x)
    {
        value = swap(x);
        return *this;
    }

    Int swap(Int x)
    {
        bytes b = { .value = x };
        for (uint i = 0; i < sizeof(Int); i++)
            std::swap(b.bytes[i], b.bytes[byte_order.bytes[i]]);
        return b.value;
    }

    Int value;
} PACKED;

// Shortcut for the commonly used types in Windows BMP files
using u32 = le<uint32_t>;
using u16 = le<uint16_t>;
using s32 = le<int32_t>;
using s16 = le<int16_t>;
using bmsize = blitter::size;


struct bmp_core_hdr
// ----------------------------------------------------------------------------
//   Initial bitmap header for Windows 2.0 (12 bytes)
// ----------------------------------------------------------------------------
{
    u32 hdrSize;                // Header size, must be 12
    u16 width;                  // Bitmap width
    u16 height;                 // Bitmap height
    u16 planes;                 // Color planes (must be 1)
    u16 bitsPerPixel;           // Number of bits per pixel
} PACKED;


struct bmp_info_hdr
// ----------------------------------------------------------------------------
//   Windows NT, 3.1 info header, most common
// ----------------------------------------------------------------------------
{
    u32 hdrSize;                // Header size, must be 40
    s32 width;                  // Width of the bitmap
    s32 height;                 // Height of the bitmap
    u16 planes;                 // Color planes (must be 1)
    u16 bitsPerPixel;           // Bits per pixel (1 for DB48X)
    u32 compression;            // Compression method (must be zero for DB48X)
    u32 imageSize;              // Size of image data (DM32 sts 12480, 416x240)
    s32 hResolution;            // Horizontal resolution (2835 pix/m = 72ppp)
    s32 vResolution;            // Vertical resolution (2835 pix/m = 72ppp)
    u32 numColors;              // Number of colors in palette (2 for DB48X)
    u32 impColors;              // Important colors, whatever that means
} PACKED;


struct bmp
// ----------------------------------------------------------------------------
//   Structure of a BMP file header
// ----------------------------------------------------------------------------
{
    char sig[2];                // 'B', 'M'
    u32  size;                  // File size
    char reserved[4];           // Reserved
    u32  pixOffset;             // Offset to pixel array
    union
    {
        u32  hdrSize;           // Header size
        bmp_core_hdr core;      // Old-style
        bmp_info_hdr info;      // More modern style
    } PACKED;
} PACKED;



bool files::store_grob(text_p name, grob_p value) const
// ----------------------------------------------------------------------------
//   Store a graphic object as a BMP
// ----------------------------------------------------------------------------
{
    if (value)
    {
        file f(filename(name, true), true);
        if (f.valid())
        {
            bmsize  width      = 0;
            bmsize  height     = 0;
            size_t  datalen    = 0;
            byte_p  pixels     = value->pixels(&width, &height, &datalen);
            size_t  sstride    = datalen / height;
            size_t  dstride    = (width + 31) / 32 * 4;
            size_t  pixsize    = dstride * height;
            byte    palette[8] = { 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0x00 };
            size_t  fsize      = sizeof(bmp) + sizeof(palette) + pixsize;

            bmp     b
            {
                .sig       = { 'B', 'M' },
                .size      = fsize,
                .reserved  = { 0, 0, 0, 0 },
                .pixOffset = sizeof(bmp),
                .info      =
                {
                    .hdrSize      = sizeof(bmp_info_hdr),
                    .width        = width,
                    .height       = height,
                    .planes       = 1,
                    .bitsPerPixel = 1,
                    .compression  = 0,
                    .imageSize    = pixsize,
                    .hResolution  = 2835,
                    .vResolution  = 2835,
                    .numColors    = 2,
                    .impColors    = 2
                }
            };

            bool ok = true;
            if (ok)
                ok = f.write((const char *) &b, sizeof(b));
            if (ok)
                ok = f.write((const char *) palette, sizeof(palette));

            char zero = 0;
            for (uint r = height; r --> 0 && ok; )
            {
                byte_p scan = pixels + sstride * r;
                char *src = (char *) scan + sstride;
                for (uint c = 0; c < sstride && ok; c++)
                    ok = f.write(--src, 1);
                for (uint c = sstride; c < dstride; c++)
                    ok = f.write(&zero, 1);
            }

            if (ok)
                return true;
        }
        rt.error(f.error());
    }
    return false;
}


grob_p files::recall_grob(text_p name) const
// ----------------------------------------------------------------------------
//  Recall graphic object from a BMP file
// ----------------------------------------------------------------------------
{
    file f(filename(name), false);
    bool ok = f.valid();
    bmp b{};

    if (ok)
        ok = f.read((char *) &b, sizeof(b));
    if (ok)
        ok = (b.sig[0] == 'B' && b.sig[1] == 'M' &&
              b.hdrSize == sizeof(bmp_info_hdr) &&
              b.info.planes == 1 &&
              b.info.bitsPerPixel == 1 &&
              b.info.compression == 0 &&
              b.info.numColors == 2);

    char palette[8] = { 0 };
    if (ok)
        ok = f.read(palette, sizeof(palette));

    if (ok)
    {
        grob_p g = grob::make(b.info.width, b.info.height);
        ok = g != nullptr;

        if (ok)
        {
            bmsize     width   = 0;
            bmsize     height  = 0;
            size_t     datalen = 0;
            byte_p     pixels  = g->pixels(&width, &height, &datalen);
            size_t     dstride = datalen / height;
            size_t     sstride = b.info.imageSize / height;

            ok = int(width) == b.info.width && int(height) == b.info.height;
            char ignore;
            for (uint r = height; r --> 0 && ok; )
            {
                byte *scan = (byte *) pixels + dstride * r;
                char *dst = (char *) scan + dstride;
                for (uint c = 0; c < dstride && ok; c++)
                    ok = f.read(--dst, 1);
                for (uint c = dstride; c < sstride; c++)
                    ok = f.read(&ignore, 1);
            }
        }

        if (ok)
            return g;
    }

    if (!rt.error())
    {
        if (!f.valid())
            rt.error(f.error());
        if (!rt.error())
            rt.invalid_bitmap_file_error();
    }
    return nullptr;
}
