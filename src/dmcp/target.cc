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


uint8_t MemToggle_B1=0;
uint8_t MemToggle_B2=0;
uint8_t MemToggle_C2=0;
uint8_t MemToggle_D1=0;
uint8_t MemToggle_D2=0;
uint8_t MemToggle_D3=0;
uint8_t MemToggle_E1=0;
uint8_t MemToggle_E2=0;
uint8_t MemToggle_F1=0;


uint platform_plane(bool ls, bool rs, bool al, bool lc, bool trans)
// ----------------------------------------------------------------------------
//   Create the platform plane for a combination of modifiers
// ----------------------------------------------------------------------------
{
    return ls + 2 * rs + 3 * al + 3 * (lc && al) + 10 * trans;
}


uint platform_keyid(uint pkey, bool ls, bool rs, bool al, bool lc, bool trans)
// ----------------------------------------------------------------------------
//   Create the platform plane for a combination of modifiers
// ----------------------------------------------------------------------------
{
    return pkey + 100 * platform_plane(ls, rs, al, lc, trans);
}

extern  const uint8_t dmcp_position[100] ;

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
