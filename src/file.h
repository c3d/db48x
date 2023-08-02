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


struct file
// ----------------------------------------------------------------------------
//   Direct access to the help file
// ----------------------------------------------------------------------------
{
    file();
    file(cstring path);
    ~file();

    void    open(cstring path);
    void    open_for_writing(cstring path);
    bool    valid();
    void    close();
    bool    put(unicode out);
    bool    put(char c);
    bool    write(const char *buf, size_t len);
    unicode get();
    unicode get(uint offset);
    char    getchar();
    void    seek(uint offset);
    unicode peek();
    uint    position();
    uint    find(unicode cp);
    uint    rfind(unicode cp);

protected:
#if SIMULATOR
    FILE *data;
#else
    FIL     data;
#endif
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

#endif // FILE_H
