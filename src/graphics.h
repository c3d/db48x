#ifndef GRAPHICS_H
#define GRAPHICS_H
// ****************************************************************************
//  graphics.h                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Low level graphic routines
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

#include "algebraic.h"
#include "command.h"
#include "list.h"
#include "stats.h"
#include "symbol.h"
#include "target.h"


struct PlotParameters : command
// ----------------------------------------------------------------------------
//   A replication of the PlotParameters / PPAR vaariable
// ----------------------------------------------------------------------------
{
    PlotParameters(id type = ID_PlotParameters) : command(type) {}

};


struct PlotParametersAccess
// ----------------------------------------------------------------------------
//  Access to the plot parameters
// ----------------------------------------------------------------------------
{
    PlotParametersAccess();

    object::id      type;
    algebraic_g     xmin;
    algebraic_g     ymin;
    algebraic_g     xmax;
    algebraic_g     ymax;
    symbol_g        independent;
    algebraic_g     imin;
    algebraic_g     imax;
    symbol_g        dependent;
    algebraic_g     resolution;
    algebraic_g     xorigin;
    algebraic_g     yorigin;
    algebraic_g     xticks;
    algebraic_g     yticks;
    text_g          xlabel;
    text_g          ylabel;

    static object_p name();

    bool            parse(list_p list);
    bool            parse(object_p n = name());

    bool            write(object_p n = name()) const;

    bool            check_validity() const;

    static coord    pixel_adjust(object_r    p,
                                 algebraic_r min,
                                 algebraic_r max,
                                 uint        scale,
                                 bool        isSize = false);
    static coord    size_adjust(object_r    p,
                                algebraic_r min,
                                algebraic_r max,
                                uint        scale);
    coord           pair_pixel_x(object_r pos) const;
    coord           pair_pixel_y(object_r pos) const;

    coord           pixel_x(algebraic_r pos) const;
    coord           pixel_y(algebraic_r pos) const;
};


inline uint ScreenWidth()
// ----------------------------------------------------------------------------
//   Adjustment for the radius of a circle or rounded rectangle
// ----------------------------------------------------------------------------
{
    return Screen.area().width();
}


inline uint ScreenHeight()
// ----------------------------------------------------------------------------
//   Adjustment for the radius of a circle or rounded rectangle
// ----------------------------------------------------------------------------
{
    return Screen.area().height();
}


object::result show(object_r obj);
// ----------------------------------------------------------------------------
//   Show the given object full screen
// ----------------------------------------------------------------------------


COMMAND_DECLARE(Disp,2);
COMMAND_DECLARE(DispXY,3);
COMMAND_DECLARE(Show,1);
COMMAND_DECLARE(PixOn,1);
COMMAND_DECLARE(PixOff, 1);
COMMAND_DECLARE(PixTest,1);
COMMAND_DECLARE(PixColor,1);
COMMAND_DECLARE(Line,2);
COMMAND_DECLARE(Ellipse,2);
COMMAND_DECLARE(Circle,2);
COMMAND_DECLARE(Rect,2);
COMMAND_DECLARE(RRect,3);
COMMAND_DECLARE(ClLCD,0);
COMMAND_DECLARE(Clip,1);
COMMAND_DECLARE(Freeze,1);
COMMAND_DECLARE(Header, 1);
COMMAND_DECLARE(CurrentClip,0);
COMMAND_DECLARE(ToGrob, 2);
COMMAND_DECLARE(GXor,3);
COMMAND_DECLARE(GOr,3);
COMMAND_DECLARE(GAnd,3);
COMMAND_DECLARE(Pict,0);
COMMAND_DECLARE(GraphicAppend,2);
COMMAND_DECLARE(GraphicStack,2);
COMMAND_DECLARE(GraphicSubscript,2);
COMMAND_DECLARE(GraphicExponent,2);
COMMAND_DECLARE(GraphicRatio,2);
COMMAND_DECLARE(GraphicRoot,1);
COMMAND_DECLARE(GraphicParentheses,1);
COMMAND_DECLARE(GraphicNorm,1);
COMMAND_DECLARE(GraphicSum,1);
COMMAND_DECLARE(GraphicProduct,1);
COMMAND_DECLARE(GraphicIntegral,1);
COMMAND_DECLARE(Gray,1);
COMMAND_DECLARE(RGB,3);

COMMAND_DECLARE(PlotMin,1);
COMMAND_DECLARE(PlotMax,1);
COMMAND_DECLARE(XRange,2);
COMMAND_DECLARE(YRange,2);
COMMAND_DECLARE(Scale,2);
COMMAND_DECLARE(XScale,1);
COMMAND_DECLARE(YScale,1);
COMMAND_DECLARE(Center,1);

#endif // GRAPHICS_H
