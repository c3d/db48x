// ****************************************************************************
//  file.cc                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//      Abstract interface for the zany DMCP filesystem
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

#include "file.h"
#include "ff_ifc.h"
#include "recorder.h"
#include "utf8.h"


RECORDER(file,          16, "File operations");
RECORDER(file_error,    16, "File errors");



// ============================================================================
//
//   DMCP wrappers
//
// ============================================================================

#ifndef SIMULATOR
static inline int fgetc(FIL &f)
// ----------------------------------------------------------------------------
//   Read one character from a file - Wrapper for DMCP filesystem
// ----------------------------------------------------------------------------
{
    UINT br                     = 0;
    char c                      = 0;
    if (f_read(&f, &c, 1, &br) != FR_OK || br != 1)
        return EOF;
    return c;
}


static inline int fputc(int c, FIL &f)
// ----------------------------------------------------------------------------
//   Read one character from a file - Wrapper for DMCP filesystem
// ----------------------------------------------------------------------------
{
    UINT bw = 0;
    if (f_write(&f, &c, 1, &bw) != FR_OK || bw != 1)
        return EOF;
    return c;
}
#endif                          // SIMULATOR



file::file()
// ----------------------------------------------------------------------------
//   Construct a file object
// ----------------------------------------------------------------------------
    : data()
{}


file::file(cstring path)
// ----------------------------------------------------------------------------
//   Construct a file object for writing
// ----------------------------------------------------------------------------
    : data()
{
    open_for_writing(path);
}


file::~file()
// ----------------------------------------------------------------------------
//   Close the help file
// ----------------------------------------------------------------------------
{
    close();
}


void file::open(cstring path)
// ----------------------------------------------------------------------------
//    Open a file for reading
// ----------------------------------------------------------------------------
{
#if SIMULATOR
    data = fopen(path, "r");
    if (!data)
    {
        record(file_error, "Error %s opening %s", strerror(errno), path);
        return;
    }
#else
    FRESULT ok = f_open(&data, path, FA_READ);
    data.err = ok;
    if (ok != FR_OK)
        data.flag = 0;
#endif                          // SIMULATOR
}


void file::open_for_writing(cstring path)
// ----------------------------------------------------------------------------
//    Open a file for writing
// ----------------------------------------------------------------------------
{
#if SIMULATOR
    data = fopen(path, "w");
    if (!data)
    {
        record(file_error, "Error %s opening %s for writing",
               strerror(errno), path);
        return;
    }
#else
    sys_disk_write_enable(1);
    FRESULT ok = f_open(&data, path, FA_WRITE | FA_CREATE_ALWAYS);
    data.err = ok;
    if (ok != FR_OK)
    {
        sys_disk_write_enable(0);
        data.flag = 0;
    }
#endif                          // SIMULATOR
}


void file::close()
// ----------------------------------------------------------------------------
//    Close the help file
// ----------------------------------------------------------------------------
{
    if (valid())
    {
        fclose(data);

#if SIMULATOR
        data = nullptr;
#else
        sys_disk_write_enable(0);
        data.flag = 0;
#endif // SIMULATOR
    }
}


bool file::put(unicode cp)
// ----------------------------------------------------------------------------
//   Emit a unicode character in the file
// ----------------------------------------------------------------------------
{
    byte   buffer[4];
    size_t count = utf8_encode(cp, buffer);

#if SIMULATOR
    return fwrite(buffer, 1, count, data) == count;
#else
    UINT bw = 0;
    return f_write(&data, buffer, count, &bw) == FR_OK && bw == count;
#endif
}


bool file::put(char c)
// ----------------------------------------------------------------------------
//   Emit a single character in the file
// ----------------------------------------------------------------------------
{
#if SIMULATOR
    return fwrite(&c, 1, 1, data) == 1;
#else
    UINT bw = 0;
    return f_write(&data, &c, 1, &bw) == FR_OK && bw == 1;
#endif
}


bool file::write(const char *buf, size_t len)
// ----------------------------------------------------------------------------
//   Emit a buffer to a file
// ----------------------------------------------------------------------------
{
#if SIMULATOR
    return fwrite(buf, 1, len, data) == len;
#else
    UINT bw = 0;
    return f_write(&data, buf, len, &bw) == FR_OK && bw == len;
#endif
}


char file::getchar()
// ----------------------------------------------------------------------------
//   Read char code at offset
// ----------------------------------------------------------------------------
{
    int c = valid() ? fgetc(data) : 0;
    if (c == EOF)
        c = 0;
    return c;
}


unicode file::get()
// ----------------------------------------------------------------------------
//   Read UTF8 code at offset
// ----------------------------------------------------------------------------
{
    unicode code = valid() ? fgetc(data) : unicode(EOF);
    if (code == unicode(EOF))
        return 0;

    if (code & 0x80)
    {
        // Reference: Wikipedia UTF-8 description
        if ((code & 0xE0)      == 0xC0)
            code = ((code & 0x1F)        <<  6)
                |  (fgetc(data) & 0x3F);
        else if ((code & 0xF0) == 0xE0)
            code = ((code & 0xF)         << 12)
                |  ((fgetc(data) & 0x3F) <<  6)
                |   (fgetc(data) & 0x3F);
        else if ((code & 0xF8) == 0xF0)
            code = ((code & 0xF)         << 18)
                |  ((fgetc(data) & 0x3F) << 12)
                |  ((fgetc(data) & 0x3F) << 6)
                |   (fgetc(data) & 0x3F);
    }
    return code;
}


uint file::find(unicode   cp)
// ----------------------------------------------------------------------------
//    Find a given code point in file looking forward
// ----------------------------------------------------------------------------
//    Return position right before code point, position file right after it
{
    unicode c;
    uint    off;
    do
    {
        off          = ftell(data);
        c            = get();
    } while (c && c != cp);
    return off;
}


uint file::rfind(unicode  cp)
// ----------------------------------------------------------------------------
//    Find a given code point in file looking backward
// ----------------------------------------------------------------------------
//    Return position right before code point, position file right after it
{
    uint    off = ftell(data);
    unicode c;
    do
    {
        if (off == 0)
            break;
        fseek(data, --off, SEEK_SET);
        c        = get();
    }
    while (c != cp);
    return off;
}
