#ifndef TARGET_DBU585_H
#define TARGET_DBU585_H
// ****************************************************************************
//  target.h                                                      DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Description of the DM42 platform
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

#include "blitter.h"
#include "dmcp.h"



enum target
// ----------------------------------------------------------------------------
//   Constants for a given target
// ----------------------------------------------------------------------------
{
    LCD_W          = LCD_WIDTH,
    LCD_H          = LCD_HEIGHT,
#ifndef CONFIG_COLOR
    BITS_PER_PIXEL = 1,
    LCD_SCANLINE   = LCD_WIDTH,        // set to LCD_W, ctrl data for sharp lcd are added in the dma buffer
#else
    BITS_PER_PIXEL = 16,
    LCD_SCANLINE   = 400,
#endif

};

// We need to reverse grobs during parsing and rendering
#define REVERSE_GROBS

#ifdef CONFIG_COLOR
using surface = blitter::surface<blitter::mode::RGB_16BPP>;
using color   = blitter::color  <blitter::mode::RGB_16BPP>;
using pattern = blitter::pattern<blitter::mode::RGB_16BPP>;
#else

using surface = blitter::surface<blitter::mode::MONOCHROME_REVERSE>;
using color   = blitter::color  <blitter::mode::MONOCHROME_REVERSE>;
using pattern = blitter::pattern<blitter::mode::MONOCHROME_REVERSE>;

#endif
using coord   = blitter::coord;
using size    = blitter::size;
using rect    = blitter::rect;
using point   = blitter::point;
using pixword = blitter::pixword;

extern surface Screen;


// Soft menu tab size
#define MENU_TAB_SPACE      1
#define MENU_TAB_INSET      2
#define MENU_TAB_WIDTH      ((LCD_W - 5 * MENU_TAB_SPACE) / 6)
#define MENU_TAB_HEIGHT     (FONT_HEIGHT(FONT_MENU) + 2 * MENU_TAB_INSET)

// Put slow-changing data in the QSPI
// FONT_QSPI defined in version.h

/*
    KEYBOARD BIT MAP
    ----------------
    This is the bit number in the 64-bit keymatrix.
    Bit set means key is pressed.
    Note that DMCP does not define keys as bitmaps,
    but rather using keycodes.

            1        2        3        4        5        6
         +--------+--------+--------+--------+--------+--------+
A        |   F1   |   F2   |   F3   |   F4   |   F5   |   F6   |
         |   38   |   39   |   40   |   41   |   42   |   43   |
         +--------+--------+--------+--------+--------+--------+
       S |  Sum-  |  y^x   |  x^2   |  10^x  |  e^x   |  GTO   |
B        |  Sum+  |  1/x   |  Sqrt  |  Log   |  Ln    |  XEQ   |
         |   1    |   2    |   3    |   4    |   5    |   6    |
       A |   A    |   B    |   C    |   D    |   E    |   F    |
         +--------+--------+--------+--------+--------+--------+
       S |        |   %    |  Pi    |  ASIN  |  ACOS  |  ATAN  |
C        |  STO   |  RCL   |  R_dwn |   SIN  |   COS  |   TAN  |
         |   7    |   8    |   9    |   10   |   11   |   12   |
       A |   G    |   H    |   I    |    J   |    K   |    L   |
         +--------+--------+--------+--------+--------+--------+
       S | Complx |   %    |  Pi    |  ASIN  |  ACOS  |  ATAN  |
D        |  STO   |  RCL   |  R_dwn |   SIN  |   COS  |   TAN  |
         |   13   |   14   |   15   |   16   |   17   |   18   |
       A |   G    |   H    |   I    |    J   |    K   |    L   |
         +--------+--------+--------+--------+--------+--------+
       S |     Alpha       | Last x |  MODES |  DISP  |  CLEAR |
E        |     ENTER       |  x<>y  |  +/-   |   E    |   <--  |
         |       19        |   20   |   21   |   22   |   23   |
       A |                 |    M   |    N   |    O   |        |
         +--------+--------+-+------+----+---+-------++--------+
       S |   BST  | Solver   |  Int f(x) |  Matrix   |  STAT   |
F        |   Up   |    7     |     8     |     9     |   /     |
         |   24   |   25     |    26     |    27     |   28    |
       A |        |    P     |     Q     |     R     |    S    |
         +--------+----------+-----------+-----------+---------+
       S |   SST  |  BASE    |  CONVERT  |  FLAGS    |  PROB   |
G        |  Down  |    4     |     5     |     6     |    x    |
         |   29   |   30     |    31     |    32     |   33    |
       A |        |    T     |     U     |     V     |    W    |
         +--------+----------+-----------+-----------+---------+
       S |        | ASSIGN   |  CUSTOM   |  PGM.FCN  |  PRINT  |
H        |  SHIFT |    1     |     2     |     3     |    -    |
         |   34   |   35     |    36     |    37     |   38    |
       A |        |    X     |     Y     |     Z     |    -    |
         +--------+----------+-----------+-----------+---------+
       S |  OFF   |  TOP.FCN |   SHOW    |   PRGM    | CATALOG |
I        |  EXIT  |    0     |     .     |    R/S    |    +    |
         |   39   |   40     |    41     |    42     |   43    |
       A |        |    :     |     .     |     ?     |   ' '   |
         +--------+----------+-----------+-----------+---------+
            1        2           3           4           5     
*/
// delete and direct use of key = (key10

/*************************************************************************************
 * keyboard
*************************************************************************************/

// auto repeat time in msec for db48x
#define KB_DB_REPEAT_FIRST (1000)
#define KB_DB_REPEAT_PERIOD (100)




uint platform_plane(bool ls, bool rs, bool al, bool lc, bool trans);
uint platform_keyid(uint k, bool ls, bool rs, bool al, bool lc, bool trans);
uint platform_keyid(uint rc, uint plane);
uint compatible_key_position(uint key);
uint compatible_key_plane(uint keyid);


extern uint8_t MemToggle_B1;
extern uint8_t MemToggle_B2;
extern uint8_t MemToggle_C2;
extern uint8_t MemToggle_D1;
extern uint8_t MemToggle_D2;
extern uint8_t MemToggle_D3;
extern uint8_t MemToggle_E1;
extern uint8_t MemToggle_E2;
extern uint8_t MemToggle_F1;


// ============================================================================
//
//    Battery configuration
//
// ============================================================================

#define BATTERY_VMAX    3600    // Max battery on display

#endif // TARGET_DBU585_H
