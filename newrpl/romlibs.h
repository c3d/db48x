/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef ROMLIBS_H
#define ROMLIBS_H

// LIST WITH ALL LIBRARIES TO BE INCLUDED IN ROM
#define ROM_LIST \
    INCLUDELIB(0,lib-zero-messages.c) , \
    INCLUDELIB(2,lib-two-ident.c) , \
    INCLUDELIB(8,lib-eight-docol.c) , \
    INCLUDELIB(9,lib-nine-docol2.c) , \
    INCLUDELIB(10,lib-ten-reals.c) , \
    INCLUDELIB(12,lib-twelve-bint.c) , \
    INCLUDELIB(20,lib-20-comments.c) , \
    INCLUDELIB(24,lib-24-string.c) , \
    INCLUDELIB(28,lib-28-dirs.c) , \
    INCLUDELIB(30,lib-30-complex.c) , \
    INCLUDELIB(32,lib-32-lam.c) , \
    INCLUDELIB(48,lib-48-angles.c), \
    INCLUDELIB(52,lib-52-matrix.c) , \
    INCLUDELIB(54,lib-54-units.c) , \
    INCLUDELIB(55,lib-55-constants.c) , \
    INCLUDELIB(56,lib-56-symbolic.c) , \
    INCLUDELIB(62,lib-62-lists.c) , \
    INCLUDELIB(64,lib-64-arithmetic.c) , \
    INCLUDELIB(65,lib-65-system.c) , \
    INCLUDELIB(66,lib-66-transcendentals.c) , \
    INCLUDELIB(68,lib-68-flags.c) , \
    INCLUDELIB(70,lib-70-binary.c) , \
    INCLUDELIB(72,lib-72-stack.c) , \
    INCLUDELIB(74,lib-74-sdcard.c) , \
    INCLUDELIB(76,lib-76-ui.c) , \
    INCLUDELIB(77,lib-77-libdata.c) , \
    INCLUDELIB(78,lib-78-fonts.c) , \
    INCLUDELIB(80,lib-80-bitmaps.c) , \
    INCLUDELIB(88,lib-88-plot.c) , \
    INCLUDELIB(96,lib-96-composites.c) , \
    INCLUDELIB(98,lib-98-statistics.c) , \
    INCLUDELIB(100,lib-100-usb.c) , \
    INCLUDELIB(102,lib-102-libptr.c) , \
    INCLUDELIB(104,lib-104-solvers.c) , \
    INCLUDELIB(112,lib-112-asm.c) , \
    INCLUDELIB(4080,lib-4080-localenv.c) , \
    INCLUDELIB(4081,lib-4081-tags.c) , \
    INCLUDELIB(4090,lib-4090-overloaded.c)

// ADD MORE LIBRARIES HERE

#endif // ROMLIBS_H
