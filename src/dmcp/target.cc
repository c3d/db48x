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

#define GPAT      blitter::pattern<blitter::mode::MONOCHROME_REVERSE>
 const GPAT GPAT::black  = GPAT(  0,   0,   0);
 const GPAT GPAT::gray10 = GPAT( 32,  32,  32);
 const GPAT GPAT::gray25 = GPAT( 64,  64,  64);
 const GPAT GPAT::gray50 = GPAT(128, 128, 128);
 const GPAT GPAT::gray75 = GPAT(192, 192, 192);
 const GPAT GPAT::gray90 = GPAT(224, 224, 224);
 const GPAT GPAT::white  = GPAT(255, 255, 255);
#undef GPAT
