// ****************************************************************************
//  target.cc                                                     DB48X project
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

#include "target.h"

// Initialize the screen
surface Screen((pixword *) lcd_line_addr(0), LCD_W, LCD_H, LCD_SCANLINE);

// Pre-built patterns for shades of grey
const pattern pattern::black   = pattern(0, 0, 0);
const pattern pattern::gray10  = pattern(32, 32, 32);
const pattern pattern::gray25  = pattern(64, 64, 64);
const pattern pattern::gray50  = pattern(128, 128, 128);
const pattern pattern::gray75  = pattern(192, 192, 192);
const pattern pattern::gray90  = pattern(224, 224, 224);
const pattern pattern::white   = pattern(255, 255, 255);
const pattern pattern::invert  = pattern(~0ULL);

static const byte dmcp_position[] =
// ----------------------------------------------------------------------------
//   Convert DMCP key codes to row/column positions
// ----------------------------------------------------------------------------
{
    KB_F1,      11,
    KB_F2,      12,
    KB_F3,      13,
    KB_F4,      14,
    KB_F5,      15,
    KB_F6,      16,

    KB_A,       21,
    KB_B,       22,
    KB_C,       23,
    KB_D,       24,
    KB_E,       25,
    KB_F,       26,

    KB_G,       31,
    KB_H,       32,
    KB_I,       33,
    KB_J,       34,
    KB_K,       35,
    KB_L,       36,

    KB_ENT,     41,
    KB_M,       42,
    KB_N,       43,
    KB_O,       44,
    KB_BKS,     45,

    KB_UP,      51,
    KB_P,       52,
    KB_Q,       53,
    KB_R,       54,
    KB_S,       55,

    KB_DN,      61,
    KB_T,       62,
    KB_U,       63,
    KB_V,       64,
    KB_W,       65,

    KB_SHIFT,   71,
    KB_X,       72,
    KB_Y,       73,
    KB_Z,       74,
    KB_SUB,     75,


    KB_ON,      81,
    KB_0,       82,
    KB_DOT,     83,
    KB_SPC,     84,
    KB_ADD,     85
};



uint platform_plane(bool ls, bool rs, bool al, bool lc, bool trans)
// ----------------------------------------------------------------------------
//   Create the platform plane for a combination of modifiers
// ----------------------------------------------------------------------------
{
    return ls + 2 * rs + 3 * al + 3 * (lc * (1 + !al)) + 10 * trans;
}


uint platform_keyid(uint pkey, bool ls, bool rs, bool al, bool lc, bool trans)
// ----------------------------------------------------------------------------
//   Create the platform plane for a combination of modifiers
// ----------------------------------------------------------------------------
{
    return pkey + 100 * platform_plane(ls, rs, al, lc, trans);
}


uint platform_keyid(uint rc, uint plane)
// ----------------------------------------------------------------------------
//   Return the complete key ID from HP48-style Row/Column/Plane position
// ----------------------------------------------------------------------------
// HP48 key codes are given in the form rc.ph, where r, c, p and h
// are one decimal digit each.
// r = row
// c = column
// p = plane
// h = hold (i.e. shift + key held simultaneously)
//
// On DMCP, we reinterpret "h" as "transient alpha"
//
// The plane is documented as follows:
// 0: Like 1
// 1: Unshifted
// 2: Left shift
// 3: Right shift
// 4: Alpha
// 5: Alpha left shift
// 6: Alpha right shift
//
// DB48X adds 7, 8 and 9 for lowercase alpha
{
    const size_t max = sizeof(dmcp_position) / sizeof(dmcp_position[0]);
    for (size_t k = 0; k < max; k += 2)
    {
        if (dmcp_position[k+1] == rc)
        {
            uint keyid = dmcp_position[k];
            if (plane)
            {
                plane = 100 * (plane/10 - 1) + 1000 * (plane % 10);
                keyid = keyid + plane;
            }
            return keyid;
        }
    }
    return 0;
}



uint compatible_key_position(uint key)
// ----------------------------------------------------------------------------
//   Return the key position for a given key ID
// ----------------------------------------------------------------------------
{
    // Strip the key plane
    key = key % 100;
    const size_t max = sizeof(dmcp_position) / sizeof(dmcp_position[0]);
    for (size_t k = 0; k < max; k += 2)
        if (dmcp_position[k] == key)
            return dmcp_position[k+1];
    return 0;
}


uint compatible_key_plane(uint keyid)
// ----------------------------------------------------------------------------
//   Return the keyplane in the format expected by RPL code
// ----------------------------------------------------------------------------
{
    uint plane = keyid / 100;
    uint h = plane / 10;
    uint p = plane % 10 + 1;
    return p * 10 + h;
}
