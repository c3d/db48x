// ****************************************************************************
//  graphics.cc                                                   DB48X project
// ****************************************************************************
//
//   File Description:
//
//     RPL graphic routines
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

#include "graphics.h"

#include "arithmetic.h"
#include "bignum.h"
#include "blitter.h"
#include "compare.h"
#include "complex.h"
#include "decimal.h"
#include "expression.h"
#include "grob.h"
#include "integer.h"
#include "list.h"
#include "sysmenu.h"
#include "target.h"
#include "tests.h"
#include "user_interface.h"
#include "util.h"
#include "variables.h"
#include <atomic>

typedef const based_integer *based_integer_p;
typedef const based_bignum  *based_bignum_p;
using std::max;
using std::min;

RECORDER(graphics, 16, "Graphics");


// ============================================================================
//
//   Plot parameters
//
// ============================================================================

PlotParametersAccess::PlotParametersAccess()
// ----------------------------------------------------------------------------
//   Default values
// ----------------------------------------------------------------------------
    : type(command::ID_Function),
      xmin(integer::make(-10)),
      ymin(integer::make(-6)),
      xmax(integer::make(10)),
      ymax(integer::make(6)),
      independent(symbol::make("x")),
      imin(integer::make(-10)),
      imax(integer::make(10)),
      dependent(symbol::make("y")),
      dmin(integer::make(-6)),
      dmax(integer::make(6)),
      resolution(integer::make(0)),
      xorigin(integer::make(0)),
      yorigin(integer::make(0)),
      xticks(integer::make(1)),
      yticks(integer::make(1)),
      xlabel(text::make("x")),
      ylabel(text::make("y"))
{
    parse();
}


object_p PlotParametersAccess::name()
// ----------------------------------------------------------------------------
//   Return the name for the variable
// ----------------------------------------------------------------------------
{
    return command::static_object(object::ID_PlotParameters);

}


bool PlotParametersAccess::parse(list_p parms)
// ----------------------------------------------------------------------------
//   Parse a PPAR / PlotParametersAccess list
// ----------------------------------------------------------------------------
{
    if (!parms)
        return false;

    uint index = 0;
    for (object_p obj: *parms)
    {
        bool valid = false;
        record(graphics, "%u: %t", index, obj);
        switch(index)
        {
        case 0:                 // xmin,ymin
        case 1:                 // xmax,ymax
            if (algebraic_g xa = obj->algebraic_child(0))
            {
                if (algebraic_g ya = obj->algebraic_child(1))
                {
                    record(graphics, "%u: xa=%t ya=%t", index, +xa, +ya);
                    (index ? xmax : xmin) = xa;
                    (index ? ymax : ymin) = ya;
                    valid = true;
                }
            }
            break;

        case 2:                 // Independent variable
            if (symbol_g sym = obj->as_quoted<symbol>())
            {
                independent = sym;
                valid = true;
            }
            else if (list_g ilist = obj->as<list>())
            {
                int ok = 0;
                if (object_p name = ilist->at(0))
                    if (symbol_p sym = name->as<symbol>())
                        ok++, independent = sym;
                if (object_p obj = ilist->at(1))
                    if (algebraic_p val = obj->as_real())
                        ok++, imin = val;
                if (object_p obj = ilist->at(2))
                    if (algebraic_p val = obj->as_real())
                        ok++, imax = val;
                valid = ok == 3;
            }
            break;

        case 3:
            valid = obj->is_real() || obj->is_based();
            if (valid)
                resolution = algebraic_p(obj);
            break;
        case 4:
            if (list_g origin = obj->as<list>())
            {
                obj = origin->at(0);
                if (object_p ticks = origin->at(1))
                {
                    if (ticks->is_real() || ticks->is_based())
                    {
                        xticks = yticks = algebraic_p(ticks);
                        valid = true;
                    }
                    else if (list_p tickxy = ticks->as<list>())
                    {
                        if (algebraic_g xa = tickxy->algebraic_child(0))
                        {
                            if (algebraic_g ya = tickxy->algebraic_child(0))
                            {
                                xticks = xa;
                                yticks = ya;
                                valid = true;
                            }
                        }
                    }

                }
                if (valid)
                {
                    if (object_p xl = origin->at(2))
                    {
                        valid = false;
                        if (object_p yl = origin->at(3))
                        {
                            if (text_p xt = xl->as<text>())
                            {
                                if (text_p yt = yl->as<text>())
                                {
                                    xlabel = xt;
                                    ylabel = yt;
                                    valid = true;
                                }
                            }
                        }
                    }
                }
                if (!valid)
                {
                    rt.invalid_ppar_error();
                    return false;
                }
            }
            if (obj->is_complex())
            {
                if (algebraic_g xa = obj->algebraic_child(0))
                {
                    if (algebraic_g ya = obj->algebraic_child(1))
                    {
                        xorigin = xa;
                        yorigin = ya;
                        valid = true;
                    }
                }
            }
            break;
        case 5:
            valid = obj->is_plot();
            if (valid)
                type = obj->type();
            break;

        case 6:                 // Dependent variable
            if (symbol_g sym = obj->as_quoted<symbol>())
            {
                dependent = sym;
                valid = true;
            }
            else if (list_g ilist = obj->as<list>())
            {
                int ok = 0;
                if (object_p name = ilist->at(0))
                    if (symbol_p sym = name->as<symbol>())
                        ok++, dependent = sym;
                if (object_p obj = ilist->at(1))
                    if (algebraic_p val = obj->as_real())
                        ok++, dmin = val;
                if (object_p obj = ilist->at(2))
                    if (algebraic_p val = obj->as_real())
                        ok++, dmax = val;
                valid = ok == 3;
            }
            break;

        default:
            break;
        }

        if (!valid)
        {
            rt.invalid_ppar_error();
            return false;
        }

        index++;
    }

    // Check that we have sane input
    if (!check_validity())
    {
        rt.invalid_ppar_error();
        return false;
    }

    return true;
}


bool PlotParametersAccess::parse(object_p name)
// ----------------------------------------------------------------------------
//   Parse plot parameters from a variable name
// ----------------------------------------------------------------------------
{
    if (object_p obj = directory::recall_all(name, false))
        if (list_p parms = obj->as<list>())
            return parse(parms);
    return false;
}


bool PlotParametersAccess::write(object_p name) const
// ----------------------------------------------------------------------------
//   Write out the plot parameters in case they were changed
// ----------------------------------------------------------------------------
{
    if (!check_validity())
    {
        rt.invalid_ppar_error();
        return false;
    }

    if (directory *dir = rt.variables(0))
    {
        rectangular_g zmin  = rectangular::make(xmin, ymin);
        rectangular_g zmax  = rectangular::make(xmax, ymax);
        list_g        indep = list::make(independent, imin, imax);
        complex_g     zorig = rectangular::make(xorigin, yorigin);
        list_g        ticks = list::make(xticks, yticks);
        list_g        axes  = list::make(zorig, ticks, xlabel, ylabel);
        object_g      ptype = command::static_object(type);
        list_g        dep   = list::make(dependent, dmin, dmax);

        list_g        par =
            list::make(zmin, zmax, indep, resolution, axes, ptype, dep);
        if (par)
            return dir->store(name, +par);
    }
    return false;

}


bool PlotParametersAccess::check_validity() const
// ----------------------------------------------------------------------------
//   Check validity of the plot parameters
// ----------------------------------------------------------------------------
{
    // All labels must be defined
    if (!xmin|| !xmax|| !ymin|| !ymax)
        return false;
    if (!independent || !imin || !imax)
        return false;
    if (!dependent   || !dmin || !dmax)
        return false;
    if (!resolution|| !xorigin|| !yorigin)
        return false;
    if (!xticks|| !yticks|| !xlabel|| !ylabel)
        return false;

    // Check values that must be real
    if (!xmin->is_real() || !xmax->is_real())
        return false;
    if (!ymin->is_real() || !ymax->is_real())
        return false;
    if (!imin->is_real() || !imax->is_real())
        return false;
    if (!dmin->is_real() || !dmax->is_real())
        return false;
    if (!resolution->is_real())
        return false;
    if (!xorigin->is_real() || !yorigin->is_real())
        return false;
    if (!xticks->is_real() && !xticks->is_based())
        return false;
    if (!yticks->is_real() && !yticks->is_based())
        return false;
    if (xlabel->type() != object::ID_text || ylabel->type() != object::ID_text)
        return false;

    // Check that the ranges are not empty
    algebraic_g test = xmin >= xmax;
    if (test->as_truth(true))
        return false;
    test = ymin >= ymax;
    if (test->as_truth(true))
        return false;
    test = imin >= imax;
    if (test->as_truth(true))
        return false;
    test = dmin >= dmax;
    if (test->as_truth(true))
        return false;

    return true;
}



// ============================================================================
//
//   Coordinate conversions
//
// ============================================================================

coord PlotParametersAccess::pixel_adjust(object_r    obj,
                                         algebraic_r min,
                                         algebraic_r max,
                                         uint        scale,
                                         bool        isSize)
// ----------------------------------------------------------------------------
//  Convert an object to a coordinate
// ----------------------------------------------------------------------------
{
    if (!obj)
        return 0;

    coord       result = 0;
    object::id  ty     = obj->type();

    switch(ty)
    {
    case object::ID_integer:
    case object::ID_neg_integer:
    case object::ID_bignum:
    case object::ID_neg_bignum:
    case object::ID_fraction:
    case object::ID_neg_fraction:
    case object::ID_big_fraction:
    case object::ID_neg_big_fraction:
    case object::ID_hwfloat:
    case object::ID_hwdouble:
    case object::ID_decimal:
    case object::ID_neg_decimal:
    {
        algebraic_g range  = max - min;
        algebraic_g pos    = algebraic_p(+obj);
        algebraic_g sa     = integer::make(scale);

        // Avoid divide by zero for bogus input
        if (!range || range->is_zero())
            range = integer::make(1);

        if (!isSize)
            pos = pos - min;
        pos = pos / range * sa;
        if (pos)
            result = pos->as_int32(0, false);
        return result;
    }

#if CONFIG_FIXED_BASED_OBJECTS
    case object::ID_hex_integer:
    case object::ID_dec_integer:
    case object::ID_oct_integer:
    case object::ID_bin_integer:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case object::ID_based_integer:
        result = based_integer_p(+obj)->value<ularge>();
        break;

#if CONFIG_FIXED_BASED_OBJECTS
    case object::ID_hex_bignum:
    case object::ID_dec_bignum:
    case object::ID_oct_bignum:
    case object::ID_bin_bignum:
#endif // CONFIG_FIXED_BASED_OBJECTS
    case object::ID_based_bignum:
        result = based_bignum_p(+obj)->value<ularge>();
        break;

    default:
        rt.type_error();
        break;
    }

    return result;
}


coord PlotParametersAccess::size_adjust(object_r    p,
                                        algebraic_r min,
                                        algebraic_r max,
                                        uint        scale)
// ----------------------------------------------------------------------------
//   Adjust the size of the parameters
// ----------------------------------------------------------------------------
{
    return pixel_adjust(p, min, max, scale, true);
}



coord PlotParametersAccess::pair_pixel_x(object_r pos) const
// ----------------------------------------------------------------------------
//   Given a position (can be a complex, a list or a vector), return x
// ----------------------------------------------------------------------------
{
    if (object_g x = pos->child(0))
        return pixel_adjust(x, xmin, xmax, display_width());
    return 0;
}


coord PlotParametersAccess::pair_pixel_y(object_r pos) const
// ----------------------------------------------------------------------------
//   Given a position (can be a complex, a list or a vector), return y
// ----------------------------------------------------------------------------
{
    if (object_g y = pos->child(1))
        return pixel_adjust(y, ymax, ymin, display_height());
    return 0;
}


coord PlotParametersAccess::pixel_x(algebraic_r x) const
// ----------------------------------------------------------------------------
//   Adjust a position given as an algebraic value
// ----------------------------------------------------------------------------
{
    object_g xo = object_p(+x);
    return pixel_adjust(xo, xmin, xmax, display_width());
}


coord PlotParametersAccess::pixel_y(algebraic_r y) const
// ----------------------------------------------------------------------------
//   Adjust a position given as an algebraic value
// ----------------------------------------------------------------------------
{
    object_g yo = object_p(+y);
    return pixel_adjust(yo, ymax, ymin, display_height());
}




// ============================================================================
//
//   Commands
//
// ============================================================================

COMMAND_BODY(Disp)
// ----------------------------------------------------------------------------
//   Display text on the given line
// ----------------------------------------------------------------------------
//   For compatibility reasons, integer values of the line from 1 to 8
//   are positioned like on the HP48, each line taking 30 pixels
//   The coordinate can additionally be one of:
//   - A non-integer value, which allows more precise positioning on screen
//   - A complex number, where the real part is the horizontal position
//     and the imaginary part is the vertical position going up
//   - A list { x y } with the same meaning as for a complex
//   - A list { #x #y } to give pixel-precise coordinates
{
    if (object_g pos = rt.pop())
    {
        if (object_g todisp = rt.pop())
        {
            PlotParametersAccess ppar;
            coord                x      = 0;
            coord                y      = 0;
            font_p               font   = settings::font(Settings.StackFont());
            bool                 erase  = true;
            bool                 invert = false;
            id                   ty     = pos->type();
            blitter::size        width  = display_width();
            algebraic_g          halign, valign;

            if (ty == ID_rectangular || ty == ID_polar ||
                ty == ID_list || ty == ID_array)
            {
                x = ppar.pair_pixel_x(pos);
                y = ppar.pair_pixel_y(pos);

                if (list_g args = pos->as_array_or_list())
                {
                    if (object_p fontid = args->at(2))
                    {
                        uint32_t i = fontid->as_uint32(settings::STACK, false);
                        font = settings::font(settings::font_id(i));
                    }
                    if (object_p eflag = args->at(3))
                        erase = eflag->as_truth(true);
                    if (object_p iflag = args->at(4))
                        invert = iflag->as_truth(true);
                    if (object_p aflag = args->at(5))
                        if (algebraic_p al = aflag->as_real())
                            halign = al;
                    if (object_p aflag = args->at(6))
                        if (algebraic_p al = aflag->as_real())
                            valign = al;
                }
            }
            else if (pos->is_based())
            {
                algebraic_g ya = algebraic_p(+pos);
                y = ppar.pixel_y(ya);
            }
            else if (pos->is_algebraic())
            {
                algebraic_g ya = algebraic_p(+pos);
                ya = ya * integer::make(LCD_H/8);
                y = ya->as_uint32(0, false) - (LCD_H/8);
            }
            else
            {
                rt.type_error();
                return ERROR;
            }


            utf8          txt = nullptr;
            size_t        len = 0;
            blitter::size h   = font->height();

            if (text_p t = todisp->as<text>())
                txt = t->value(&len);
            else if (text_p tr = todisp->as_text())
                txt = tr->value(&len);

            uint64_t bg   = Settings.Background();
            uint64_t fg   = Settings.Foreground();
            utf8     last = txt + len;
            coord    x0   = x;

            if (invert)
                std::swap(bg, fg);
            ui.draw_graphics();

            if (halign || valign)
            {
                blitter::size width  = 0;
                blitter::size lwidth = 0;
                uint rows = 1;
                for (utf8 p = txt; p < last; p = utf8_next(p))
                {
                    unicode cp = utf8_codepoint(p);
                    if (cp == '\n')
                    {
                        if (width < lwidth)
                            width = lwidth;
                        lwidth = 0;
                        rows++;
                    }
                    else
                    {
                        blitter::size w = font->width(cp);
                        lwidth += w;
                    }
                }
                if (width < lwidth)
                    width = lwidth;
                if (halign)
                {
                    algebraic_g o = integer::make(width/2);
                    o = o * halign;
                    if (o)
                        x0 += o->as_int32(0, false);
                    x0 -= width/2;
                    x = x0;
                }
                if (valign)
                {
                    blitter::size height = rows * font->height();
                    algebraic_g o = integer::make(height / 2);
                    o = o * valign;
                    if (o)
                        y += o->as_int32(0, false);
                    y -= height / 2;
                }
            }

            while (txt < last)
            {
                unicode       cp = utf8_codepoint(txt);
                blitter::size w  = font->width(cp);

                txt = utf8_next(txt);
                if (cp == '\n' || (!halign && x + w >= width))
                {
                    x = x0;
                    y += font->height();
                    if (cp == '\n')
                        continue;
                }
                if (cp == '\t')
                    cp = ' ';

                DISPLAY(if (erase)
                            display.fill(x, y, x+w-1, y+h-1, bg);
                        display.glyph(x, y, cp, font, fg));
                ui.draw_dirty(x, y , x+w-1, y+h-1);
                x += w;
            }

            ui.refresh();
            return OK;
        }
    }
    return ERROR;
}


COMMAND_BODY(DispXY)
// ----------------------------------------------------------------------------
//   Temporarily change the font setting, otherwise same as Disp
// ----------------------------------------------------------------------------
{
    if (object_g fsize = rt.pop())
    {
        uint fsz = fsize->as_uint32(0, true);
        if (!rt.error())
        {
            settings::font_id fid = settings::font_id(fsz);
            settings::SaveStackFont ssf(fid);
            return Disp::evaluate();
        }
        else
        {
            // Restore stack
            rt.push(fsize);
        }

    }
    return ERROR;
}


void draw_prompt(text_r msg)
// ----------------------------------------------------------------------------
//   Draw a prompt for user input commands
// ----------------------------------------------------------------------------
{
    size_t len = 0;
    utf8   txt = msg->value(&len);
    return draw_prompt(txt, len);
}


void draw_prompt(utf8 txt, size_t len)
// ----------------------------------------------------------------------------
//   Draw a prompt for user input commands
// ----------------------------------------------------------------------------
{
    using size   = blitter::size;
    using coord  = blitter::coord;

    coord   y    = 0;
    coord   x    = 0;
    bool    clr  = true;
    utf8    last = txt + len;
    pattern bg   = Settings.Background();
    pattern fg   = Settings.Foreground();
    font_p  font = settings::font(Settings.StackFont());
    size    h    = font->height();

    while (txt < last)
    {
        unicode cp = utf8_codepoint(txt);
        size    w  = font->width(cp);
        txt        = utf8_next(txt);
        if (cp == '\n' || x + w >= LCD_W)
        {
            x = 0;
            y += h;
            clr = true;
            if (cp == '\n')
                continue;
        }
        if (cp == '\t')
            cp = ' ';

        if (clr)
        {
            Screen.fill(0, y, LCD_W-1, y+h-1, bg);
            Screen.fill(0, y+h, LCD_W-1, y+h, fg);
            ui.draw_dirty(0, y, LCD_W-1, y+h);
            clr = false;
        }
        Screen.glyph(x, y, cp, font, fg);
        x += w;
    }
    ui.freeze(1);
    ui.stack_screen_top(y + h + 1);

    uint    top  = ui.stack_screen_top();
    uint    bot  = ui.stack_screen_bottom();
    Screen.fill(0, top, LCD_W-1, bot, bg);
    ui.draw_dirty(0, top, LCD_W-1, bot);
    ui.refresh();
}


COMMAND_BODY(Prompt)
// ----------------------------------------------------------------------------
//   Display the given message in the first line, then halt program
// ----------------------------------------------------------------------------
{
    if (object_p msgo = rt.pop())
    {
        text_g msg = msgo->as_text();
        if (msg)
        {
            draw_prompt(msg);
            program::halted = true;
            program::stepping = 0;
            return OK;
        }
    }
    return ERROR;
}



// ============================================================================
//
//    Input command and modes
//
// ============================================================================

static void configure_alpha()
// ----------------------------------------------------------------------------
//   Configure for alphabetic mode
// ----------------------------------------------------------------------------
{
    ui.alpha_plane(1);
    ui.editing_mode(ui.TEXT);
}


static bool validate_alpha(gcutf8 &src, size_t len)
// ----------------------------------------------------------------------------
//  Check if we can accept an alphabetic value
// ----------------------------------------------------------------------------
{
    if (text_p text = text::make(+src, len))
        return rt.push(+text);
    return false;
}


static void configure_alg()
// ----------------------------------------------------------------------------
//   Configure for algebraic mode
// ----------------------------------------------------------------------------
{
    ui.alpha_plane(0);
    ui.editing_mode(ui.ALGEBRAIC);
}


static bool validate_alg(gcutf8 &src, size_t len)
// ----------------------------------------------------------------------------
//  Check if we can accept an algebraic value and return it as text
// ----------------------------------------------------------------------------
{
    if (object_p obj = object::parse_all(src, len))
        if (obj->as_extended_algebraic())
            if (text_p text = text::make(src, len))
                return rt.push(+text);
    return false;
}


static bool validate_algebraic(gcutf8 &src, size_t len)
// ----------------------------------------------------------------------------
//  Check if we can accept an algebraic value and return it as algebraic
// ----------------------------------------------------------------------------
{
    if (object_p obj = object::parse_all(src, len))
        if (algebraic_p alg = obj->as_extended_algebraic())
            return rt.push(alg);
    return false;
}


static bool validate_expression(gcutf8 &src, size_t len)
// ----------------------------------------------------------------------------
//  Check if we can accept an algeraic value and return it as expression
// ----------------------------------------------------------------------------
{
    if (expression_p expr = expression::parse_all(src, len))
        return rt.push(expr);
    return false;
}


static void configure_value()
// ----------------------------------------------------------------------------
//   Configure for algebraic mode to enter a single value
// ----------------------------------------------------------------------------
{
    ui.alpha_plane(0);
    ui.editing_mode(ui.ALGEBRAIC);
}


static bool validate_value(gcutf8 &src, size_t len)
// ----------------------------------------------------------------------------
//  Check if we can accept a single object
// ----------------------------------------------------------------------------
{
    if (object_p obj = object::parse_all(src, len))
        return rt.push(obj);
    return false;
}


static void configure_values()
// ----------------------------------------------------------------------------
//   Configure for program mode to enter an RPL command line
// ----------------------------------------------------------------------------
{
    ui.alpha_plane(0);
    ui.editing_mode(ui.PROGRAM);
}


static bool validate_values(gcutf8 &src, size_t len)
// ----------------------------------------------------------------------------
//  Check if we can accept a command line
// ----------------------------------------------------------------------------
{
    if (program_p cmds = program::parse(src, len))
        return rt.push(cmds);
    return false;
}


static bool validate_values_source(gcutf8 &src, size_t len)
// ----------------------------------------------------------------------------
//  Check if we can accept a command line
// ----------------------------------------------------------------------------
{
    if (program::parse(src, len))
        if (text_p txt = text::make(src, len))
            return rt.push(txt);
    return false;
}


static bool validate_number(gcutf8 &src, size_t len)
// ----------------------------------------------------------------------------
//  Check if we can accept a number
// ----------------------------------------------------------------------------
{
    if (object_p obj = object::parse_all(src, len))
        if (obj->is_real() || obj->is_complex())
            return rt.push(obj);
    return false;
}


static bool validate_real(gcutf8 &src, size_t len)
// ----------------------------------------------------------------------------
//  Check if we can accept a real number
// ----------------------------------------------------------------------------
{
    if (object_p obj = object::parse_all(src, len))
        if (obj->is_real())
            return rt.push(obj);
    return false;
}


static bool validate_integer(gcutf8 &src, size_t len)
// ----------------------------------------------------------------------------
//  Check if we can accept an integer number
// ----------------------------------------------------------------------------
{
    if (object_p obj = object::parse_all(src, len))
        if (obj->is_integer())
            return rt.push(obj);
    return false;
}


static bool validate_positive(gcutf8 &src, size_t len)
// ----------------------------------------------------------------------------
//  Check if we can accept an integer number
// ----------------------------------------------------------------------------
{
    if (object_p obj = object::parse_all(src, len))
        if (obj->is_integer())
            if (!obj->is_negative(false))
                return rt.push(obj);
    return false;
}


static const struct validate_input_lookup
// ----------------------------------------------------------------------------
//   Structure associating a name to an input lookup function
// ----------------------------------------------------------------------------
{
    cstring name;
    void (*configure)();
    bool (*validate)(gcutf8 &src, size_t len);
}

input_validators[] =
// ----------------------------------------------------------------------------
//   List of input validators
// ----------------------------------------------------------------------------
{
    { "α",              configure_alpha,        validate_alpha },
    { "alpha",          configure_alpha,        validate_alpha },
    { "text",           configure_alpha,        validate_alpha },
    { "alg",            configure_alg,          validate_alg },
    { "algebraic",      configure_alg,          validate_algebraic },
    { "expression",     configure_alg,          validate_expression },
    { "value",          configure_value,        validate_value },
    { "object",         configure_value,        validate_value },
    { "v",              configure_values,       validate_values_source },
    { "values",         configure_values,       validate_values_source },
    { "objects",        configure_values,       validate_values_source },
    { "p",              configure_values,       validate_values },
    { "prog",           configure_values,       validate_values },
    { "program",        configure_values,       validate_values },
    { "n",              configure_alg,          validate_number },
    { "number",         configure_alg,          validate_number },
    { "r",              configure_alg,          validate_real },
    { "real",           configure_alg,          validate_real },
    { "i",              configure_alg,          validate_integer },
    { "integer",        configure_alg,          validate_integer },
    { "positive",       configure_alg,          validate_positive },
};



COMMAND_BODY(Input)
// ----------------------------------------------------------------------------
//   Display the given message in the first line, then halt program for input
// ----------------------------------------------------------------------------
{
    bool (*validate)(gcutf8 &, size_t) = validate_alpha;
    void (*config)() = configure_alpha;

    object_p edo  = rt.pop();
    object_p msgo = rt.pop();
    uint     pos  = ~0;

    // Check the prompt
    text_g   msg  = msgo->as_text();

    // Check the editor value
    text_g ed;
    if (list_g lst = edo->as_array_or_list())
    {
        edo = lst->at(0);
        if (edo)
        {
            ed = edo->as_text();
            if (!ed)
                goto error;
            object_p poso = lst->at(1);
            if (poso)
            {
                if (poso->is_real())
                {
                    pos = poso->as_uint32(0, true) - 1;
                }
                else if (list_p posl = poso->as_array_or_list())
                {
                    uint row = 0;
                    uint col = 0;
                    if (object_p rowo = posl->at(0))
                    {
                        row = rowo->as_uint32(0, true) - 1;
                        if (object_p colo = posl->at(1))
                            col = colo->as_uint32(0, true) - 1;
                    }
                    size_t sz  = 0;
                    utf8   edt = ed->value(&sz);
                    while (pos < sz)
                    {
                        if (!row)
                        {
                            if (!col)
                                break;
                            col--;
                        }
                        if (edt[pos] == '\n')
                            row--;
                        pos++;
                    }
                }
                else
                {
                    rt.value_error();
                    goto error;
                }

                if (object_p valo = lst->at(2))
                {
                    id ty = valo->type();
                    if (ty == ID_text || ty == ID_symbol)
                    {
                        text_p valn = text_p(valo);
                        size_t sz   = 0;
                        utf8   valt = valn->value(&sz);
                        for (const auto &p : input_validators)
                        {
                            if (strncasecmp(p.name, cstring(valt), sz) == 0)
                            {
                                validate = p.validate;
                                config = p.configure;
                                break;
                            }
                        }
                    }
                    else if (ty == ID_program)
                    {
                        validate = (typeof validate) valo;
                    }
                }
            }
        }
    }
    else
    {
        ed = edo->as_text();
    }

    if (msg && ed)
    {
        draw_prompt(msg);

        size_t sz  = 0;
        utf8   txt = ed->value(&sz);
        if (pos > sz)
            pos = sz;
        rt.edit(txt, sz);
        config();
        ui.cursor_position(pos);
        ui.input(validate);
        program::halted = true;
        program::stepping = 0;
        return OK;
    }

error:
    if (!rt.error())
        rt.value_error();
    return ERROR;
}


static object::result compile_to(bool (*validator)(gcutf8 &src, size_t sz))
// ----------------------------------------------------------------------------
//   Process text input to check if it has the expected type
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.pop())
    {
        if (text_p srct = top->as<text>())
        {
            size_t length = 0;
            gcutf8 src = srct->value(&length);
            if (validator(src, length))
                return object::OK;
            rt.input_validation_error();
        }
        else
        {
            rt.type_error();
        }
    }
    return object::ERROR;
}


COMMAND_BODY(CompileToAlgebraic){ return compile_to(validate_algebraic); }
COMMAND_BODY(CompileToNumber)   { return compile_to(validate_number); }
COMMAND_BODY(CompileToInteger)  { return compile_to(validate_integer); }
COMMAND_BODY(CompileToPositive) { return compile_to(validate_positive); }
COMMAND_BODY(CompileToReal)     { return compile_to(validate_real); }
COMMAND_BODY(CompileToObject)   { return compile_to(validate_value); }
COMMAND_BODY(CompileToExpression){ return compile_to(validate_expression); }




// ============================================================================
//
//    Show command
//
// ============================================================================

COMMAND_BODY(Show)
// ----------------------------------------------------------------------------
//   Show the top-level of the stack graphically, using entire screen
// ----------------------------------------------------------------------------
{
    object_g obj = rt.top();
    return show(obj);
}


static coord show_x       = 0;
static coord show_y       = 0;
static coord show_delta   = 8;

static void draw_show_horizontal_arrow(coord tipx, coord tipy,
                                       coord size, coord padding,
                                       bool right, pattern fc, pattern bc)
// ----------------------------------------------------------------------------
//   Draw a left/right pointing arrow with tip at (tipx, tipy)
// ----------------------------------------------------------------------------
//   tipx, tipy: Arrow tip coordinates on screen (assuming the padding is 0)
//   size:       Arrow size in pixels
//   padding:    Extra pixels around the arrow body
//   right:      true for a right arrow, false for a left arrow
//   fc:         Foreground pattern for the arrow
//   bc:         Background pattern behind the arrow
{
    Screen.fill(right ? tipx - size - 2*padding: tipx,
                tipy - size - padding,
                right ? tipx : tipx + size + 2*padding,
                tipy + size + padding,
                bc);
    for (coord offset = 0; offset < size; offset++)
    {
        coord span = offset + 1;
        coord x = right ? tipx - offset - padding : tipx + offset + padding;
        Screen.fill(x, tipy - span + 1, x, tipy + span - 1, fc);
    }
}



static void draw_show_vertical_arrow(coord tipx, coord tipy,
                                     coord size, coord padding,
                                     bool down, pattern fc, pattern bc)
// ----------------------------------------------------------------------------
//   Draw an up/down pointing arrow with tip at (tipx, tipy)
// ----------------------------------------------------------------------------
//   tipx, tipy: Arrow tip coordinates on screen (assuming padding is 0)
//   size:       Arrow size in pixels
//   padding:    Extra pixels around the arrow body
//   down:       true for a down arrow, false for an up arrow
//   fc:         Foreground pattern for the arrow
//   bc:         Background pattern behind the arrow
{
    Screen.fill(tipx - size - padding,
                down ? tipy - size - 2*padding : tipy,
                tipx + size + padding,
                down ? tipy : tipy + size + 2*padding,
                bc);
    for (coord offset = 0; offset < size; offset++)
    {
        coord span = offset + 1;
        coord y = down ? tipy - offset - padding : tipy + offset + padding;
        Screen.fill(tipx - span + 1, y, tipx + span - 1, y, fc);
    }
}


static void draw_show_arrows(grob::pixsize width, grob::pixsize height)
// ----------------------------------------------------------------------------
//   Render navigation arrows if the grob object is bigger than the display
// ----------------------------------------------------------------------------
//   width, height: Size of the full grob being shown
{
    auto        fc            = Settings.Foreground();
    auto        bc            = pattern::gray90;
    const coord arrow_size    = 6;
    const coord arrow_padding = 2;
    coord       midx          = LCD_W / 2;
    coord       midy          = LCD_H / 2;

    if (show_x > 0)
        draw_show_horizontal_arrow(0, midy, arrow_size, arrow_padding,
                                   false, fc, bc);
    if (show_x + LCD_W < coord(width))
        draw_show_horizontal_arrow(LCD_W - 1, midy, arrow_size, arrow_padding,
                                   true, fc, bc);
    if (show_y > 0)
        draw_show_vertical_arrow(midx, 0, arrow_size, arrow_padding,
                                 false, fc, bc);
    if (show_y + LCD_H < coord(height))
        draw_show_vertical_arrow(midx, LCD_H - 1, arrow_size, arrow_padding,
                                 true, fc, bc);
}


void show_grob(grob_p graph)
// ----------------------------------------------------------------------------
//   Show a graphical object at the
// ----------------------------------------------------------------------------
{
    size    width  = graph->width();
    size    height = graph->height();

    coord   scrx   = width < LCD_W ? (LCD_W - width) / 2 : 0;
    coord   scry   = height < LCD_H ? (LCD_H - height) / 2 : 0;
    rect    r(scrx, scry, scrx + width - 1, scry + height - 1);

    if (width < LCD_W || height < LCD_H)
        Screen.fill(pattern::gray50);
#if CONFIG_COLOR
    if (pixmap_p pix = graph->as<pixmap>())
    {
        pixmap::surface s = pix->pixels();
        Screen.copy(s, r, point(show_x, show_y));
    }
    else
#endif // CONFIG_COLOR
    {
        grob::surface s = graph->pixels();
        Screen.copy(s, r, point(show_x,show_y));
    }
    draw_show_arrows(width, height);
    mark_dirty(0, 0, LCD_W-1, LCD_H-1);
    refresh_dirty();
}


int show_grob_keyboard_movements(int key, size width, size height)
// ----------------------------------------------------------------------------
//   Keyboard movements in the show command
// ----------------------------------------------------------------------------
{
    int update = false;
    switch(key)
    {
    case KEY_EXIT:
        update = -2;
        break;
    case KEY_ENTER:
    case KEY_BSP:
        update = -1;
        break;
    case KEY_SHIFT:
        show_delta = show_delta == 1 ? 8 : show_delta == 8 ? 32 : 1;
        break;
    case KEY_DOWN:
        if (width <= LCD_W)
        {
        case KEY_2:
            if (show_y + show_delta + LCD_H < coord(height))
                show_y += show_delta;
            else if (height > LCD_H)
                show_y = height - LCD_H;
            else
                show_y = 0;
            update = true;
            break;
        }
        else
        {
            case KEY_6:
                if (show_x + show_delta + LCD_W < coord(width))
                    show_x += show_delta;
                else if (width > LCD_W)
                    show_x = width - LCD_W;
                else
                    show_x = 0;
                update = true;
                break;
        }
    case KEY_UP:
        if (width <= LCD_W)
        {
        case KEY_8:
            if (show_y > show_delta)
                show_y -= show_delta;
            else
                show_y = 0;
            update = true;
            break;
        }
        else
        {
            case KEY_4:
                if (show_x > show_delta)
                    show_x -= show_delta;
                else
                    show_x = 0;
                update = true;
                break;
        }
    case KEY_SCREENSHOT:
        screenshot();
        break;
    case 0:
        break;
    }
    return update;
}


object::result show(object_r obj)
// ----------------------------------------------------------------------------
//   Draw an obejct
// ----------------------------------------------------------------------------
{
    if (obj)
    {
        grob_g graph = obj->graph(true);
        if (!graph)
        {
            if (!rt.error())
                rt.graph_does_not_fit_error();
            return object::ERROR;
        }

        save<bool> disable_user(user_display_enable, false);
        ui.draw_graphics();

        using size   = grob::pixsize;
        size width   = graph->width();
        size height  = graph->height();
        bool running = true;
        int  key     = 0;
        while (running)
        {
            show_grob(graph);

            bool update = false;
            while (!update)
            {
                // Key repeat rate
                int remains = 60;

                // Refresh screen after the requested period
                set_timer(TIMER1, remains);

                // Do not switch off if on USB power
                if (usb_powered())
                    reset_auto_off();

                // Honor auto-off while waiting, do not erase drawn image
                power_check(true);

                if (!key_empty())
                {
                    key = key_pop();
#if SIMULATOR
                    record(tests_rpl,
                           "Show cmd popped key %d, last=%d", key, last_key);
                    process_test_key(key);
#endif // SIMULATOR
                }
                int moved = show_grob_keyboard_movements(key, width, height);
                if (key && !moved)
                {
                    key = 0;
                    beep(440, 20);
                }
                update = moved;
                running = moved >= 0;

#if SIMULATOR && !WASM
                if (tests::running && test_command && key_empty())
                    process_test_commands();
#endif // SIMULATOR && !WASM
            }
        }
        show_x = 0;
        show_y = 0;
        sys_timer_disable(TIMER0);
        sys_timer_disable(TIMER1);
        redraw_lcd(true);
    }
    return object::OK;
}


COMMAND_BODY(ToGrob)
// ----------------------------------------------------------------------------
//   Convert an object to graphical form
// ----------------------------------------------------------------------------
{
    uint size = rt.stack(0)->as_uint32(0, true);
    if (!rt.error())
    {
        object_p obj = rt.stack(1);
        settings::font_id fid = size
            ? settings::font_id(size-1)
            : Settings.StackFont();
        grapher g(Settings.MaximumShowWidth(), Settings.MaximumShowHeight(),
                  fid,
                  Settings.Foreground(), Settings.Background(),
                  true, false, true);
        if (grob_p gr = obj->graph(g))
            if (rt.drop() && rt.top(gr))
                return OK;
    }
    return ERROR;
}


static object::result to_graphic(bool compatible, bool colorized)
// ----------------------------------------------------------------------------
//   Convert an object to monochrome
// ----------------------------------------------------------------------------
{
    using size = blitter::size;
    settings::SaveCompatibleGROBs scg(compatible);
    object_g obj = rt.top();
    if (!obj)
        return object::ERROR;
    (void) colorized;

#if CONFIG_COLOR
    if (pixmap_g pix = obj->as<pixmap>())
    {
        if (colorized)
            return object::OK;  // No-op
        size    w = pix->width();
        size    h = pix->height();
        grapher g(w, h);
        if (grob_p  r = g.grob(w, h))
        {
            pixmap::surface src = pix->pixels();
            grob::surface dst = r->pixels();
            for (coord y = 0; y < h; y++)
            {
                for (coord x = 0; x < w; x++)
                {
                    pixmap::surface::color c = src.pixel_color(x, y);
                    grob::surface::pattern p(c.red(), c.green(), c.blue());
                    dst.fill(x, y, x, y, p);
                }
            }
            if (rt.top(r))
                return object::OK;
        }
        return object::ERROR;
    }
#endif // CONFIG_COLOR

    if (grob_g pict = obj->as_monochrome())
    {
        if (pict->type() == (compatible ? object::ID_grob : object::ID_bitmap))
            return object::OK;          // No-op
        size    w = pict->width();
        size    h = pict->height();
        grapher g(w, h);
#if CONFIG_COLOR
        if (colorized)
        {
            if (pixmap_p r = g.pixmap(w, h))
            {
                grob::surface   src = pict->pixels();
                pixmap::surface dst = r->pixels();
                rect            area(w, h);
                dst.copy(src, area);
                if (rt.top(r))
                    return object::OK;
            }
            return object::ERROR;
        }
#endif // CONFIG_COLOR
        if (grob_p r = g.grob(w, h))
        {
            grob::surface src = pict->pixels();
            grob::surface dst = r->pixels();
            rect area(w ,h);
            dst.copy(src, area);
            if (rt.top(r))
                return object::OK;
        }
        return object::ERROR;
    }

    grapher g(Settings.MaximumShowWidth(), Settings.MaximumShowHeight(),
              Settings.ResultFont(),
              Settings.Foreground(), Settings.Background(),
              true, false, true);
    if (grob_p r = obj->graph(g))
    {
#if CONFIG_COLOR
        if (colorized)
        {
            size    w = r->width();
            size    h = r->height();
            grapher g(w, h);
            if (pixmap_p pix = g.pixmap(w, h))
            {
                grob::surface   src = r->pixels();
                pixmap::surface dst = pix->pixels();
                rect            area(w, h);
                dst.copy(src, area);
                if (rt.top(r))
                    return object::OK;
            }
            return object::ERROR;
        }
#endif // CONFIG_COLOR
        if (rt.top(r))
            return object::OK;
    }

    if (!rt.error())
        rt.graph_does_not_fit_error();
    return object::ERROR;
}


COMMAND_BODY(ToHPGrob)
// ----------------------------------------------------------------------------
//   Convert the stack object to an HP-compatible graphic object
// ----------------------------------------------------------------------------
{
    return to_graphic(true, false);
}


COMMAND_BODY(ToBitmap)
// ----------------------------------------------------------------------------
//   Convert the stack object to a new style graphic object
// ----------------------------------------------------------------------------
{
    return to_graphic(false, false);
}


#if CONFIG_COLOR
COMMAND_BODY(ToPixmap)
// ----------------------------------------------------------------------------
//   Convert the stack object to a new style graphic object
// ----------------------------------------------------------------------------
{
    return to_graphic(false, true);
}
#endif // CONFIG_COLOR


COMMAND_BODY(BlankGraphic)
// ----------------------------------------------------------------------------
//   Generate a blank graphic object of the given size (background color)
// ----------------------------------------------------------------------------
{
    using size = blitter::size;
    size w = rt.stack(1)->as_uint32(0, true);
    size h = rt.stack(0)->as_uint32(0, true);
    if (!rt.error())
    {
#if CONFIG_COLOR
        if (!Settings.CompatibleGROBs())
        {
            if (pixmap_p result = pixmap::make(w, h))
            {
                pixmap::surface s = result->pixels();
                s.fill(pixmap::pattern(Settings.Background()));
                if (rt.drop() && rt.top(result))
                    return OK;
            }
        }
#endif // CONFIG_COLOR

        if (grob_p result = Settings.CompatibleGROBs()
            ? grob::make(w, h)
            : bitmap::make(w, h))
        {
            grob::surface s = result->pixels();
            s.fill(grob::pattern(Settings.Background()));
            if (rt.drop() && rt.top(result))
                return OK;
        }
    }
    return ERROR;
}


COMMAND_BODY(BlankGrob)
// ----------------------------------------------------------------------------
//   Generate a blank HP-compatible GROB
// ----------------------------------------------------------------------------
{
    using size = blitter::size;
    size w = rt.stack(1)->as_uint32(0, true);
    size h = rt.stack(0)->as_uint32(0, true);
    if (!rt.error())
    {
        if (grob_p result = grob::make(w, h))
        {
            grob::surface s = result->pixels();
            s.fill(grob::pattern(Settings.Background()));
            if (rt.drop() && rt.top(result))
                return OK;
        }
    }
    return ERROR;
}


COMMAND_BODY(BlankBitmap)
// ----------------------------------------------------------------------------
//   Generate a blank DB48x bitmnap
// ----------------------------------------------------------------------------
{
    using size = blitter::size;
    size w = rt.stack(1)->as_uint32(0, true);
    size h = rt.stack(0)->as_uint32(0, true);
    if (!rt.error())
    {
        if (bitmap_p result = bitmap::make(w, h))
        {
            bitmap::surface s = result->pixels();
            s.fill(bitmap::pattern(Settings.Background()));
            if (rt.drop() && rt.top(result))
                return OK;
        }
    }
    return ERROR;
}


#if CONFIG_COLOR
COMMAND_BODY(BlankPixmap)
// ----------------------------------------------------------------------------
//   Generate a blank color pixmap
// ----------------------------------------------------------------------------
{
    using size = blitter::size;
    size w = rt.stack(1)->as_uint32(0, true);
    size h = rt.stack(0)->as_uint32(0, true);
    if (!rt.error())
    {
        if (pixmap_p result = pixmap::make(w, h))
        {
            pixmap::surface s = result->pixels();
            s.fill(pixmap::pattern(Settings.Background()));
            if (rt.drop() && rt.top(result))
                return OK;
        }
    }
    return ERROR;
}
#endif // CONFIG_COLOR


COMMAND_BODY(ToLCD)
// ----------------------------------------------------------------------------
//   Send a graphic object to the screen
// ----------------------------------------------------------------------------
{
    using size = blitter::size;

    size dw = display_width();
    size dh = display_height();
    object_p obj = rt.top();
    if (grob_p pict = obj->as_monochrome())
    {
        grob::surface s      = pict->pixels();
        size          width  = s.width();
        size          height = s.height();
        coord         scrx   = width < dw ? (dw - width) / 2 : 0;
        coord         scry   = height < dh ? (dh - height) / 2 : 0;
        rect          r(scrx, scry, scrx + width - 1, scry + height - 1);

        ui.draw_graphics();
        DISPLAY(display.fill(display.area(), pattern::gray50.bits);
                display.copy(s, r));
        rt.drop();
        ui.draw_dirty(r);
        ui.refresh();
        return OK;
    }
#if CONFIG_COLOR
    else if (pixmap_p pict = obj->as<pixmap>())
    {
        pixmap::surface s      = pict->pixels();
        size            width  = s.width();
        size            height = s.height();
        coord           scrx   = width < dw ? (dw - width) / 2 : 0;
        coord           scry   = height < dh ? (dh - height) / 2 : 0;
        rect            r(scrx, scry, scrx + width - 1, scry + height - 1);

        ui.draw_graphics();
        DISPLAY(display.fill(display.area(), pattern::gray50.bits);
                display.copy(s, r));
        rt.drop();
        ui.draw_dirty(r);
        refresh_dirty();
        return OK;
    }
#endif

    rt.type_error();
    return ERROR;
}


COMMAND_BODY(FromLCD)
// ----------------------------------------------------------------------------
//   Turn the screen into a graphic object
// ----------------------------------------------------------------------------
{
    using size = blitter::size;

    size width = Screen.width();
    size height = Screen.height();

#if CONFIG_COLOR
    if (pixmap_p pict = pixmap::make(width, height))
    {
        pixmap::surface s = pict->pixels();
        rect r(width, height);
        s.copy(Screen, r);
        if (rt.push(pict))
            return OK;
    }
#else
    if (bitmap_p pict = bitmap::make(width, height))
    {
        bitmap::surface s = pict->pixels();
        rect r(width, height);
        s.copy(Screen, r);
        if (rt.push(pict))
            return OK;
    }
#endif // CONFIG_COLOR

    return ERROR;
}


static void graphics_dirty(coord x1, coord y1, coord x2, coord y2, size lw)
// ----------------------------------------------------------------------------
//   Mark region as dirty with extra size
// ----------------------------------------------------------------------------
{
    if (x1 > x2)
        std::swap(x1, x2);
    if (y1 > y2)
        std::swap(y1, y2);
    size a = lw/2;
    size b = (lw+1)/2 - 1;
    ui.draw_dirty(x1 - a, y1 - a, x2 + b, y2 + b);
    ui.refresh();
}


static object::result draw_pixel(pattern color)
// ----------------------------------------------------------------------------
//   Draw a pixel on or off
// ----------------------------------------------------------------------------
{
    if (object_g p = rt.stack(0))
    {
        PlotParametersAccess ppar;
        coord                x = ppar.pair_pixel_x(p);
        coord                y = ppar.pair_pixel_y(p);
        if (!rt.error())
        {
            rt.drop();

            blitter::size lw = Settings.LineWidth();
            if (!lw)
                lw = 1;
            blitter::size a = lw/2;
            blitter::size b = (lw + 1) / 2 - 1;
            rect r(x-a, y-a, x+b, y+b);
            ui.draw_graphics();
            DISPLAY(display.fill(r, color.bits));
            ui.draw_dirty(r);
            ui.refresh();
            return object::OK;
        }
    }
    return object::ERROR;
}


COMMAND_BODY(PixOn)
// ----------------------------------------------------------------------------
//   Draw a pixel at the given coordinates
// ----------------------------------------------------------------------------
{
    return draw_pixel(Settings.Foreground());
}


COMMAND_BODY(PixOff)
// ----------------------------------------------------------------------------
//   Clear a pixel at the given coordinates
// ----------------------------------------------------------------------------
{
    return draw_pixel(Settings.Background());
}


static bool pixel_color(color &c)
// ----------------------------------------------------------------------------
//   Return the color at given coordinates
// ----------------------------------------------------------------------------
{
    if (object_g p = rt.stack(0))
    {
        PlotParametersAccess ppar;
        coord x = ppar.pair_pixel_x(p);
        coord y = ppar.pair_pixel_y(p);
        if (!rt.error())
        {
            DISPLAY(auto scol = display.pixel_color(x, y);
                    c = color(scol.red(), scol.green(), scol.blue()));            return true;
        }
    }
    return false;
}


COMMAND_BODY(PixTest)
// ----------------------------------------------------------------------------
//   Check if a pixel is on or off
// ----------------------------------------------------------------------------
{
    color c(0);
    if (pixel_color(c))
    {
        algebraic_g level = integer::make(c.red() + c.green() + c.blue());
        algebraic_g scale = integer::make(3 * 255);
        scale = level / scale;
        if (scale && rt.top(scale))
            return object::OK;
    }
    return object::ERROR;
}


COMMAND_BODY(PixColor)
// ----------------------------------------------------------------------------
//   Check the RGB components of a pixel
// ----------------------------------------------------------------------------
{
    color c(0);
    if (pixel_color(c))
    {
        algebraic_g scale = integer::make(255);
        algebraic_g red   = integer::make(c.red()) / scale;
        algebraic_g green = integer::make(c.green()) / scale;
        algebraic_g blue  = integer::make(c.blue()) / scale;
        if (scale && rt.top(+red) && rt.push(+green) && rt.push(+blue))
            return object::OK;
    }
    return object::ERROR;
}


COMMAND_BODY(Line)
// ----------------------------------------------------------------------------
//   Draw a line between the coordinates
// ----------------------------------------------------------------------------
{
    object_g p1 = rt.stack(1);
    object_g p2 = rt.stack(0);
    if (p1 && p2)
    {
        PlotParametersAccess ppar;
        coord x1 = ppar.pair_pixel_x(p1);
        coord y1 = ppar.pair_pixel_y(p1);
        coord x2 = ppar.pair_pixel_x(p2);
        coord y2 = ppar.pair_pixel_y(p2);
        if (!rt.error())
        {
            blitter::size lw   = Settings.LineWidth();
            uint64_t      fg   = Settings.Foreground();
            rt.drop(2);
            ui.draw_graphics();
            DISPLAY(display.line(x1, y1, x2, y2, lw, fg));
            graphics_dirty(x1, y1, x2, y2, lw);
            return OK;
        }
    }
    return ERROR;
}


COMMAND_BODY(Ellipse)
// ----------------------------------------------------------------------------
//   Draw an ellipse between the given coordinates
// ----------------------------------------------------------------------------
{
    object_g p1 = rt.stack(1);
    object_g p2 = rt.stack(0);
    if (p1 && p2)
    {
        PlotParametersAccess ppar;
        coord x1 = ppar.pair_pixel_x(p1);
        coord y1 = ppar.pair_pixel_y(p1);
        coord x2 = ppar.pair_pixel_x(p2);
        coord y2 = ppar.pair_pixel_y(p2);
        if (!rt.error())
        {
            blitter::size lw = Settings.LineWidth();
            rt.drop(2);
            ui.draw_graphics();
            DISPLAY(display.ellipse(x1, y1, x2, y2, lw, Settings.Foreground()));
            graphics_dirty(x1, y1, x2, y2, lw);
            return OK;
        }
    }
    return ERROR;
}


COMMAND_BODY(Circle)
// ----------------------------------------------------------------------------
//   Draw a circle between the given coordinates
// ----------------------------------------------------------------------------
{
    object_g co = rt.stack(1);
    object_g ro = rt.stack(0);
    if (co && ro)
    {
        using size = blitter::size;
        PlotParametersAccess ppar;
        size    width  = display_width();
        size    height = display_height();
        coord   x      = ppar.pair_pixel_x(co);
        coord   y      = ppar.pair_pixel_y(co);
        coord   rx     = ppar.size_adjust(ro, ppar.xmin, ppar.xmax, 2 * width);
        coord   ry     = ppar.size_adjust(ro, ppar.ymin, ppar.ymax, 2 * height);
        if (rx < 0)
            rx = -rx;
        if (ry < 0)
            ry = -ry;
        if (!rt.error())
        {
            size     lw = Settings.LineWidth();
            uint64_t fg = Settings.Foreground();
            rt.drop(2);
            coord x1 = x - rx/2;
            coord x2 = x + (rx-1)/2;
            coord y1 = y - ry/2;
            coord y2 = y + (ry-1)/2;
            ui.draw_graphics();
            DISPLAY(display.ellipse(x1, y1, x2, y2, lw, fg));
            graphics_dirty(x1, y1, x2, y2, lw);
            return OK;
        }
    }
    return ERROR;
}


COMMAND_BODY(Rect)
// ----------------------------------------------------------------------------
//   Draw a rectangle between the given coordinates
// ----------------------------------------------------------------------------
{
    object_g p1 = rt.stack(1);
    object_g p2 = rt.stack(0);
    if (p1 && p2)
    {
        PlotParametersAccess ppar;
        coord x1 = ppar.pair_pixel_x(p1);
        coord y1 = ppar.pair_pixel_y(p1);
        coord x2 = ppar.pair_pixel_x(p2);
        coord y2 = ppar.pair_pixel_y(p2);
        if (!rt.error())
        {
            blitter::size lw = Settings.LineWidth();
            uint64_t      fg = Settings.Foreground();
            rt.drop(2);
            ui.draw_graphics();
            DISPLAY(display.rectangle(x1, y1, x2, y2, lw, fg));
            ui.draw_dirty(min(x1,x2), min(y1,y2), max(x1,x2), max(y1,y2));
            ui.refresh();
            return OK;
        }
    }
    return ERROR;
}


COMMAND_BODY(RRect)
// ----------------------------------------------------------------------------
//   Draw a rounded rectangle between the given coordinates
// ----------------------------------------------------------------------------
{
    object_g p1 = rt.stack(2);
    object_g p2 = rt.stack(1);
    object_g ro = rt.stack(0);
    if (p1 && p2 && ro)
    {
        using size = blitter::size;
        PlotParametersAccess ppar;
        size  w  = display_width();
        coord x1 = ppar.pair_pixel_x(p1);
        coord y1 = ppar.pair_pixel_y(p1);
        coord x2 = ppar.pair_pixel_x(p2);
        coord y2 = ppar.pair_pixel_y(p2);
        coord r  = ppar.size_adjust(ro, ppar.xmin, ppar.xmax, 2*w);
        if (!rt.error())
        {
            blitter::size lw = Settings.LineWidth();
            uint64_t      fg = Settings.Foreground();
            rt.drop(3);
            ui.draw_graphics();
            DISPLAY(display.rounded_rectangle(x1, y1, x2, y2, r, lw, fg));
            graphics_dirty(x1, y1, x2, y2, lw);
            return OK;
        }
    }
    return ERROR;
}


COMMAND_BODY(ClLCD)
// ----------------------------------------------------------------------------
//   Clear the LCD screen before drawing stuff on it
// ----------------------------------------------------------------------------
{
    ui.draw_graphics(true);
    ui.refresh();
    return OK;
}


COMMAND_BODY(Clip)
// ----------------------------------------------------------------------------
//   Set the clipping rectangle
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.pop())
    {
        if (list_p parms = top->as<list>())
        {
            rect    clip(0, 0, 1<<30, 1<<30);
            uint    index = 0;
            for (object_p parm : *parms)
            {
                coord arg = parm->as_int32(0, true);
                if (rt.error())
                    return ERROR;
                switch(index++)
                {
                case 0: clip.x1 = arg; break;
                case 1: clip.y1 = arg; break;
                case 2: clip.x2 = arg; break;
                case 3: clip.y2 = arg; break;
                default:        rt.value_error(); return ERROR;
                }
            }
            if (user_display())
                grob::clip = clip;
            else
                Screen.clip(clip);
            return OK;
        }
        else
        {
            rt.type_error();
        }
    }
    return ERROR;
}


COMMAND_BODY(CurrentClip)
// ----------------------------------------------------------------------------
//   Retuyrn the current clipping rectangle
// ----------------------------------------------------------------------------
{
    rect      clip = user_display() ? grob::clip : Screen.clip();
    integer_g x1   = integer::make(clip.x1);
    integer_g y1   = integer::make(clip.y1);
    integer_g x2   = integer::make(clip.x2);
    integer_g y2   = integer::make(clip.y2);
    if (x1 && y1 && x2 && y2)
    {
        list_g obj = list::make(x1, y1, x2, y2);
        if (obj && rt.push(+obj))
            return OK;
    }
    return ERROR;
}


COMMAND_BODY(Freeze)
// ----------------------------------------------------------------------------
//   Set the freeze flags
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.pop())
    {
        uint flags = top->as_uint32(0, true);
        if (!rt.error())
            if (ui.freeze(flags))
                return OK;
    }
    return ERROR;
}


COMMAND_BODY(Header)
// ----------------------------------------------------------------------------
//   Set the current header
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
        if (object_p name = static_object(ID_Header))
            if (directory::store_here(name, obj))
                if (rt.drop())
                    return OK;
    return ERROR;
}



// ============================================================================
//
//   Graphic objects (grob)
//
// ============================================================================

COMMAND_BODY(GXor)
// ----------------------------------------------------------------------------
//   Graphic xor
// ----------------------------------------------------------------------------
{
    return grob::command(blitter::blitop_xor);
}


COMMAND_BODY(GOr)
// ----------------------------------------------------------------------------
//   Graphic or
// ----------------------------------------------------------------------------
{
    return grob::command(blitter::blitop_or);
}


COMMAND_BODY(GAnd)
// ----------------------------------------------------------------------------
//   Graphic and
// ----------------------------------------------------------------------------
{
    return grob::command(blitter::blitop_and);
}


COMMAND_BODY(Pict)
// ----------------------------------------------------------------------------
//   Reference to the graphic display
// ----------------------------------------------------------------------------
{
    rt.push(static_object(ID_Pict));
    return OK;
}


COMMAND_BODY(GraphicAppend)
// ----------------------------------------------------------------------------
//  Append two graphic objects side by side
// ----------------------------------------------------------------------------
{
    return grob::command([](grob_r y, grob_r x)
    {
        grapher g;
        return expression::prefix(g, 0, y, 0, x);
    });
}


COMMAND_BODY(GraphicStack)
// ----------------------------------------------------------------------------
//   Append two graphic objects on top of one another
// ----------------------------------------------------------------------------
{
    return grob::command([](grob_r y, grob_r x) -> grob_p
    {
        blitter::size xh = x->height();
        blitter::size xw = x->width();
        blitter::size yh = y->height();
        blitter::size yw = y->width();
        blitter::size gw = std::max(xw, yw);
        blitter::size gh = xh + yh;
        grapher g;
        grob_g  result = g.grob(gw, gh);
        if (!result)
            return nullptr;

        grob::surface xs = x->pixels();
        grob::surface ys = y->pixels();
        grob::surface rs = result->pixels();

        rs.fill(0, 0, gw, gh, g.background);
        rs.copy(ys, (gw - yw) / 2, 0);
        rs.copy(xs, (gw - xw) / 2, yh);

        return result;
    });
}


COMMAND_BODY(GraphicRatio)
// ----------------------------------------------------------------------------
//  Compute a ratio betwen two graphic objects
// ----------------------------------------------------------------------------
{
    return grob::command([](grob_r y, grob_r x)
    {
        grapher g;
        return expression::ratio(g, y, x);
    });
}


COMMAND_BODY(GraphicSubscript)
// ----------------------------------------------------------------------------
//  Position a graphic as a subscript
// ----------------------------------------------------------------------------
{
    return grob::command([](grob_r y, grob_r x)
    {
        grapher g;
        return expression::suscript(g, 0, y, 0, x, -1);
    });
}


COMMAND_BODY(GraphicExponent)
// ----------------------------------------------------------------------------
//   Position a graphic as an exponent
// ----------------------------------------------------------------------------
{
    return grob::command([](grob_r y, grob_r x)
    {
        grapher g;
        return expression::suscript(g, 0, y, 0, x, 1);
    });
}


COMMAND_BODY(GraphicRoot)
// ----------------------------------------------------------------------------
//  Put a graphic inside a square root sign
// ----------------------------------------------------------------------------
{
    return grob::command([](grob_r x)
    {
        grapher g;
        return expression::root(g, x);
    });
}


COMMAND_BODY(GraphicParentheses)
// ----------------------------------------------------------------------------
//  Put a graphic inside parentheses
// ----------------------------------------------------------------------------
{
    return grob::command([](grob_r x)
    {
        grapher g;
        return expression::parentheses(g, x);
    });
}


COMMAND_BODY(GraphicNorm)
// ----------------------------------------------------------------------------
//   Draw a norm around the graphic object
// ----------------------------------------------------------------------------
{
    return grob::command([](grob_r x)
    {
        grapher g;
        return expression::abs_norm(g, x);
    });
}


COMMAND_BODY(GraphicSum)
// ----------------------------------------------------------------------------
//  Compute a sum sign for the given height
// ----------------------------------------------------------------------------
{
    return grob::command([](blitter::size h)
    {
        grapher g;
        return expression::sum(g, h);
    });
}


COMMAND_BODY(GraphicProduct)
// ----------------------------------------------------------------------------
//   Compute a product sign for the given height
// ----------------------------------------------------------------------------
{
    return grob::command([](blitter::size h)
    {
        grapher g;
        return expression::product(g, h);
    });
}


COMMAND_BODY(GraphicIntegral)
// ----------------------------------------------------------------------------
//   Compute an integral sign for the given height
// ----------------------------------------------------------------------------
{
    return grob::command([](blitter::size h)
    {
        grapher g;
        return expression::integral(g, h);
    });
}


static object::result set_ppar_corner(bool max)
// ----------------------------------------------------------------------------
//   Shared code for PMin and PMax
// ----------------------------------------------------------------------------
{
    object_p corner = rt.top();
    if (corner->is_complex())
    {
        if (rectangular_g pos = complex_p(corner)->as_rectangular())
        {
            PlotParametersAccess ppar;
            (max ? ppar.xmax : ppar.xmin) = pos->re();
            (max ? ppar.ymax : ppar.ymin) = pos->im();
            if (ppar.write())
            {
                rt.drop();
                return object::OK;
            }
        }
        else
        {
            rt.type_error();
        }
    }
    return object::ERROR;
}


COMMAND_BODY(PlotMin)
// ----------------------------------------------------------------------------
//   Set the plot min factor in the plot parameters
// ----------------------------------------------------------------------------
{
    return set_ppar_corner(false);
}


COMMAND_BODY(PlotMax)
// ----------------------------------------------------------------------------
//  Set the plot max factor int he lot parameters
// ----------------------------------------------------------------------------
{
    return set_ppar_corner(true);
}


static object::result set_ppar_range(bool y)
// ----------------------------------------------------------------------------
//   Shared code for XRange and YRange
// ----------------------------------------------------------------------------
{
    object_p min = rt.stack(1);
    object_p max = rt.stack(0);
    if (min->is_real() && max->is_real())
    {
        PlotParametersAccess ppar;
        (y ? ppar.ymin : ppar.xmin) = algebraic_p(min);
        (y ? ppar.ymax : ppar.xmax) = algebraic_p(max);
        if (ppar.write())
        {
            rt.drop(2);
            return object::OK;
        }
    }
    else
    {
        rt.type_error();
    }
    return object::ERROR;
}


COMMAND_BODY(XRange)
// ----------------------------------------------------------------------------
//   Select the horizontal range for plotting
// ----------------------------------------------------------------------------
{
    return set_ppar_range(false);
}


COMMAND_BODY(YRange)
// ----------------------------------------------------------------------------
//   Select the vertical range for plotting
// ----------------------------------------------------------------------------
{
    return set_ppar_range(true);
}


static object::result set_ppar_scale(bool y)
// ----------------------------------------------------------------------------
//   Shared code for XScale and YScale
// ----------------------------------------------------------------------------
{
    object_p scale = rt.top();
    if (scale->is_real())
    {
        PlotParametersAccess ppar;
        algebraic_g s = algebraic_p(scale);
        algebraic_g &min = y ? ppar.ymin : ppar.xmin;
        algebraic_g &max = y ? ppar.ymax : ppar.xmax;
        algebraic_g two = integer::make(2);
        algebraic_g center = (min + max) / two;
        algebraic_g width = (max - min) / two;
        min = center - width * s;
        max = center + width * s;
        if (ppar.write())
        {
            rt.drop();
            return object::OK;
        }
    }
    else
    {
        rt.type_error();
    }
    return object::ERROR;
}


COMMAND_BODY(XScale)
// ----------------------------------------------------------------------------
//   Adjust the horizontal scale
// ----------------------------------------------------------------------------
{
    return set_ppar_scale(false);
}


COMMAND_BODY(YScale)
// ----------------------------------------------------------------------------
//   Adjust the vertical scale
// ----------------------------------------------------------------------------
{
    return set_ppar_scale(true);
}


COMMAND_BODY(Scale)
// ----------------------------------------------------------------------------
//  Adjust both horizontal and vertical scale
// ----------------------------------------------------------------------------
{
    if (object::result err = set_ppar_scale(true))
        return err;
    if (object::result err = set_ppar_scale(false))
        return err;
    return OK;
}


COMMAND_BODY(Center)
// ----------------------------------------------------------------------------
//   Center around the given coordinate
// ----------------------------------------------------------------------------
{
    object_p center = rt.top();
    if (center->is_complex())
    {
        if (rectangular_g pos = complex_p(center)->as_rectangular())
        {
            PlotParametersAccess ppar;
            algebraic_g          two = integer::make(2);
            algebraic_g          w   = (ppar.xmax - ppar.xmin) / two;
            algebraic_g          h   = (ppar.ymax - ppar.ymin) / two;
            algebraic_g          cx  = pos->re();
            algebraic_g          cy  = pos->im();
            ppar.xmin = cx - w;
            ppar.xmax = cx + w;
            ppar.ymin = cy - h;
            ppar.ymax = cy + h;
            if (ppar.write())
            {
                rt.drop();
                return object::OK;
            }
        }
        else
        {
            rt.type_error();
        }
    }
    return object::ERROR;
}


COMMAND_BODY(Res)
// ----------------------------------------------------------------------------
//   Setup the resolution
// ----------------------------------------------------------------------------
{
    object_p res = rt.top();
    if (res && res->is_real())
    {
        PlotParametersAccess ppar;
        ppar.resolution = algebraic_p(res);
        if (ppar.write())
        {
            rt.drop();
            return OK;
        }
    }
    else
    {
        rt.type_error();
    }
    return ERROR;
}


static bool set_ppar_variable(bool dependent)
// ----------------------------------------------------------------------------
//   Shared code for Indep and Depnd
// ----------------------------------------------------------------------------
//   These commands are special in that they take a variable number of args
//   Four forms take 1 argument (name or list)
//   The last form takes 2 real numbers from the stack
{
    uint depth = rt.depth();
    if (!depth)
    {
        rt.missing_argument_error();
        return false;
    }

    PlotParametersAccess ppar;
    if (depth >= 2)
    {
        object_p arg  = rt.stack(0);
        object_p arg2 = rt.stack(1);
        if (arg->is_real() && arg2->is_real())
        {
            if (!rt.args(2))
                return false;
            algebraic_g min = algebraic_p(arg2);
            algebraic_g max = algebraic_p(arg);
            if (dependent)
            {
                ppar.ymin = min;
                ppar.ymax = max;
            }
            else
            {
                ppar.imin = min;
                ppar.imax = max;
            }
            if (ppar.write() && rt.drop(2))
                return true;
            return false;
        }
    }

    if (!rt.args(1))
        return false;

    object_p arg = rt.top();
    symbol_p    name = nullptr;
    algebraic_p min  = nullptr;
    algebraic_p max  = nullptr;
    if (list_p specs = arg->as<list>())
    {
        for (auto it : *specs)
        {
            if (min && max)
                break;
            if (symbol_p sym = it->as_quoted<symbol>())
            {
                if (name || min || max)
                    rt.type_error();
                else
                    name = sym;
            }
            else if (algebraic_p rv = it->as_real())
            {
                if (!min)
                    min = rv;
                else if (!max)
                    max = rv;
                else
                    break;
            }
            else
            {
                rt.type_error();
                return false;
            }
        }
    }
    else if (symbol_p sym = arg->as_quoted<symbol>())
    {
        name = sym;
    }
    else
    {
        rt.type_error();
        return false;
    }

    if (min && !max)
    {
        rt.domain_error();
        return false;
    }
    if (dependent)
    {
        if (name)
            ppar.dependent = name;
        if (min)
            ppar.dmin = min;
        if (max)
            ppar.dmax = max;
    }
    else
    {
        if (name)
            ppar.independent = name;
        if (min)
            ppar.imin = min;
        if (max)
            ppar.imax = max;
    }
    return ppar.write() && rt.drop();
}


COMMAND_BODY(Indep)
// ----------------------------------------------------------------------------
//   Set the independent variable in the plot parameters
// ----------------------------------------------------------------------------
{
    return set_ppar_variable(false) ? OK : ERROR;
}


COMMAND_BODY(Depnd)
// ----------------------------------------------------------------------------
//   Set the dependent variable in the plot parameters
// ----------------------------------------------------------------------------
{
    return set_ppar_variable(true) ? OK : ERROR;
}


COMMAND_BODY(Gray)
// ----------------------------------------------------------------------------
//   Create a pattern from a gray level
// ----------------------------------------------------------------------------
{
    algebraic_g gray = algebraic_p(rt.top());
    if (gray->is_real())
    {
        gray = gray * integer::make(255);
        uint level = gray->as_uint32(0, true);
        if (rt.error())
            return ERROR;
        if (level > 255)
            level = 255;
        pattern pat = pattern(level, level, level);
#if CONFIG_FIXED_BASED_OBJECTS
        integer_p bits = rt.make<hex_integer>(pat.bits);
#else
        integer_p bits = rt.make<based_integer>(pat.bits);
#endif
        if (bits && rt.top(bits))
            return OK;
    }
    else
    {
        rt.type_error();
    }
    return ERROR;
}


static void hsv_to_rgb(uint hue, uint saturation, uint value,
                       uint &red, uint &green, uint &blue)
// ----------------------------------------------------------------------------
//   Ad-hoc conversion from HSV (color wheel) to RGB
// ----------------------------------------------------------------------------
{
    // Normalize hue to 0..359
    hue %= 360;
    if (hue < 0)
        hue += 360;

    // Normalize hue to [0, 6)
    float h = hue / 60.0f;
    int   i = int(floor(h));
    float f = h - i;

    // Clamp saturation and value to 0..1
    float v = std::max(0.0f, std::min(1.0f, value / 255.0f));
    float s = std::max(0.0f, std::min(1.0f, saturation / 255.0f));

    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    float rf, gf, bf;
    switch (i % 6)
    {
    case 0: rf = v; gf = t; bf = p; break;
    case 1: rf = q; gf = v; bf = p; break;
    case 2: rf = p; gf = v; bf = t; break;
    case 3: rf = p; gf = q; bf = v; break;
    case 4: rf = t; gf = p; bf = v; break;
    case 5: rf = v; gf = p; bf = q; break;
    default: rf = gf = bf = 0.0f; break;
    }

    red = uint(rf * 255.0f);
    green = uint(gf * 255.0f);
    blue = uint(bf * 255.0f);
}


static bool color_pattern(bool hsv)
// ----------------------------------------------------------------------------
//   Build a color pattern from 3 values on the stack
// ----------------------------------------------------------------------------
{
    algebraic_g red   = algebraic_p(rt.stack(2)); // hue in hsv mode
    algebraic_g green = algebraic_p(rt.stack(1)); // sat in hsv mode
    algebraic_g blue  = algebraic_p(rt.stack(0)); // val in hsv mode
    if (red->is_real() && green->is_real() && blue->is_real())
    {
        algebraic_g scale = integer::make(255);
        red = red * scale;
        green = green * scale;
        blue = blue * scale;
        uint rl = red->as_uint32(0, true);
        uint gl = green->as_uint32(0, true);
        uint bl = blue->as_uint32(0, true);
        if (rt.error())
            return false;
        if (rl > 255)
            rl = 255;
        if (gl > 255)
            gl = 255;
        if (bl > 255)
            bl = 255;
        if (hsv)
            hsv_to_rgb(rl, gl, bl, rl, gl, bl);
        pattern pat = pattern(rl, gl, bl);
#if CONFIG_FIXED_BASED_OBJECTS
        integer_p bits = rt.make<hex_integer>(pat.bits);
#else
        integer_p bits = rt.make<based_integer>(pat.bits);
#endif
        if (bits && rt.drop(2) && rt.top(bits))
            return true;
    }
    else
    {
        rt.type_error();
    }
    return false;
}


COMMAND_BODY(RGB)
// ----------------------------------------------------------------------------
//   Create a pattern from RGB levels
// ----------------------------------------------------------------------------
{
    return color_pattern(false) ? OK : ERROR;
}


COMMAND_BODY(HSV)
// ----------------------------------------------------------------------------
//   Create a pattern from HSV levels
// ----------------------------------------------------------------------------
{
    return color_pattern(true) ? OK : ERROR;
}


COMMAND_BODY(Color)
// ----------------------------------------------------------------------------
//   Create a pattern from color object levels
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        pattern pat = color_pattern(obj);
#if CONFIG_FIXED_BASED_OBJECTS
        integer_p bits = rt.make<hex_integer>(pat.bits);
#else
        integer_p bits = rt.make<based_integer>(pat.bits);
#endif
        if (bits && rt.top(bits))
            return OK;
    }
    return ERROR;
}



// ============================================================================
//
//   Indirect display
//
// ============================================================================

bool user_display_enable = true;


grob_p user_display()
// ----------------------------------------------------------------------------
//   Return the GROB in the `Pict` variable if any, or nullptr
// ----------------------------------------------------------------------------
{
    if (user_display_enable)
    {
        object_p pict = object::static_object(object::ID_Pict);
        if (object_p obj = directory::recall_all(pict, false))
            if (obj->is_graph())
                return grob_p(obj);
    }
    return nullptr;
}


blitter::size  display_width()
// ----------------------------------------------------------------------------
//   Return width of display
// ----------------------------------------------------------------------------
{
    if (grob_p pict = user_display())
        return pict->width();
    return Screen.width();
}


blitter::size  display_height()
// ----------------------------------------------------------------------------
//   Return height of display
// ----------------------------------------------------------------------------
{
    if (grob_p pict = user_display())
        return pict->height();
    return Screen.height();
}



// ============================================================================
//
//   Color management
//
// ============================================================================
//   Color objects are converted as follows:
//   - True:            Foreground color
//   - False:           Background color
//   - Based:           64-bit bit pattern
//   - Real:            Grayscale value
//   - Complex:         HSV with saturation at 1
//   - List/array:      RGB value (positive), HSV (negative)
//   - Text/Symbol:     Named color

static const struct { uint r; uint g; uint b; cstring name; } color_names[] =
// ----------------------------------------------------------------------------
//   Color table (from Emacs rgb.txt file)
// ----------------------------------------------------------------------------
{
    { 255, 250, 250, "snow" },
    { 248, 248, 255, "ghostwhite" },
    { 245, 245, 245, "whitesmoke" },
    { 220, 220, 220, "gainsboro" },
    { 255, 250, 240, "floralwhite" },
    { 253, 245, 230, "oldlace" },
    { 250, 240, 230, "linen" },
    { 250, 235, 215, "antiquewhite" },
    { 255, 239, 213, "papayawhip" },
    { 255, 235, 205, "blanchedalmond" },
    { 255, 228, 196, "bisque" },
    { 255, 218, 185, "peachpuff" },
    { 255, 222, 173, "navajowhite" },
    { 255, 228, 181, "moccasin" },
    { 255, 248, 220, "cornsilk" },
    { 255, 255, 240, "ivory" },
    { 255, 250, 205, "lemonchiffon" },
    { 255, 245, 238, "seashell" },
    { 240, 255, 240, "honeydew" },
    { 245, 255, 250, "mintcream" },
    { 240, 255, 255, "azure" },
    { 240, 248, 255, "aliceblue" },
    { 230, 230, 250, "lavender" },
    { 255, 240, 245, "lavenderblush" },
    { 255, 228, 225, "mistyrose" },
    { 255, 255, 255, "white" },
    {   0,   0,   0, "black" },
    {  47,  79,  79, "darkslategray" },
    { 105, 105, 105, "dimgray" },
    { 112, 128, 144, "slategray" },
    { 119, 136, 153, "lightslategray" },
    { 190, 190, 190, "gray" },
    { 211, 211, 211, "lightgray" },
    {  25,  25, 112, "midnightblue" },
    {   0,   0, 128, "navy" },
    {   0,   0, 128, "navyblue" },
    { 100, 149, 237, "cornflowerblue" },
    {  72,  61, 139, "darkslateblue" },
    { 106,  90, 205, "slateblue" },
    { 123, 104, 238, "mediumslateblue" },
    { 132, 112, 255, "lightslateblue" },
    {   0,   0, 205, "mediumblue" },
    {  65, 105, 225, "royalblue" },
    {   0,   0, 255, "blue" },
    {  30, 144, 255, "dodgerblue" },
    {   0, 191, 255, "deepskyblue" },
    { 135, 206, 235, "skyblue" },
    { 135, 206, 250, "lightskyblue" },
    {  70, 130, 180, "steelblue" },
    { 176, 196, 222, "lightsteelblue" },
    { 173, 216, 230, "lightblue" },
    { 176, 224, 230, "powderblue" },
    { 175, 238, 238, "paleturquoise" },
    {   0, 206, 209, "darkturquoise" },
    {  72, 209, 204, "mediumturquoise" },
    {  64, 224, 208, "turquoise" },
    {   0, 255, 255, "cyan" },
    { 224, 255, 255, "lightcyan" },
    {  95, 158, 160, "cadetblue" },
    { 102, 205, 170, "mediumaquamarine" },
    { 127, 255, 212, "aquamarine" },
    {   0, 100,   0, "darkgreen" },
    {  85, 107,  47, "darkolivegreen" },
    { 143, 188, 143, "darkseagreen" },
    {  46, 139,  87, "seagreen" },
    {  60, 179, 113, "mediumseagreen" },
    {  32, 178, 170, "lightseagreen" },
    { 152, 251, 152, "palegreen" },
    {   0, 255, 127, "springgreen" },
    { 124, 252,   0, "lawngreen" },
    {   0, 255,   0, "green" },
    { 127, 255,   0, "chartreuse" },
    {   0, 250, 154, "mediumspringgreen" },
    { 173, 255,  47, "greenyellow" },
    {  50, 205,  50, "limegreen" },
    { 154, 205,  50, "yellowgreen" },
    {  34, 139,  34, "forestgreen" },
    { 107, 142,  35, "olivedrab" },
    { 189, 183, 107, "darkkhaki" },
    { 240, 230, 140, "khaki" },
    { 238, 232, 170, "palegoldenrod" },
    { 250, 250, 210, "lightgoldenrodyellow" },
    { 255, 255, 224, "lightyellow" },
    { 255, 255,   0, "yellow" },
    { 255, 215,   0, "gold" },
    { 238, 221, 130, "lightgoldenrod" },
    { 218, 165,  32, "goldenrod" },
    { 184, 134,  11, "darkgoldenrod" },
    { 188, 143, 143, "rosybrown" },
    { 205,  92,  92, "indianred" },
    { 139,  69,  19, "saddlebrown" },
    { 160,  82,  45, "sienna" },
    { 205, 133,  63, "peru" },
    { 222, 184, 135, "burlywood" },
    { 245, 245, 220, "beige" },
    { 245, 222, 179, "wheat" },
    { 244, 164,  96, "sandybrown" },
    { 210, 180, 140, "tan" },
    { 210, 105,  30, "chocolate" },
    { 178,  34,  34, "firebrick" },
    { 165,  42,  42, "brown" },
    { 233, 150, 122, "darksalmon" },
    { 250, 128, 114, "salmon" },
    { 255, 160, 122, "lightsalmon" },
    { 255, 165,   0, "orange" },
    { 255, 140,   0, "darkorange" },
    { 255, 127,  80, "coral" },
    { 240, 128, 128, "lightcoral" },
    { 255,  99,  71, "tomato" },
    { 255,  69,   0, "orangered" },
    { 255,   0,   0, "red" },
    { 255, 105, 180, "hotpink" },
    { 255,  20, 147, "deeppink" },
    { 255, 192, 203, "pink" },
    { 255, 182, 193, "lightpink" },
    { 219, 112, 147, "palevioletred" },
    { 176,  48,  96, "maroon" },
    { 199,  21, 133, "mediumvioletred" },
    { 208,  32, 144, "violetred" },
    { 255,   0, 255, "magenta" },
    { 238, 130, 238, "violet" },
    { 221, 160, 221, "plum" },
    { 218, 112, 214, "orchid" },
    { 186,  85, 211, "mediumorchid" },
    { 153,  50, 204, "darkorchid" },
    { 148,   0, 211, "darkviolet" },
    { 138,  43, 226, "blueviolet" },
    { 160,  32, 240, "purple" },
    { 147, 112, 219, "mediumpurple" },
    { 216, 191, 216, "thistle" },
    { 255, 250, 250, "snow1" },
    { 238, 233, 233, "snow2" },
    { 205, 201, 201, "snow3" },
    { 139, 137, 137, "snow4" },
    { 255, 245, 238, "seashell1" },
    { 238, 229, 222, "seashell2" },
    { 205, 197, 191, "seashell3" },
    { 139, 134, 130, "seashell4" },
    { 255, 239, 219, "antiquewhite1" },
    { 238, 223, 204, "antiquewhite2" },
    { 205, 192, 176, "antiquewhite3" },
    { 139, 131, 120, "antiquewhite4" },
    { 255, 228, 196, "bisque1" },
    { 238, 213, 183, "bisque2" },
    { 205, 183, 158, "bisque3" },
    { 139, 125, 107, "bisque4" },
    { 255, 218, 185, "peachpuff1" },
    { 238, 203, 173, "peachpuff2" },
    { 205, 175, 149, "peachpuff3" },
    { 139, 119, 101, "peachpuff4" },
    { 255, 222, 173, "navajowhite1" },
    { 238, 207, 161, "navajowhite2" },
    { 205, 179, 139, "navajowhite3" },
    { 139, 121,  94, "navajowhite4" },
    { 255, 250, 205, "lemonchiffon1" },
    { 238, 233, 191, "lemonchiffon2" },
    { 205, 201, 165, "lemonchiffon3" },
    { 139, 137, 112, "lemonchiffon4" },
    { 255, 248, 220, "cornsilk1" },
    { 238, 232, 205, "cornsilk2" },
    { 205, 200, 177, "cornsilk3" },
    { 139, 136, 120, "cornsilk4" },
    { 255, 255, 240, "ivory1" },
    { 238, 238, 224, "ivory2" },
    { 205, 205, 193, "ivory3" },
    { 139, 139, 131, "ivory4" },
    { 240, 255, 240, "honeydew1" },
    { 224, 238, 224, "honeydew2" },
    { 193, 205, 193, "honeydew3" },
    { 131, 139, 131, "honeydew4" },
    { 255, 240, 245, "lavenderblush1" },
    { 238, 224, 229, "lavenderblush2" },
    { 205, 193, 197, "lavenderblush3" },
    { 139, 131, 134, "lavenderblush4" },
    { 255, 228, 225, "mistyrose1" },
    { 238, 213, 210, "mistyrose2" },
    { 205, 183, 181, "mistyrose3" },
    { 139, 125, 123, "mistyrose4" },
    { 240, 255, 255, "azure1" },
    { 224, 238, 238, "azure2" },
    { 193, 205, 205, "azure3" },
    { 131, 139, 139, "azure4" },
    { 131, 111, 255, "slateblue1" },
    { 122, 103, 238, "slateblue2" },
    { 105,  89, 205, "slateblue3" },
    {  71,  60, 139, "slateblue4" },
    {  72, 118, 255, "royalblue1" },
    {  67, 110, 238, "royalblue2" },
    {  58,  95, 205, "royalblue3" },
    {  39,  64, 139, "royalblue4" },
    {   0,   0, 255, "blue1" },
    {   0,   0, 238, "blue2" },
    {   0,   0, 205, "blue3" },
    {   0,   0, 139, "blue4" },
    {  30, 144, 255, "dodgerblue1" },
    {  28, 134, 238, "dodgerblue2" },
    {  24, 116, 205, "dodgerblue3" },
    {  16,  78, 139, "dodgerblue4" },
    {  99, 184, 255, "steelblue1" },
    {  92, 172, 238, "steelblue2" },
    {  79, 148, 205, "steelblue3" },
    {  54, 100, 139, "steelblue4" },
    {   0, 191, 255, "deepskyblue1" },
    {   0, 178, 238, "deepskyblue2" },
    {   0, 154, 205, "deepskyblue3" },
    {   0, 104, 139, "deepskyblue4" },
    { 135, 206, 255, "skyblue1" },
    { 126, 192, 238, "skyblue2" },
    { 108, 166, 205, "skyblue3" },
    {  74, 112, 139, "skyblue4" },
    { 176, 226, 255, "lightskyblue1" },
    { 164, 211, 238, "lightskyblue2" },
    { 141, 182, 205, "lightskyblue3" },
    {  96, 123, 139, "lightskyblue4" },
    { 198, 226, 255, "slategray1" },
    { 185, 211, 238, "slategray2" },
    { 159, 182, 205, "slategray3" },
    { 108, 123, 139, "slategray4" },
    { 202, 225, 255, "lightsteelblue1" },
    { 188, 210, 238, "lightsteelblue2" },
    { 162, 181, 205, "lightsteelblue3" },
    { 110, 123, 139, "lightsteelblue4" },
    { 191, 239, 255, "lightblue1" },
    { 178, 223, 238, "lightblue2" },
    { 154, 192, 205, "lightblue3" },
    { 104, 131, 139, "lightblue4" },
    { 224, 255, 255, "lightcyan1" },
    { 209, 238, 238, "lightcyan2" },
    { 180, 205, 205, "lightcyan3" },
    { 122, 139, 139, "lightcyan4" },
    { 187, 255, 255, "paleturquoise1" },
    { 174, 238, 238, "paleturquoise2" },
    { 150, 205, 205, "paleturquoise3" },
    { 102, 139, 139, "paleturquoise4" },
    { 152, 245, 255, "cadetblue1" },
    { 142, 229, 238, "cadetblue2" },
    { 122, 197, 205, "cadetblue3" },
    {  83, 134, 139, "cadetblue4" },
    {   0, 245, 255, "turquoise1" },
    {   0, 229, 238, "turquoise2" },
    {   0, 197, 205, "turquoise3" },
    {   0, 134, 139, "turquoise4" },
    {   0, 255, 255, "cyan1" },
    {   0, 238, 238, "cyan2" },
    {   0, 205, 205, "cyan3" },
    {   0, 139, 139, "cyan4" },
    { 151, 255, 255, "darkslategray1" },
    { 141, 238, 238, "darkslategray2" },
    { 121, 205, 205, "darkslategray3" },
    {  82, 139, 139, "darkslategray4" },
    { 127, 255, 212, "aquamarine1" },
    { 118, 238, 198, "aquamarine2" },
    { 102, 205, 170, "aquamarine3" },
    {  69, 139, 116, "aquamarine4" },
    { 193, 255, 193, "darkseagreen1" },
    { 180, 238, 180, "darkseagreen2" },
    { 155, 205, 155, "darkseagreen3" },
    { 105, 139, 105, "darkseagreen4" },
    {  84, 255, 159, "seagreen1" },
    {  78, 238, 148, "seagreen2" },
    {  67, 205, 128, "seagreen3" },
    {  46, 139,  87, "seagreen4" },
    { 154, 255, 154, "palegreen1" },
    { 144, 238, 144, "palegreen2" },
    { 124, 205, 124, "palegreen3" },
    {  84, 139,  84, "palegreen4" },
    {   0, 255, 127, "springgreen1" },
    {   0, 238, 118, "springgreen2" },
    {   0, 205, 102, "springgreen3" },
    {   0, 139,  69, "springgreen4" },
    {   0, 255,   0, "green1" },
    {   0, 238,   0, "green2" },
    {   0, 205,   0, "green3" },
    {   0, 139,   0, "green4" },
    { 127, 255,   0, "chartreuse1" },
    { 118, 238,   0, "chartreuse2" },
    { 102, 205,   0, "chartreuse3" },
    {  69, 139,   0, "chartreuse4" },
    { 192, 255,  62, "olivedrab1" },
    { 179, 238,  58, "olivedrab2" },
    { 154, 205,  50, "olivedrab3" },
    { 105, 139,  34, "olivedrab4" },
    { 202, 255, 112, "darkolivegreen1" },
    { 188, 238, 104, "darkolivegreen2" },
    { 162, 205,  90, "darkolivegreen3" },
    { 110, 139,  61, "darkolivegreen4" },
    { 255, 246, 143, "khaki1" },
    { 238, 230, 133, "khaki2" },
    { 205, 198, 115, "khaki3" },
    { 139, 134,  78, "khaki4" },
    { 255, 236, 139, "lightgoldenrod1" },
    { 238, 220, 130, "lightgoldenrod2" },
    { 205, 190, 112, "lightgoldenrod3" },
    { 139, 129,  76, "lightgoldenrod4" },
    { 255, 255, 224, "lightyellow1" },
    { 238, 238, 209, "lightyellow2" },
    { 205, 205, 180, "lightyellow3" },
    { 139, 139, 122, "lightyellow4" },
    { 255, 255,   0, "yellow1" },
    { 238, 238,   0, "yellow2" },
    { 205, 205,   0, "yellow3" },
    { 139, 139,   0, "yellow4" },
    { 255, 215,   0, "gold1" },
    { 238, 201,   0, "gold2" },
    { 205, 173,   0, "gold3" },
    { 139, 117,   0, "gold4" },
    { 255, 193,  37, "goldenrod1" },
    { 238, 180,  34, "goldenrod2" },
    { 205, 155,  29, "goldenrod3" },
    { 139, 105,  20, "goldenrod4" },
    { 255, 185,  15, "darkgoldenrod1" },
    { 238, 173,  14, "darkgoldenrod2" },
    { 205, 149,  12, "darkgoldenrod3" },
    { 139, 101,   8, "darkgoldenrod4" },
    { 255, 193, 193, "rosybrown1" },
    { 238, 180, 180, "rosybrown2" },
    { 205, 155, 155, "rosybrown3" },
    { 139, 105, 105, "rosybrown4" },
    { 255, 106, 106, "indianred1" },
    { 238,  99,  99, "indianred2" },
    { 205,  85,  85, "indianred3" },
    { 139,  58,  58, "indianred4" },
    { 255, 130,  71, "sienna1" },
    { 238, 121,  66, "sienna2" },
    { 205, 104,  57, "sienna3" },
    { 139,  71,  38, "sienna4" },
    { 255, 211, 155, "burlywood1" },
    { 238, 197, 145, "burlywood2" },
    { 205, 170, 125, "burlywood3" },
    { 139, 115,  85, "burlywood4" },
    { 255, 231, 186, "wheat1" },
    { 238, 216, 174, "wheat2" },
    { 205, 186, 150, "wheat3" },
    { 139, 126, 102, "wheat4" },
    { 255, 165,  79, "tan1" },
    { 238, 154,  73, "tan2" },
    { 205, 133,  63, "tan3" },
    { 139,  90,  43, "tan4" },
    { 255, 127,  36, "chocolate1" },
    { 238, 118,  33, "chocolate2" },
    { 205, 102,  29, "chocolate3" },
    { 139,  69,  19, "chocolate4" },
    { 255,  48,  48, "firebrick1" },
    { 238,  44,  44, "firebrick2" },
    { 205,  38,  38, "firebrick3" },
    { 139,  26,  26, "firebrick4" },
    { 255,  64,  64, "brown1" },
    { 238,  59,  59, "brown2" },
    { 205,  51,  51, "brown3" },
    { 139,  35,  35, "brown4" },
    { 255, 140, 105, "salmon1" },
    { 238, 130,  98, "salmon2" },
    { 205, 112,  84, "salmon3" },
    { 139,  76,  57, "salmon4" },
    { 255, 160, 122, "lightsalmon1" },
    { 238, 149, 114, "lightsalmon2" },
    { 205, 129,  98, "lightsalmon3" },
    { 139,  87,  66, "lightsalmon4" },
    { 255, 165,   0, "orange1" },
    { 238, 154,   0, "orange2" },
    { 205, 133,   0, "orange3" },
    { 139,  90,   0, "orange4" },
    { 255, 127,   0, "darkorange1" },
    { 238, 118,   0, "darkorange2" },
    { 205, 102,   0, "darkorange3" },
    { 139,  69,   0, "darkorange4" },
    { 255, 114,  86, "coral1" },
    { 238, 106,  80, "coral2" },
    { 205,  91,  69, "coral3" },
    { 139,  62,  47, "coral4" },
    { 255,  99,  71, "tomato1" },
    { 238,  92,  66, "tomato2" },
    { 205,  79,  57, "tomato3" },
    { 139,  54,  38, "tomato4" },
    { 255,  69,   0, "orangered1" },
    { 238,  64,   0, "orangered2" },
    { 205,  55,   0, "orangered3" },
    { 139,  37,   0, "orangered4" },
    { 255,   0,   0, "red1" },
    { 238,   0,   0, "red2" },
    { 205,   0,   0, "red3" },
    { 139,   0,   0, "red4" },
    { 255,  20, 147, "deeppink1" },
    { 238,  18, 137, "deeppink2" },
    { 205,  16, 118, "deeppink3" },
    { 139,  10,  80, "deeppink4" },
    { 255, 110, 180, "hotpink1" },
    { 238, 106, 167, "hotpink2" },
    { 205,  96, 144, "hotpink3" },
    { 139,  58,  98, "hotpink4" },
    { 255, 181, 197, "pink1" },
    { 238, 169, 184, "pink2" },
    { 205, 145, 158, "pink3" },
    { 139,  99, 108, "pink4" },
    { 255, 174, 185, "lightpink1" },
    { 238, 162, 173, "lightpink2" },
    { 205, 140, 149, "lightpink3" },
    { 139,  95, 101, "lightpink4" },
    { 255, 130, 171, "palevioletred1" },
    { 238, 121, 159, "palevioletred2" },
    { 205, 104, 137, "palevioletred3" },
    { 139,  71,  93, "palevioletred4" },
    { 255,  52, 179, "maroon1" },
    { 238,  48, 167, "maroon2" },
    { 205,  41, 144, "maroon3" },
    { 139,  28,  98, "maroon4" },
    { 255,  62, 150, "violetred1" },
    { 238,  58, 140, "violetred2" },
    { 205,  50, 120, "violetred3" },
    { 139,  34,  82, "violetred4" },
    { 255,   0, 255, "magenta1" },
    { 238,   0, 238, "magenta2" },
    { 205,   0, 205, "magenta3" },
    { 139,   0, 139, "magenta4" },
    { 255, 131, 250, "orchid1" },
    { 238, 122, 233, "orchid2" },
    { 205, 105, 201, "orchid3" },
    { 139,  71, 137, "orchid4" },
    { 255, 187, 255, "plum1" },
    { 238, 174, 238, "plum2" },
    { 205, 150, 205, "plum3" },
    { 139, 102, 139, "plum4" },
    { 224, 102, 255, "mediumorchid1" },
    { 209,  95, 238, "mediumorchid2" },
    { 180,  82, 205, "mediumorchid3" },
    { 122,  55, 139, "mediumorchid4" },
    { 191,  62, 255, "darkorchid1" },
    { 178,  58, 238, "darkorchid2" },
    { 154,  50, 205, "darkorchid3" },
    { 104,  34, 139, "darkorchid4" },
    { 155,  48, 255, "purple1" },
    { 145,  44, 238, "purple2" },
    { 125,  38, 205, "purple3" },
    {  85,  26, 139, "purple4" },
    { 171, 130, 255, "mediumpurple1" },
    { 159, 121, 238, "mediumpurple2" },
    { 137, 104, 205, "mediumpurple3" },
    {  93,  71, 139, "mediumpurple4" },
    { 255, 225, 255, "thistle1" },
    { 238, 210, 238, "thistle2" },
    { 205, 181, 205, "thistle3" },
    { 139, 123, 139, "thistle4" },
    {   0,   0,   0, "gray0" },
    {   3,   3,   3, "gray1" },
    {   5,   5,   5, "gray2" },
    {   8,   8,   8, "gray3" },
    {  10,  10,  10, "gray4" },
    {  13,  13,  13, "gray5" },
    {  15,  15,  15, "gray6" },
    {  18,  18,  18, "gray7" },
    {  20,  20,  20, "gray8" },
    {  23,  23,  23, "gray9" },
    {  26,  26,  26, "gray10" },
    {  28,  28,  28, "gray11" },
    {  31,  31,  31, "gray12" },
    {  33,  33,  33, "gray13" },
    {  36,  36,  36, "gray14" },
    {  38,  38,  38, "gray15" },
    {  41,  41,  41, "gray16" },
    {  43,  43,  43, "gray17" },
    {  46,  46,  46, "gray18" },
    {  48,  48,  48, "gray19" },
    {  51,  51,  51, "gray20" },
    {  54,  54,  54, "gray21" },
    {  56,  56,  56, "gray22" },
    {  59,  59,  59, "gray23" },
    {  61,  61,  61, "gray24" },
    {  64,  64,  64, "gray25" },
    {  66,  66,  66, "gray26" },
    {  69,  69,  69, "gray27" },
    {  71,  71,  71, "gray28" },
    {  74,  74,  74, "gray29" },
    {  77,  77,  77, "gray30" },
    {  79,  79,  79, "gray31" },
    {  82,  82,  82, "gray32" },
    {  84,  84,  84, "gray33" },
    {  87,  87,  87, "gray34" },
    {  89,  89,  89, "gray35" },
    {  92,  92,  92, "gray36" },
    {  94,  94,  94, "gray37" },
    {  97,  97,  97, "gray38" },
    {  99,  99,  99, "gray39" },
    { 102, 102, 102, "gray40" },
    { 105, 105, 105, "gray41" },
    { 107, 107, 107, "gray42" },
    { 110, 110, 110, "gray43" },
    { 112, 112, 112, "gray44" },
    { 115, 115, 115, "gray45" },
    { 117, 117, 117, "gray46" },
    { 120, 120, 120, "gray47" },
    { 122, 122, 122, "gray48" },
    { 125, 125, 125, "gray49" },
    { 127, 127, 127, "gray50" },
    { 130, 130, 130, "gray51" },
    { 133, 133, 133, "gray52" },
    { 135, 135, 135, "gray53" },
    { 138, 138, 138, "gray54" },
    { 140, 140, 140, "gray55" },
    { 143, 143, 143, "gray56" },
    { 145, 145, 145, "gray57" },
    { 148, 148, 148, "gray58" },
    { 150, 150, 150, "gray59" },
    { 153, 153, 153, "gray60" },
    { 156, 156, 156, "gray61" },
    { 158, 158, 158, "gray62" },
    { 161, 161, 161, "gray63" },
    { 163, 163, 163, "gray64" },
    { 166, 166, 166, "gray65" },
    { 168, 168, 168, "gray66" },
    { 171, 171, 171, "gray67" },
    { 173, 173, 173, "gray68" },
    { 176, 176, 176, "gray69" },
    { 179, 179, 179, "gray70" },
    { 181, 181, 181, "gray71" },
    { 184, 184, 184, "gray72" },
    { 186, 186, 186, "gray73" },
    { 189, 189, 189, "gray74" },
    { 191, 191, 191, "gray75" },
    { 194, 194, 194, "gray76" },
    { 196, 196, 196, "gray77" },
    { 199, 199, 199, "gray78" },
    { 201, 201, 201, "gray79" },
    { 204, 204, 204, "gray80" },
    { 207, 207, 207, "gray81" },
    { 209, 209, 209, "gray82" },
    { 212, 212, 212, "gray83" },
    { 214, 214, 214, "gray84" },
    { 217, 217, 217, "gray85" },
    { 219, 219, 219, "gray86" },
    { 222, 222, 222, "gray87" },
    { 224, 224, 224, "gray88" },
    { 227, 227, 227, "gray89" },
    { 229, 229, 229, "gray90" },
    { 232, 232, 232, "gray91" },
    { 235, 235, 235, "gray92" },
    { 237, 237, 237, "gray93" },
    { 240, 240, 240, "gray94" },
    { 242, 242, 242, "gray95" },
    { 245, 245, 245, "gray96" },
    { 247, 247, 247, "gray97" },
    { 250, 250, 250, "gray98" },
    { 252, 252, 252, "gray99" },
    { 255, 255, 255, "gray100" },
    { 169, 169, 169, "darkgray" },
    {   0,   0, 139, "darkblue" },
    {   0, 139, 139, "darkcyan" },
    { 139,   0, 139, "darkmagenta" },
    { 139,   0,   0, "darkred" },
    { 144, 238, 144, "lightgreen" }
};


pattern color_pattern(object_p obj)
// ----------------------------------------------------------------------------
//   Build a color pattern from an object
// ----------------------------------------------------------------------------
{
    // Compute color from returned value
    object::id ty = obj->type();
    if (ty == object::ID_True)
    {
        return Settings.Foreground();
    }
    else if (ty == object::ID_False)
    {
        return Settings.Background();
    }
    else if (object::is_based(ty))
    {
        ularge bits = object::is_bignum(ty) ? bignum_p(obj)->value<ularge>()
                                            : integer_p(obj)->value<ularge>();
        return bits;
    }
    else if (object::is_real(ty))
    {
        algebraic_g r = (algebraic_p) obj;
        r = r * integer::make(255);
        if (r)
        {
            int level = r->as_int32(0, true);
            if (!rt.error())
            {
                if (level < 0)
                    level = -level;
                if (level > 255)
                    level = 255;
                return pattern(level, level, level);
            }
        }
    }
    else if (object::is_complex(ty))
    {
        complex_g z = (complex_p) obj;
        algebraic_g a = z->arg(object::ID_Deg);
        algebraic_g m = z->mod() * integer::make(255);
        if (a && m)
        {
            coord hue = a->as_int32(0, true);
            coord val = m->as_int32(0, true);
            if (!rt.error())
            {
                uint rr, gg, bb;
                hsv_to_rgb(hue, 255, val, rr, gg, bb);
                return pattern(rr, gg, bb);
            }
        }
    }
    else if (object::is_array_or_list(ty))
    {
        uint rgb[3] = { 0, 0, 0 };
        uint i = 0;
        bool hsv = ty == object::ID_array;
        for (object_p c : *list_p(obj))
        {
            c = object::strip(c);
            algebraic_g cval = c->as_algebraic();
            bool nothue = !hsv || i > 0;
            cval = cval * integer::make(nothue ? 255 : 360);
            if (!cval)
                break;
            int scale = cval->as_int32(0, true);
            if (rt.error())
                break;
            if (scale < 0 && nothue)
                scale = -scale;
            if (scale > 255)
                scale = 255;
            rgb[i] = scale;
            if (++i >= 3)
                break;
        }
        if (hsv)
            hsv_to_rgb(rgb[0], rgb[1], rgb[2], rgb[0], rgb[1], rgb[2]);
        return pattern(rgb[0], rgb[1], rgb[2]);
    }
    else if (ty == object::ID_text || ty == object::ID_symbol)
    {
        size_t len = 0;
        utf8 txt = text_p(obj)->value(&len);
        for (const auto &color : color_names)
        {
            bool found = true;
            cstring name = color.name;
            for (size_t i = 0; found && i < len; i = utf8_next(txt, i, len))
            {
                unicode cp = utf8_codepoint(txt + i);
                if (cp < 128)
                {
                    char c = (char) cp;
                    if (isspace(c))
                        continue;
                    cp = tolower(c);
                }
                if (*name++ != cp)
                    found = false;
            }
            if (*name != 0)
                found = false;
            if (found)
                return pattern(color.r, color.g, color.b);
        }
    }

    // If we did not have any real error above, report a type error on input
    if (!rt.error())
        rt.type_error();

    return Settings.ErrorForeground();
}
