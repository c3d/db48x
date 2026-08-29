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
#include "text.h"
#include "utf8.h"

#include <unistd.h>
#include <string.h>



RECORDER(file,          16, "File operations");
RECORDER(file_error,    16, "File errors");


// The one and only open file in DMCP...
file *file::current = nullptr;


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
    : data(), name(), closed(), previous(nullptr), writing(), xlate()
{}


file::file(cstring path, mode wrmode)
// ----------------------------------------------------------------------------
//   Construct a file object for writing
// ----------------------------------------------------------------------------
    : file()
{
    open(path, wrmode);
}


file::file(text_p name, mode wrmode)
// ----------------------------------------------------------------------------
//   Open a file from a text value
// ----------------------------------------------------------------------------
    : file()
{
    if (name)
    {
        static char buf[80];
        size_t len  = 0;
        utf8   path = name->value(&len);
        if (len < sizeof(buf))
        {
            memcpy(buf, path, len);
            buf[len] = 0;
            open(buf, wrmode);
        }
        else
        {
            rt.file_name_too_long_error();
        }
    }
}


file::~file()
// ----------------------------------------------------------------------------
//   Close the help file
// ----------------------------------------------------------------------------
{
    close();
}


void file::open(cstring path, mode wrmode)
// ----------------------------------------------------------------------------
//    Open a file for reading
// ----------------------------------------------------------------------------
{
    bool reading = wrmode == READING || wrmode == READING_BINARY;
    bool append  = wrmode == APPEND;
    writing      = append || wrmode == WRITING;
    previous     = current;
    if (previous)
        previous->close(false);
    current = this;
    name = path;

#if SIMULATOR
    if (wrmode == READING_BINARY)
        data = fopen(path, "rb");
    else
        data = fopen(path, reading ? "r" : append ? "a" : "w");
    if (!data)
    {
        record(file_error, "Error %s opening %s", strerror(errno), path);
        current = nullptr;
    }
#else // !SIMULATOR
    if (writing)
        sys_disk_write_enable(1);

    BYTE    mode = (reading  ? FA_READ
                    : append ? (FA_WRITE | FA_OPEN_APPEND)
                             : (FA_WRITE | FA_CREATE_ALWAYS));
    FRESULT ok   = f_open(&data, path, mode);
    data.err = ok;
    if (ok != FR_OK)
    {
        data.flag = 0;
        sys_disk_write_enable(0);
        current = nullptr;
    }
#endif // SIMULATOR
}


void file::reopen()
// ----------------------------------------------------------------------------
//   Reopen file from saved data
// ----------------------------------------------------------------------------
{
    open(name, writing ? APPEND : READING);
    if (valid() && !writing)
        seek(closed);
}


void file::close(bool reopen)
// ----------------------------------------------------------------------------
//    Close the help file
// ----------------------------------------------------------------------------
{
    if (valid())
    {
        closed = ftell(data);
        fclose(data);
#if SIMULATOR
        data = nullptr;
#else
        sys_disk_write_enable(0);
        data.flag = 0;
#endif // SIMULATOR
    }
    current = nullptr;

    if (previous && reopen)
    {
        previous->reopen();
        previous = nullptr;
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


bool file::read(char *buf, size_t len)
// ----------------------------------------------------------------------------
//   Read data from a file
// ----------------------------------------------------------------------------
{
#if SIMULATOR
    return fread(buf, 1, len, data) == len;
#else
    UINT bw = 0;
    return f_read(&data, buf, len, &bw) == FR_OK && bw == len;
#endif
}


char file::getchar()
// ----------------------------------------------------------------------------
//   Read char code at offset
// ----------------------------------------------------------------------------
{
    int c = 0;
    do
    {
        c = valid() ? fgetc(data) : EOF;
    } while (c == '\r');
    if (c == EOF)
        c = 0;
    return c;
}


unicode file::get()
// ----------------------------------------------------------------------------
//   Read UTF8 code at offset
// ----------------------------------------------------------------------------
{
    unicode code = 0;
    do
    {
        code = valid() ? fgetc(data) : unicode(EOF);
    } while (code == '\r');
    if (code == unicode(EOF))
        return 0;

    if (xlate && (code == '<' || code == '>' || code == '-'))
    {
        uint    offs = ftell(data);
        unicode next = valid() ? fgetc(data) : unicode(EOF);
        if (code == '<' && next == '<')
            return U'«';
        else if (code == '>' && next == '>')
            return U'»';
        else if (code == '-' && next == '>')
            return U'→';
        seek(offs);
    }

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


uint file::find(unicode cp)
// ----------------------------------------------------------------------------
//    Find a given code point in file looking forward
// ----------------------------------------------------------------------------
//    Return position right before code point, position file right after it
{
    unicode c;
    uint    off;
    do
    {
        off = ftell(data);
        c   = get();
    } while (c && c != cp);
    return off;
}


uint file::find(unicode cp1, unicode cp2)
// ----------------------------------------------------------------------------
//    Find a given code point in file looking forward
// ----------------------------------------------------------------------------
//    Return position right before code point, position file right after it
{
    unicode c;
    uint    off;
    bool    in = false;
    do
    {
        off = ftell(data);
        c   = get();
        if (c == cp2)
            in = !in;
    } while (c && (in || c != cp1));
    return off;
}


uint file::rfind(unicode  cp)
// ----------------------------------------------------------------------------
//    Find a given code point in file looking backward
// ----------------------------------------------------------------------------
//    Return position right before code point, position file right after it
{
    uint    off = ftell(data);
    unicode c   = 0;
    do
    {
        if (off == 0)
            break;
        off--;
        fseek(data, off, SEEK_SET);
        int raw = valid() ? fgetc(data) : EOF;
        c = raw == EOF ? 0 : unicode(raw);
    } while (c != cp);

    if (c == '\n' && off)
    {
        fseek(data, off - 1, SEEK_SET);
        if (valid() && fgetc(data) == '\r')
            off--;
    }


    seek(off);
    get();
    return off;
}


uint file::rfind(unicode  cp1, unicode cp2)
// ----------------------------------------------------------------------------
//    Find a given code point in file looking backward
// ----------------------------------------------------------------------------
//    Return position right before code point, position file right after it
{
    uint    off = ftell(data);
    unicode c   = 0;
    bool    in  = false;
    do
    {
        if (off == 0)
            break;
        off--;
        fseek(data, off, SEEK_SET);
        int raw = valid() ? fgetc(data) : EOF;
        c = raw == EOF ? 0 : unicode(raw);
        if (c == '\r')
            continue;
        if (c == cp2)
            in = !in;
    }
    while (c && (in || c != cp1));

    seek(off);
    get();
    return off;
}


cstring file::error(int err) const
// ----------------------------------------------------------------------------
//   Return error from error code
// ----------------------------------------------------------------------------
{
#ifdef SIMULATOR
    return strerror(err);
#else
    switch (err)
    {
    case FR_OK:
        return nullptr;

#define ERROR(name, msg)
#define FRROR(name, msg, sys)   case FR_##sys: return msg; break;
#include "errors.tbl"

    default: break;
    }
    return "Unkown error";
#endif // SIMULATOR
}


cstring file::error() const
// ----------------------------------------------------------------------------
//   Return error from errno or data.err
// ----------------------------------------------------------------------------
{
#ifdef SIMULATOR
    return error(errno);
#else
    return error(data.err);
#endif
}


bool file::unlink(text_p name)
// ----------------------------------------------------------------------------
//   Purge (unlink) a file
// ----------------------------------------------------------------------------
{
    char   buf[80];
    size_t len  = 0;
    utf8   path = name->value(&len);
    if (len < sizeof(buf))
    {
        memcpy(buf, path, len);
        buf[len] = 0;
        return unlink(buf);
    }
    rt.file_name_too_long_error();
    return false;
}


bool file::unlink(cstring file)
// ----------------------------------------------------------------------------
//   Purge (unlink) a file
// ----------------------------------------------------------------------------
{
#ifdef SIMULATOR
    return ::unlink(file) == 0;
#else // !SIMULATOR
    return f_unlink(file) == FR_OK;
#endif // SIMULATOR
}


cstring file::extension(cstring path)
// ----------------------------------------------------------------------------
//   Extract the extension of the given path
// ----------------------------------------------------------------------------
{
    cstring ext = nullptr;
    if (path)
    {
        for (cstring p = path; *p; p++)
        {
            if (*p == '\\' || *p == '/')
                ext = nullptr;
            else if (*p == '.')
                ext = p;
        }
    }
    return ext;
}


cstring file::basename(cstring path)
// ----------------------------------------------------------------------------
//   Extract the basename for the given path
// ----------------------------------------------------------------------------
{
    for (cstring p = path; *p; p++)
        if (*p == '/' || *p == '\\')
            path = p + 1;
    return path;
}
