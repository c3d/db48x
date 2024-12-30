#ifndef FILE_H
#  define FILE_H
// ****************************************************************************
//  file.h                                                        DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Abstract the DMCP zany filesystem interface
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

#include "dmcp.h"
#include "types.h"

#include <stdio.h>


// For the text pointer variant of the constructor
typedef const struct text *text_p;


struct file
// ----------------------------------------------------------------------------
//   Direct access to daa files
// ----------------------------------------------------------------------------
//   This class deals with a linked list of files, because DMCP has a really
//   annoying limit where only one file can be open at a time.
{
    enum mode { READING, WRITING, APPEND };
    file();
    file(cstring path, mode wrmode);
    file(text_p path, mode wrmode);
    ~file();

    void    open(cstring path, mode wrmode);
    void    close(bool reopen = true);
    void    reopen();

    bool    valid();
    bool    eof();
    bool    put(unicode out);
    bool    put(char c);
    bool    write(const char *buf, size_t len);
    bool    read(char *buf, size_t len);
    unicode get();
    unicode get(uint offset);
    char    getchar();
    void    seek(uint offset);
    unicode peek();
    uint    position();
    uint    find(unicode cp);
    uint    find(unicode cp1, unicode cp2);
    uint    rfind(unicode cp);
    uint    rfind(unicode cp1, unicode cp2);
    cstring error(int err) const;
    cstring error() const;
    cstring filename() const { return name; }

    static bool    unlink(text_p path);
    static bool    unlink(cstring path);
    static cstring extension(cstring path);
    static cstring basename(cstring path);

protected:
    static file *current;       // Only one open file at a time
#if SIMULATOR
    typedef FILE *FIL;
#endif // SIMULATOR
    FIL         data;
    cstring     name;           // File name to use when reopening
    uint        closed;         // Position in file when closing
    file *      previous;       // Previous file to reopen when closing
    bool        writing;        // Should we reopen for writing
};


#define MAGIC_SAVE_STATE         0x05121968


// ============================================================================
//
//   DMCP wrappers
//
// ============================================================================

#ifndef SIMULATOR
#define ftell(f)     f_tell(&f)
#define fseek(f,o,w) f_lseek(&f,o)
#define fclose(f)    f_close(&f)
#define feof(f)      f_eof(&f)
#endif // SIMULATOR



// ============================================================================
//
//    Inline functions for simple stuff
//
// ============================================================================

inline bool file::valid()
// ----------------------------------------------------------------------------
//    Return true if the input file is OK
// ----------------------------------------------------------------------------
{
#if SIMULATOR
    return data          != 0;
#else
    return data.flag && !data.err;
#endif
}


inline void file::seek(uint off)
// ----------------------------------------------------------------------------
//    Move the read position in the data file
// ----------------------------------------------------------------------------
{
    fseek(data, off, SEEK_SET);
}


inline unicode file::peek()
// ----------------------------------------------------------------------------
//    Look at what is as current position without moving it
// ----------------------------------------------------------------------------
{
    uint off       = ftell(data);
    unicode result = get();
    seek(off);
    return result;
}


inline unicode file::get(uint off)
// ----------------------------------------------------------------------------
//    Get code point at given offset
// ----------------------------------------------------------------------------
{
    seek(off);
    return get();
}


inline uint file::position()
// ----------------------------------------------------------------------------
//   Return current position in help file
// ----------------------------------------------------------------------------
{
    return ftell(data);
}


inline bool file::eof()
// ----------------------------------------------------------------------------
//   Indicate if end of file
// ----------------------------------------------------------------------------
{
    return feof(data);
}

#endif // FILE_H
