#ifndef TYPES_H
#define TYPES_H
// ****************************************************************************
//  types.h                                                       DB48X project
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

#include <cstdint>
#include <cstddef>


// ============================================================================
//
//    Basic data types
//
// ============================================================================

typedef unsigned           uint;
typedef uint8_t            byte;
typedef const byte        *byte_p;
typedef uint64_t           ularge;
typedef int64_t            large;
typedef const char        *cstring;
typedef const byte        *utf8;
typedef unsigned           unicode;

// Indicate that an argument may be unused
#define UNUSED          __attribute__((unused))

#define COMPILE_TIME_ASSERT(x)   extern int CompileTimeAssert(int[!!(x)-1])

#define INLINE  __attribute__((always_inline))
#define NOINLINE  __attribute__((noinline))

template <typename value_type>
struct save
// ----------------------------------------------------------------------------
//   Save a value and reset it to what it was on scope exit
// ----------------------------------------------------------------------------
{
    save(value_type &ref, value_type value): ref(ref), saved(ref)
    {
        ref = value;
    }
    ~save()
    {
        ref = saved;
    }
    value_type  &ref;
    value_type  saved;
};

extern void debug_printf(int row, cstring format, ...);
extern void debug_printf(cstring format, ...);
extern void debug_wait(int delay);

#endif // TYPES_H
