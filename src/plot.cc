// ****************************************************************************
//  plot.cc                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Function and curve plotting
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

#include "plot.h"

#include "arithmetic.h"
#include "compare.h"
#include "equations.h"
#include "expression.h"
#include "functions.h"
#include "graphics.h"
#include "grob.h"
#include "program.h"
#include "stats.h"
#include "sysmenu.h"
#include "target.h"
#include "variables.h"


void draw_axes(const PlotParametersAccess &ppar)
// ----------------------------------------------------------------------------
//   Draw axes
// ----------------------------------------------------------------------------
{
    coord   w    = display_width();
    coord   h    = display_height();
    coord   x    = ppar.pixel_adjust(+ppar.xorigin, ppar.xmin, ppar.xmax, w);
    coord   y    = ppar.pixel_adjust(+ppar.yorigin, ppar.ymax, ppar.ymin, h);
    coord   tx   = ppar.size_adjust(+ppar.xticks, ppar.xmin, ppar.xmax, w);
    coord   ty   = ppar.size_adjust(+ppar.yticks, ppar.ymin, ppar.ymax, h);
    uint64_t pat = Settings.Foreground();

    DISPLAY(

        // Draw axes proper
        display.fill(0, y, w, y, pat);
        display.fill(x, 0, x, h, pat);

        // Draw tick marks
        if (tx > 0)
        {
            for (coord i = tx; x + i <= w; i += tx)
                display.fill(x + i, y - 2, x + i, y + 2, pat);
            for (coord i = tx; x - i >= 0; i += tx)
                display.fill(x - i, y - 2, x - i, y + 2, pat);
        }
        if (ty > 0)
        {
            for (coord i = ty; y + i <= h; i += ty)
                display.fill(x - 2, y + i, x + 2, y + i, pat);
            for (coord i = ty; y - i >= 0; i += ty)
                display.fill(x - 2, y - i, x + 2, y - i, pat);
        }

        // Draw arrows at end of axes
        for (uint i = 0; i < 4; i++)
        {
            display.fill(w - 3*(i+1), y - i, w - 3*i, y + i, pat);
            display.fill(x - i, 3*i, x + i, 3*(i+1), pat);
        });

    ui.draw_dirty(0, 0, w, h);
}


uint draw_data(array::iterator &it, array::iterator &end,
               algebraic_g &x, algebraic_g &y, size_t xcol, size_t ycol)
// ----------------------------------------------------------------------------
//   Fetch data from a stats array
// ----------------------------------------------------------------------------
{
    if (it == end)
        return 0;

    object_p data = *it++;
    if (data->is_real())
    {
        y = algebraic_p(data);
        return 1;
    }
    else if (data->type() == object::ID_array)
    {
        array_p row = array_p(data);
        algebraic_g xx, yy;
        size_t col = 1;
        for (object_p cdata : *row)
        {
            if (!cdata->is_real())
                return false;
            if (col == xcol)
                xx = algebraic_p(cdata);
            if (col == ycol)
                yy = algebraic_p(cdata);
            if (xx && yy)
            {
                x = xx;
                y = yy;
                return 2;
            }
            col++;
        }
    }
    return 0;
}



object::result draw_plot(object::id                  kind,
                         const PlotParametersAccess &ppar,
                         object_g                    to_plot = nullptr)
// ----------------------------------------------------------------------------
//  Draw an equation that takes input from the stack
// ----------------------------------------------------------------------------
{
    object::result result = object::ERROR;
    coord          lx     = -1;
    coord          ly     = -1;
    uint           start  = sys_current_ms();
    algebraic_g    min, max, step;
    object::id     dname;

    // Select plotting parameters
    switch(kind)
    {
    default:
    case object::ID_Function:
        min = ppar.xmin;
        max = ppar.xmax;
        dname = object::ID_Equation;
        break;

    case object::ID_Polar:
    case object::ID_Parametric:
        min = ppar.imin;
        max = ppar.imax;
        step = ppar.resolution;
        if (step->is_zero())
            step = (max - min) / integer::make(display_width());
        dname = object::ID_Equation;
        break;

    case object::ID_Scatter:
    case object::ID_Bar:
        min = ppar.xmin;
        max = ppar.xmax;
        dname = object::ID_StatsData;
        break;
    }

    step = ppar.resolution;
    if (step->is_zero())
        step = (max - min) / integer::make(display_width());

    if (!to_plot)
    {
        to_plot = directory::recall_all(command::static_object(dname), false);
        if (!to_plot)
        {
            if (dname == object::ID_Equation)
                rt.no_equation_error();
            else
                rt.no_data_error();
            return object::ERROR;
        }
    }

    program_g       eq;
    array_g         data;
    array::iterator it, end;
    size_t          xcol = 0, ycol = 0;
    size            bar_width = 0, bar_skip = 0;
    size            bar_x = 0;
    coord           yzero = 0;

    if (dname == object::ID_Equation)
    {
        if (to_plot->type() == object::ID_equation)
        {
            to_plot = equation_p(+to_plot)->value();
            if (!to_plot)
                return object::ERROR;
        }

        if (!to_plot->is_program())
        {
            rt.invalid_equation_error();
            return object::ERROR;
        }
        eq = program_p(+to_plot);
    }
    else if (dname == object::ID_StatsData)
    {
        if (to_plot->type() != object::ID_array)
        {
            rt.invalid_plot_data_error();
            return object::ERROR;
        }

        size width = display_width();
        data = array_p(+to_plot);
        size_t items = data->items();
        data = array_p(+to_plot);
        step = (max - min) / integer::make(items);
        bar_skip = items && items < width ? width / items : 1;
        bar_width = bar_skip > 2 ? bar_skip - 2: bar_skip;
        it = data->begin();
        end = data->end();
        StatsParameters::Access stats;
        xcol = stats.xcol;
        ycol = stats.ycol;
        yzero = ppar.pixel_y(integer::make(0));
    }

    algebraic_g      x = min;
    algebraic_g      y;
    save<symbol_g *> iref(expression::independent,
                          (symbol_g *) &ppar.independent);
    settings::PrepareForFunctionEvaluation willEvaluateFunction;
    if (ui.draw_graphics())
        if (Settings.DrawPlotAxes())
            draw_axes(ppar);

    bool     split_points = Settings.NoCurveFilling();
    size     lw           = Settings.LineWidth();
    uint64_t fg           = Settings.Foreground();
    uint64_t errbg        = Settings.PlotErrorBackground();

    while (!program::interrupted())
    {
        coord rx     = 0;
        coord ry     = 0;
        uint  dcount = 1;
        if (dname == object::ID_Equation)
        {
            y = algebraic::evaluate_function(eq, x);
        }
        else
        {
            dcount = draw_data(it, end, x, y, xcol, ycol);
            if (!dcount)
                break;
        }

        if (y)
        {
            switch(kind)
            {
            default:
            case object::ID_Function:
                rx = ppar.pixel_x(x);
                ry = ppar.pixel_y(y);
                break;
            case object::ID_Polar:
            {
                algebraic_g i = rectangular::make(integer::make(0),
                                                  integer::make(1));
                y = y * exp::run(i * x);
            }
            // Fall-through
            case object::ID_Parametric:
                if (y->is_real())
                    y = rectangular::make(y, integer::make(0));
                if (y)
                {
                    if (algebraic_g cx = y->algebraic_child(0))
                        rx = ppar.pixel_x(cx);
                    if (algebraic_g cy = y->algebraic_child(1))
                        ry = ppar.pixel_y(cy);
                }
                break;

            case object::ID_Scatter:
            case object::ID_Bar:
                rx = ppar.pixel_x(x);
                ry = ppar.pixel_y(y);
                break;
            }
        }

        if (y)
        {
            if (kind != object::ID_Bar)
            {
                if (lx < 0 || split_points)
                {
                    lx = rx;
                    ly = ry;
                }
                DISPLAY(display.line(lx,ly,rx,ry, lw, fg));
            }
            else
            {
                lx = bar_x;
                ly = dcount == 1 ? yzero : rx;
                rx = lx + bar_width - 1;
                if (ry < ly)
                    std::swap(ly, ry);
                DISPLAY(display.fill(lx, ly, rx, ry, fg));
                bar_x += bar_skip;
            }
            ui.draw_dirty(lx, ly, rx, ry);
            lx = rx;
            ly = ry;
        }
        else
        {
            if (kind == object::ID_Function)
            {
                size dh = display_height();
                rx = ppar.pixel_x(x);
                ry = ppar.pixel_y(ppar.yorigin);
                if (ry < 0)
                    ry = 0;
                else if (ry >= coord(dh))
                    ry = dh - 1;
                rect r(rx, ry - LCD_H/32, rx, ry + LCD_H/32);
                DISPLAY(display.fill(r, errbg));
                ui.draw_dirty(r);
            }
            if (!rt.error())
                rt.invalid_function_error();
            uint64_t fg = Settings.Foreground();
            uint64_t bg = Settings.Background();
            DISPLAY(display.text(0, 0, rt.error(), ErrorFont, bg, fg));
            ui.draw_dirty(0, 0, LCD_W, ErrorFont->height());
            lx = ly = -1;
            rt.clear_error();
        }

        if (kind != object::ID_Scatter)
        {
            x = x + step;
            if (kind != object::ID_Bar)
            {
                algebraic_g cmp = x > max;
                if (!cmp)
                    goto err;
                if (cmp->as_truth(false))
                    break;
            }
        }
        uint end = sys_current_ms();
        if (end - start >= Settings.PlotRefreshRate())
        {
            refresh_dirty();
            start = sys_current_ms();
        }
    }
    result = object::OK;

err:
    refresh_dirty();
    return result;
}


static object::result draw_plot(object::id type)
// ----------------------------------------------------------------------------
//   Draw the various kinds of plot
// ----------------------------------------------------------------------------
{
    if (object_g eq = rt.pop())
    {
        PlotParametersAccess ppar;
        return draw_plot(type, ppar, eq);
    }
    return object::ERROR;
}


COMMAND_BODY(Function)
// ----------------------------------------------------------------------------
//   Draw plot from function on the stack taking stack arguments
// ----------------------------------------------------------------------------
{
    return draw_plot(ID_Function);
}


COMMAND_BODY(Parametric)
// ----------------------------------------------------------------------------
//   Draw plot from function on the stack taking stack arguments
// ----------------------------------------------------------------------------
{
    return draw_plot(ID_Parametric);
}


COMMAND_BODY(Polar)
// ----------------------------------------------------------------------------
//   Draw polar plot from function on the stack
// ----------------------------------------------------------------------------
{
    return draw_plot(ID_Polar);
}


COMMAND_BODY(Scatter)
// ----------------------------------------------------------------------------
//   Draw scatter plot from data on the stack
// ----------------------------------------------------------------------------
{
    return draw_plot(ID_Scatter);
}


COMMAND_BODY(Bar)
// ----------------------------------------------------------------------------
//   Draw bar plot from data on the stack
// ----------------------------------------------------------------------------
{
    return draw_plot(ID_Bar);
}


COMMAND_BODY(Draw)
// ----------------------------------------------------------------------------
//   Draw plot in EQ according to PPAR
// ----------------------------------------------------------------------------
{
    PlotParametersAccess ppar;
    switch(ppar.type)
    {
    default:
    case ID_Function:
    case ID_Parametric:
    case ID_Polar:
        return draw_plot(ppar.type, ppar);
    }
    rt.invalid_plot_type_error();
    return ERROR;
}


COMMAND_BODY(Drax)
// ----------------------------------------------------------------------------
//   Draw plot axes
// ----------------------------------------------------------------------------
{
    ui.draw_graphics();

    PlotParametersAccess ppar;
    draw_axes(ppar);
    refresh_dirty();

    return OK;
}
