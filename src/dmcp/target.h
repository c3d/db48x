#ifndef TARGET_DM42_H
#define TARGET_DM42_H
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
#include <dmcp.h>

enum target
// ----------------------------------------------------------------------------
//   Constants for a given target
// ----------------------------------------------------------------------------
{
    BITS_PER_PIXEL = 1,
    LCD_W          = 400,
    LCD_H          = 240,
    LCD_SCANLINE   = 416,
};

using surface = blitter::surface<blitter::mode::MONOCHROME_REVERSE>;
using color   = blitter::color  <blitter::mode::MONOCHROME_REVERSE>;
using pattern = blitter::pattern<blitter::mode::MONOCHROME_REVERSE>;
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
#if SIMULATOR
#  define FONT_QSPI
#else
#  define FONT_QSPI __attribute__((section(".fonts")))
#endif // SIMULATOR

/*
    KEYBOARD BIT MAP
    ----------------
    This is the bit number in the 64-bit keymatrix.
    Bit set means key is pressed.
    Note that DMCP does not define keys as bitmaps,
    but rather using keycodes.

      +--------+--------+--------+--------+--------+--------+
      |   F1   |   F2   |   F3   |   F4   |   F5   |   F6   |
      |   38   |   39   |   40   |   41   |   42   |   43   |
      +--------+--------+--------+--------+--------+--------+
    S |  Sum-  |  y^x   |  x^2   |  10^x  |  e^x   |  GTO   |
      |  Sum+  |  1/x   |  Sqrt  |  Log   |  Ln    |  XEQ   |
      |   1    |   2    |   3    |   4    |   5    |   6    |
    A |   A    |   B    |   C    |   D    |   E    |   F    |
      +--------+--------+--------+--------+--------+--------+
    S | Complx |   %    |  Pi    |  ASIN  |  ACOS  |  ATAN  |
      |  STO   |  RCL   |  R_dwn |   SIN  |   COS  |   TAN  |
      |   7    |   8    |   9    |   10   |   11   |   12   |
    A |   G    |   H    |   I    |    J   |    K   |    L   |
      +--------+--------+--------+--------+--------+--------+
    S |     Alpha       | Last x |  MODES |  DISP  |  CLEAR |
      |     ENTER       |  x<>y  |  +/-   |   E    |   <--  |
      |       13        |   14   |   15   |   16   |   17   |
    A |                 |    M   |    N   |    O   |        |
      +--------+--------+-+------+----+---+-------++--------+
    S |   BST  | Solver   |  Int f(x) |  Matrix   |  STAT   |
      |   Up   |    7     |     8     |     9     |   /     |
      |   18   |   19     |    20     |    21     |   22    |
    A |        |    P     |     Q     |     R     |    S    |
      +--------+----------+-----------+-----------+---------+
    S |   SST  |  BASE    |  CONVERT  |  FLAGS    |  PROB   |
      |  Down  |    4     |     5     |     6     |    x    |
      |   23   |   24     |    25     |    26     |   27    |
    A |        |    T     |     U     |     V     |    W    |
      +--------+----------+-----------+-----------+---------+
    S |        | ASSIGN   |  CUSTOM   |  PGM.FCN  |  PRINT  |
      |  SHIFT |    1     |     2     |     3     |    -    |
      |   28   |   29     |    30     |    31     |   32    |
    A |        |    X     |     Y     |     Z     |    -    |
      +--------+----------+-----------+-----------+---------+
    S |  OFF   |  TOP.FCN |   SHOW    |   PRGM    | CATALOG |
      |  EXIT  |    0     |     .     |    R/S    |    +    |
      |   33   |   34     |    35     |    36     |   37    |
    A |        |    :     |     .     |     ?     |   ' '   |
      +--------+----------+-----------+-----------+---------+

*/

#define KB_ALPHA             28         //! Alpha
#define KB_ON                33         //! ON
#define KB_ESC               33         //! Exit
#define KB_DOT               35         //! Dot
#define KB_SPC               36         //! Space (on R/S)
#define KB_RUNSTOP           36         //! R/S
#define KB_QUESTION          36         //! ?
#define KB_SHIFT             28         //! Shift
#define KB_LSHIFT            28         //! Left shift
#define KB_RSHIFT            28         //! Right shift

#define KB_ADD               37         //! +
#define KB_SUB               32         //! -
#define KB_MUL               27         //! *
#define KB_DIV               22         //! /

#define KB_ENT               13         //! ENTER
#define KB_BKS               17         //! backspace
#define KB_UP                18         //! up arrow
#define KB_DN                23         //! down arrow
#define KB_LF                18         //! left arrow
#define KB_RT                23         //! right arrow

#define KB_F1                38         //! Function key 1
#define KB_F2                39         //! Function key 2
#define KB_F3                40         //! Function key 3
#define KB_F4                41         //! Function key 4
#define KB_F5                42         //! Function key 5
#define KB_F6                43         //! Function key 6

#define KB_0                 34         //! 0
#define KB_1                 29         //! 1
#define KB_2                 30         //! 2
#define KB_3                 31         //! 3
#define KB_4                 24         //! 4
#define KB_5                 25         //! 5
#define KB_6                 26         //! 6
#define KB_7                 19         //! 7
#define KB_8                 20         //! 8
#define KB_9                 21         //! 9

#define KB_A                  1         //! A
#define KB_B                  2         //! B
#define KB_C                  3         //! C
#define KB_D                  4         //! D
#define KB_E                  5         //! E
#define KB_F                  6         //! F
#define KB_G                  7         //! G
#define KB_H                  8         //! H
#define KB_I                  9         //! I
#define KB_J                 10         //! J
#define KB_K                 11         //! K
#define KB_L                 12         //! L
#define KB_M                 14         //! M
#define KB_N                 15         //! N
#define KB_O                 16         //! O
#define KB_P                 19         //! P
#define KB_Q                 20         //! Q
#define KB_R                 21         //! R
#define KB_S                 22         //! S
#define KB_T                 24         //! T
#define KB_U                 25         //! U
#define KB_V                 26         //! V
#define KB_W                 27         //! W
#define KB_X                 29         //! X
#define KB_Y                 30         //! Y
#define KB_Z                 31         //! Z

#endif // TARGET_DM42_H
