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

#include "version.h"
#include "SEGGER_RTT.h"


inline void RTT_vprintf_cr_time( const char * sFormat, ...);

RECORDER(file,          16, "File operations");
RECORDER(file_error,    16, "File errors");


// The one and only open file in DMCP...
file *file::current = nullptr;
file* file::open_files[MAX_FILE] = { nullptr };

// ============================================================================
//
//   DMCP wrappers
//
// ============================================================================



#if USE_EmFile

int file::allocate_slot() {
    for (int i = 0; i < MAX_FILE; i++)
        if (!open_files[i])
            return i;
    return -1;
}

static inline int fgetc_(FS_FILE *f)
// ----------------------------------------------------------------------------
//   Read one character from a file - Wrapper for DMCP filesystem
// ----------------------------------------------------------------------------
{
    UINT br                     = 0;
   unsigned char c;
 if (f == NULL) {
        return EOF;
    }
    
    if (FS_FRead(&c, 1, 1, f) != 1) {
        return EOF;
    }
    
    return (int)c;
}


long ftell_(FS_FILE *pFile) {
    
    
    if (pFile == NULL) {
        return -1L;
    }
    
    return FS_FTell(pFile);
}

int fseek_(FS_FILE *pFile, long offset, int whence) {
   
    
    if (pFile == NULL) {
        return -1;
    }
    
    int fs_whence;
    switch (whence) {
        case SEEK_SET:
            fs_whence = FS_FILE_BEGIN;
            break;
        case SEEK_CUR:
            fs_whence = FS_FILE_CURRENT;
            break;
        case SEEK_END:
            fs_whence = FS_FILE_END;
            break;
        default:
            return -1;
    }
    
    return FS_FSeek(pFile, offset, fs_whence);
}

#endif                          // SIMULATOR




file::file()
// ----------------------------------------------------------------------------
//   Construct a file object
// ----------------------------------------------------------------------------
    : data(), name(), closed(), previous(nullptr), slot(-1)
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
    bool reading = wrmode == READING;
    bool append  = wrmode == APPEND;
    writing      = append || wrmode == WRITING;
/*
    previous     = current;
    if (previous)
        previous->close(false);
    current = this;
*/
slot = allocate_slot();
if (slot < 0) {
   RTT_vprintf_cr_time( "Too many open files (max %d)", MAX_FILE);
    rt.too_many_open_files_error();
    return;
}
open_files[slot] = this;

    name = path;

#if (SIMULATOR & ! USE_EmFile)
    data = fopen(path, reading ? "r" : append ? "a" : "w");
    if (!data)
    {
        record(file_error, "Error %s opening %s", strerror(errno), path);
        current = nullptr;
    }
#elif USE_EmFile
   char  n_name[256]={0};
   uint32_t ii =0;

// ctring = const char *
    for (cstring p = path; *p; p++){
      if (*p == '/' ) {n_name[ii] = '\\';}
      else        n_name[ii] = *p;
      ii++;
   }
   int err = FS_FOpenEx(n_name, reading ? "r" : append ? "a" : "w+", &data);
   RTT_vprintf_cr_time( "open : %s => %s, slot %d", n_name,  err ? FS_ErrorNo2Text(err): "ok", slot);
   if (err){
      open_files[slot]=nullptr;
      if (FS_ERRCODE_FILE_IS_OPEN == err)
         rt.file_access_denied_error();
      else
         rt.file_not_found_error();
   }
   f_eof = false;
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
#if (SIMULATOR & ! USE_EmFile)
        closed = ftell(data);
        fclose(data);
        data = nullptr;
#elif USE_EmFile

      closed = FS_FTell(data);
      FS_FClose(data);
      data = nullptr;
      RTT_vprintf_cr_time( "close : %s, slot %d ", name, slot);

#else
        closed = ftell(data);
        fclose(data);
        sys_disk_write_enable(0);
        data.flag = 0;
#endif // SIMULATOR
    }

/*
    current = nullptr;

    if (previous && reopen)
    {
        previous->reopen();
        previous = nullptr;
    }
*/

// Remove from open table
   if (slot >= 0 && slot < MAX_FILE) {
      open_files[slot] = nullptr;
   }
   slot = -1;
}


bool file::put(unicode cp)
// ----------------------------------------------------------------------------
//   Emit a unicode character in the file
// ----------------------------------------------------------------------------
{
    byte   buffer[4];
    size_t count = utf8_encode(cp, buffer);

#if (SIMULATOR & ! USE_EmFile)
    return fwrite(buffer, 1, count, data) == count;

#elif  USE_EmFile
    f_eof =  FS_FWrite(buffer, 1, count, data) != count;
    return !f_eof;

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
#if (SIMULATOR & ! USE_EmFile)
    return fwrite(&c, 1, 1, data) == 1;
#elif  USE_EmFile
    f_eof =  FS_FWrite(&c, 1, 1, data) != 1;
    return !f_eof;

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
#if (SIMULATOR & ! USE_EmFile)
    return fwrite(buf, 1, len, data) == len;
#elif  USE_EmFile
    f_eof =  FS_FWrite(buf, 1, len, data) != len;
    return !f_eof;
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
#if (SIMULATOR & ! USE_EmFile)
    return fread(buf, 1, len, data) == len;
#elif  USE_EmFile

    f_eof =  FS_FRead(buf, 1, len, data) != len;
    return !f_eof;
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
#if USE_EmFile
   unsigned char c;
    if (data == NULL) {
      f_eof = true;
      c = 0;
        return c;
    }
    if (FS_FRead(&c, 1, 1, data) != 1) {
      f_eof = true;
      c = 0;
        return c;
    }    
   f_eof = false;
   return (int)c;
#else
    int c = valid() ? fgetc(data) : 0;
    if (c == EOF)
        c = 0;
    return c;
#endif
}


unicode file::get()
// ----------------------------------------------------------------------------
//   Read UTF8 code at offset
// ----------------------------------------------------------------------------
{
    unicode code = valid() ? fgetc_(data) : unicode(EOF);
    if (code == unicode(EOF)){
      f_eof = true;
       return 0;
   }
    if (code & 0x80)
    {
        // Reference: Wikipedia UTF-8 description
        if ((code & 0xE0)      == 0xC0)
            code = ((code & 0x1F)        <<  6)
                |  (fgetc_(data) & 0x3F);
        else if ((code & 0xF0) == 0xE0)
            code = ((code & 0xF)         << 12)
                |  ((fgetc_(data) & 0x3F) <<  6)
                |   (fgetc_(data) & 0x3F);
        else if ((code & 0xF8) == 0xF0)
            code = ((code & 0xF)         << 18)
                |  ((fgetc_(data) & 0x3F) << 12)
                |  ((fgetc_(data) & 0x3F) << 6)
                |   (fgetc_(data) & 0x3F);
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
        off = ftell_(data);
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
        off = ftell_(data);
        c   = get();
    } while (c && c != cp1 && (c != cp2 || (in = !in)));
    return off;
}


uint file::rfind(unicode  cp)
// ----------------------------------------------------------------------------
//    Find a given code point in file looking backward
// ----------------------------------------------------------------------------
//    Return position right before code point, position file right after it
{
    uint    off = ftell_(data);
    unicode c;
    do
    {
        if (off == 0)
            break;
        fseek_(data, --off, SEEK_SET);
        c = get();
    }
    while (c != cp);
    return off;
}


uint file::rfind(unicode  cp1, unicode cp2)
// ----------------------------------------------------------------------------
//    Find a given code point in file looking backward
// ----------------------------------------------------------------------------
//    Return position right before code point, position file right after it
{
    uint    off = ftell_(data);
    unicode c;
    bool    in = false;
    do
    {
        if (off == 0)
            break;
        fseek_(data, --off, SEEK_SET);
        c = get();
    }
    while (c != cp1 && (c != cp2 || (in = !in)));
    return off;
}


cstring file::error(int err) const
// ----------------------------------------------------------------------------
//   Return error from error code
// ----------------------------------------------------------------------------
{
#ifdef SIMULATOR
    return strerror(err);
#elif  USE_EmFile
   return FS_ErrorNo2Text(err);
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

#elif  USE_EmFile
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
//#endif
}


bool file::unlink(cstring file)
// ----------------------------------------------------------------------------
//   Purge (unlink) a file
// ----------------------------------------------------------------------------
{
#if (SIMULATOR & ! USE_EmFile)
    return ::unlink(file) == 0;
#elif  USE_EmFile
  return FS_Remove(file) == FS_ERRCODE_OK;
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


