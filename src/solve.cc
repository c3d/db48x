// ****************************************************************************
//  solve.cc                                                      DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Numerical root finder
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

#include "solve.h"

#include "algebraic.h"
#include "arithmetic.h"
#include "array.h"
#include "compare.h"
#include "equations.h"
#include "expression.h"
#include "functions.h"
#include "integer.h"
#include "list.h"
#include "recorder.h"
#include "settings.h"
#include "symbol.h"
#include "tag.h"
#include "unit.h"
#include "variables.h"

RECORDER(solve, 16, "Numerical solver");
RECORDER(solve_error, 16, "Numerical solver errors");


NFUNCTION_BODY(Root)
// ----------------------------------------------------------------------------
//   Numerical solver
// ----------------------------------------------------------------------------
{
    if (arity != 3)
    {
        rt.internal_error();
        return nullptr;
    }
    algebraic_g &eqobj    = args[2];
    algebraic_g &variable = args[1];
    algebraic_g &guess    = args[0];
    if (!eqobj || !variable || !guess)
        return nullptr;

    record(solve, "Solving %t for variable %t with guess %t",
           +eqobj, +variable, +guess);

    // Check that we have a variable name on stack level 1 and
    // a proram or equation on level 2
    symbol_g name = variable->as_quoted<symbol>();
    id       eqty = eqobj->type();
    if (eqty == ID_equation)
    {
        eqobj = algebraic_p(equation_p(+eqobj)->value());
        if (!eqobj || !eqobj->is_algebraic())
            return nullptr;
        eqty = eqobj->type();
    }
    if ((eqty != ID_program && eqty != ID_expression) || !name)
    {
        rt.type_error();
        return nullptr;
    }

    if (!eqobj->is_program())
    {
        rt.invalid_equation_error();
        return nullptr;
    }

    // Actual solving
    program_g eq = program_p(+eqobj);
    if (algebraic_g x = solve(eq, +name, +guess))
    {
        size_t   nlen = 0;
        gcutf8   ntxt = name->value(&nlen);
        object_g top  = tag::make(ntxt, nlen, +x);
        if (top && !rt.error())
            return algebraic_p(+top);
    }

    return nullptr;
}


static inline void solver_command_error()
// ----------------------------------------------------------------------------
//   Report `EquationSolver` as the failing command
// ----------------------------------------------------------------------------
{
    rt.command(object::static_object(object::ID_EquationSolver));
}


algebraic_p solve(program_g eq, algebraic_g goal, object_g guess)
// ----------------------------------------------------------------------------
//   The core of the solver
// ----------------------------------------------------------------------------
{
    // Check if the guess is an algebraic or if we need to extract one
    algebraic_g x, dx, lx, hx, nx, px;
    algebraic_g y, dy, ly, hy;
    object::id  gty = guess->type();
    save<bool>  nodates(unit::nodates, true);

    // Convert A=B+C into A-(B+C)
    if (eq->type() == object::ID_expression)
        if (expression_g diff = expression_p(+eq)->as_difference_for_solve())
            if (+diff != +eq)
                eq = +diff;

    // Check if low and hight values were given explicitly
    if (gty == object::ID_list || gty == object::ID_array)
    {
        lx = guess->algebraic_child(0);
        hx = guess->algebraic_child(1);
    }
    else
    {
        lx = guess->as_algebraic();
        hx = lx;
    }
    if (!hx || !lx)
    {
        rt.type_error();
        return nullptr;
    }

    // Check if the variable we solve for is a unit
    unit_g      uname = goal->as<unit>();
    algebraic_g uexpr;
    if (uname || lx->type() == object::ID_unit || hx->type() == object::ID_unit)
    {
        unit_g lu = lx->as<unit>();
        unit_g hu = hx->as<unit>();
        if (uname)
            uexpr = uname->uexpr();
        else if (lu)
            uexpr = lu->uexpr();
        else if (hu)
            uexpr = hu->uexpr();
        if (!uexpr)
        {
            rt.internal_error();
            return nullptr;
        }

        if (!lu)
            lu = unit::make(lx, uexpr);
        if (!hu)
            hu = unit::make(hx, uexpr);

        if (!lu || !hu || !lu->convert(hu))
            return nullptr;
        lx = +lu;
        hx = +hu;
    }

    // Check if low and hight are identical, if so pick hx=1.01*lx
    if (algebraic_p diff = hx - lx)
    {
        if (diff->is_zero(true))
        {
            algebraic_g delta =
                +fraction::make(integer::make(1234), integer::make(997));
            if (!hx->is_zero(true))
                hx = hx * delta;
            else if (uexpr)
                hx = unit::simple(delta, uexpr);
            else
                hx = delta;
        }
    }
    if (rt.error())
        return nullptr;

    // Initialize starting value
    x = lx;
    record(solve, "Initial range %t-%t", +lx, +hx);

    // We will run programs, do not save stack, etc.
    settings::PrepareForFunctionEvaluation willEvaluateFunctions;
    settings::SaveNumericalConstants snc(true);

    // Set independent variable
    symbol_g name = goal->as_quoted<symbol>();
    if (!name)
    {
        if (!uname)
        {
            rt.type_error();
            return nullptr;
        }
        name = uname->value()->as_quoted<symbol>();
        if (!name)
        {
            rt.some_invalid_name_error();
            return nullptr;
        }
    }
    save<symbol_g *> iref(expression::independent, &name);
    int              prec = Settings.Precision() - Settings.SolverImprecision();
    algebraic_g      yeps  = decimal::make(1, prec <= 0 ? -1 : -prec);
    algebraic_g      xeps  = (lx + hx) * yeps;
    bool             is_constant = true;
    bool             is_valid    = false;
    uint             max         = Settings.SolverIterations();

    for (uint i = 0; i < max && !program::interrupted(); i++)
    {
        bool bad = false;

        // If we are starting to use really big numbers, switch to decimal
        if (!algebraic::to_decimal_if_big(x))
            return x;

        // Evaluate equation
        y = algebraic::evaluate_function(eq, x);

        // If the function evaluates as 10^23 and eps=10^-18, use 10^(23-18)
        if (!i && y && !y->is_zero())
        {
            if (algebraic_g neps = abs::run(y) * yeps)
            {
                if (unit_p ru = neps->as<unit>())
                    neps = ru->value();
                if (smaller_magnitude(yeps, neps))
                    yeps = neps;
            }
        }
        record(solve, "[%u] x=%t (%t to %t)  y=%t (%t to %t) err=%t",
               i, +x, +lx, +hx, +y, +ly, +hy, +yeps);
        if (!y)
        {
            // Error on last function evaluation, try again
            record(solve_error, "Got error %+s", rt.error());
            if (!ly || !hy)
            {
                if (!rt.error())
                    rt.bad_guess_error();
                solver_command_error();
                return nullptr;
            }
            bad = true;
        }
        else
        {
            is_valid = true;
            if (unit_p yu = y->as<unit>())
                dy = yu->value();
            else
                dy = y;
            if (dy->is_zero() || smaller_magnitude(dy, yeps))
            {
                record(solve, "[%u] Solution=%t value=%t", i, +x, +y);
                return x;
            }

            if (!ly)
            {
                record(solve, "Setting low");
                ly = y;
                lx = x;
                x  = hx;
                continue;
            }
            else if (!hy)
            {
                record(solve, "Setting high");
                hy = y;
                hx = x;
            }
            else if (smaller_magnitude(y, ly))
            {
                // Smaller than the smallest
                record(solve, "Smallest");
                hx = lx;
                hy = ly;
                lx = x;
                ly = y;
            }
            else if (smaller_magnitude(y, hy))
            {
                record(solve, "Improvement");
                // Between smaller and biggest
                hx = x;
                hy = y;
            }
            else if (smaller_magnitude(hy, y))
            {
                // y became bigger, and all have the same sign:
                // try to get closer to low
                record(solve, "New value is worse");
                is_constant = false;
                bad         = true;
            }
            else
            {
                // y is constant - Try a random spot
                record(solve, "Value seems constant");
                bad = true;
            }

            // Check if cross zero (change sign)
            if (dy->is_negative(false))
                nx = x;
            else
                px = x;

            // Check the x interval
            dx = hx - lx;
            if (!dx)
                return nullptr;
            dy = hx + lx;
            if (!dy || dy->is_zero(false))
                dy = yeps;
            else
                dy = dy * yeps;
            if (dx->is_zero(false) || smaller_magnitude(dx, xeps))
            {
                x = lx;
                record(solve, "[%u] Minimum=%t value=%t", i, +x, +y);
                if (nx && px)
                    rt.sign_reversal_error();
                else
                    rt.no_solution_error();
                solver_command_error();
                return x;
            }

            // Check the y interval
            dy = hy - ly;
            if (!dy)
                return nullptr;
            if (dy->is_zero(false))
            {
                record(solve,
                       "[%u] unmoving %t between %t and %t", +hy, +lx, +hx);
                bad = true;
            }
            else
            {
                // Interpolate to find new position
                record(solve, "[%u] Moving to %t - %t * %t / %t",
                       i, +lx, +y, +dx, +dy);
                is_constant = false;
                x = lx - y * dx / dy;
            }

            // Check if there are unresolved symbols
            if (!x || x->is_symbolic())
            {
                if (!rt.error())
                    rt.invalid_function_error();
                solver_command_error();
                return x;
            }
        }

        // If we have some issue improving things, shake it a bit
        if (bad)
        {
            // Check if we crossed and the new x does not look good
            if (nx && px)
            {
                // Try to bisect
                dx = integer::make(2);
                x  = (nx + px) / dx;
                if (!x)
                    return nullptr;
            }
            else
            {
                // Jitter value
                int s = (i & 2) - 1;
                if (x && x->is_complex())
                    dx = polar::make(integer::make(997 * s * i),
                                     integer::make(421 * s * i * i),
                                     object::ID_Deg);
                else
                    dx = integer::make(0x1081 * s * i);
                dx = dx * yeps;
                if (x && x->is_zero())
                    x = dx;
                else
                    x = x + x * dx;
                if (!x)
                    return nullptr;
                record(solve, "Jitter x=%t", +x);
            }
        }
    }

    record(solve, "Exited after too many loops, x=%t y=%t lx=%t ly=%t",
           +x, +y, +lx, +ly);

    if (!is_valid)
        rt.invalid_function_error();
    else if (is_constant)
        rt.constant_value_error();
    else
        rt.no_solution_error();
    if (rt.error())
        solver_command_error();
    return lx;
}


// ============================================================================
//
//   Solving menu
//
// ============================================================================

static bool is_well_defined(expression_r expr, symbol_p solving = nullptr)
// ----------------------------------------------------------------------------
//   Check if all variables but the one we solve for are defined
// ----------------------------------------------------------------------------
{
    symbol_g sym = solving;
    list_p vars = expr->names();
    for (auto var : *vars)
    {
        if (symbol_p vsym = var->as<symbol>())
        {
            if (!sym || !sym->is_same_as(vsym))
            {
                if (!directory::recall_all(var, false))
                {
                    rt.some_undefined_name_error();
                    return false;
                }
            }
        }
        else
        {
            rt.some_invalid_name_error();
            return false;
        }
    }
    return true;
}


COMMAND_BODY(StEq)
// ----------------------------------------------------------------------------
//   Store expression in `Equation` variable
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        id objty = obj->type();
        if (objty == ID_list || objty == ID_array)
        {
            for (object_p obj: *list_p(obj))
            {
                objty = obj->type();
                if (objty != ID_expression && objty != ID_polynomial &&
                    objty != ID_equation)
                    rt.type_error();
            }
        }
        if (objty != ID_expression && objty != ID_polynomial &&
                objty != ID_equation)
                rt.type_error();
        else if (directory::store_here(static_object(ID_Equation), obj))
            if (rt.drop())
                return OK;
    }
    return ERROR;
}


COMMAND_BODY(RcEq)
// ----------------------------------------------------------------------------
//   Store expression in `Equation` variable
// ----------------------------------------------------------------------------
{
    if (expression_p expr = expression::current_equation(true))
        if (rt.push(expr))
            return OK;
    return ERROR;
}


COMMAND_BODY(NextEq)
// ----------------------------------------------------------------------------
//   Cycle equations in the Eq variable if it's a list
// ----------------------------------------------------------------------------
{
    object_p eqname = static_object(ID_Equation);
    object_p obj = directory::recall_all(eqname, false);
    if (obj)
    {
        id ty = obj->type();
        if (ty == ID_equation)
        {
            obj = equation_p(obj)->value();
            if (obj)
                ty = obj->type();
        }

        if (ty == ID_list || ty == ID_array)
        {
            size_t sz = 0;
            if (list_p(obj)->expand_without_size(&sz))
            {
                rt.roll(sz);
                list_g now = to_list_object(sz);
                if (directory::store_here(eqname, now))
                {
                    ui.menu_refresh(ID_SolvingMenu);
                    return OK;
                }
            }
        }
    }
    return ERROR;
}


COMMAND_BODY(EvalEq)
// ----------------------------------------------------------------------------
//   Evaluate the current equation
// ----------------------------------------------------------------------------
{
    if (expression_g expr = expression::current_equation(true))
    {
        if (is_well_defined(expr))
        {
            // We will run programs, do not save stack, etc.
            settings::PrepareForFunctionEvaluation willEvaluateFunctions;
            save<bool> nodates(unit::nodates, true);

            expression_g diff = expr->as_difference_for_solve();
            if (+diff != +expr)
            {
                expression_g l = expr->left_of_equation();
                expression_g r = expr->right_of_equation();
                algebraic_g lv = l->evaluate();
                algebraic_g rv = r->evaluate();
                algebraic_g d = lv - rv;
                if (d && !d->is_zero(false))
                {
                    if (d->is_negative(false))
                        r = expression::make(ID_sub, rv, -d);
                    else
                        r = expression::make(ID_add, rv, d);
                    rv = +r;
                }
                r = expression::make(ID_TestEQ, lv, rv);
                if (r && rt.push(+r))
                    return OK;
            }
        }
    }
    return ERROR;
}


MENU_BODY(SolvingMenu)
// ----------------------------------------------------------------------------
//   Process the MENU command for SolvingMenu
// ----------------------------------------------------------------------------
{
    expression_p expr   = expression::current_equation(false);
    list_g       vars   = expr ? expr->names() : nullptr;
    size_t       nitems = vars ? vars->items() : 0;
    items_init(mi, nitems+1, 3, 1);
    if (!vars)
        return false;

    uint skip = mi.skip;

    // First row: Store variables
    mi.plane  = 0;
    mi.planes = 1;
    menu::items(mi, "EvalEq", ID_EvalEq);
    for (auto name : *vars)
        if (symbol_p sym = name->as<symbol>())
            menu::items(mi, sym, menu::ID_SolvingMenuStore);

    // Second row: Solve for variables
    mi.plane  = 1;
    mi.planes = 2;
    mi.skip   = skip;
    mi.index  = mi.plane * ui.NUM_SOFTKEYS;
    menu::items(mi, "NextEq", ID_NextEq);
    for (auto name : *vars)
        if (symbol_p sym = name->as<symbol>())
            menu::items(mi, sym, menu::ID_SolvingMenuSolve);

    // Third row: Recall variable
    mi.plane  = 2;
    mi.planes = 3;
    mi.skip   = skip;
    mi.index  = mi.plane * ui.NUM_SOFTKEYS;
    menu::items(mi, "Eq▶", ID_RcEq);
    for (auto name : *vars)
    {
        if (symbol_g sym = name->as<symbol>())
        {
            settings::SaveDisplayDigits sdd(3);
            object_p value = directory::recall_all(sym, false);
            if (!value)
                value = symbol::make("?");
            if (value)
                sym = value->as_symbol(false);
            menu::items(mi, sym, menu::ID_SolvingMenuRecall);
        }
    }

    // Add markers
    for (uint k = 1; k < ui.NUM_SOFTKEYS - (mi.pages > 1); k++)
    {
        ui.marker(k + 0 * ui.NUM_SOFTKEYS, L'▶', true);
        ui.marker(k + 1 * ui.NUM_SOFTKEYS, L'?', false);
        ui.marker(k + 2 * ui.NUM_SOFTKEYS, L'▶', false);
    }

    if (expression_p expr = expression::current_equation(false))
        ui.transient_object(expr);

    return true;
}


static symbol_p expression_variable(uint index)
// ----------------------------------------------------------------------------
//   Return the variable in EQ for a given index
// ----------------------------------------------------------------------------
{
    if (expression_p expr = expression::current_equation(true))
        if (list_g vars = expr->names())
            if (object_p obj = vars->at(index))
                if (symbol_p sym = obj->as<symbol>())
                    return sym;
    return nullptr;
}


static algebraic_p expression_variable_or_unit(uint index)
// ----------------------------------------------------------------------------
//   Return the unit in EQ for a given index if there is one, otherwise name
// ----------------------------------------------------------------------------
{
    if (expression_p expr = expression::current_equation(true))
    {
        if (list_g vars = expr->names(true))
        {
            if (object_p obj = vars->at(index))
            {
                if (symbol_p sym = obj->as<symbol>())
                    return sym;
                if (unit_p u = obj->as<unit>())
                    return u;
            }
        }
    }
    return nullptr;
}


static tag_p tagged_value(symbol_p sym, object_p value)
// ----------------------------------------------------------------------------
//  Tag a solver value with a symbol
// ----------------------------------------------------------------------------
{
    size_t nlen   = 0;
    gcutf8 ntxt   = sym->value(&nlen);
    tag_p  tagged = tag::make(ntxt, nlen, value);
    return tagged;
}


COMMAND_BODY(SolvingMenuRecall)
// ----------------------------------------------------------------------------
//   Recall a variable from the SolvingMenu
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (key >= KEY_F1 && key <= KEY_F6)
    {
        uint index = key - KEY_F1 + 5 * ui.page() - 1;
        if (symbol_g sym = expression_variable(index))
            if (object_p value = directory::recall_all(sym, true))
                if (tag_p tagged = tagged_value(sym, value))
                    if (rt.push(tagged))
                        return OK;
    }

    return ERROR;
}


INSERT_BODY(SolvingMenuRecall)
// ----------------------------------------------------------------------------
//   Insert the name of a variable with `Recall` after it
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    return ui.insert_softkey(key, " '", "' Recall ", false);
}


COMMAND_BODY(SolvingMenuStore)
// ----------------------------------------------------------------------------
//   Store a variable from the SolvingMenu
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (key >= KEY_F1 && key <= KEY_F6)
    {
        uint index = key - KEY_F1 + 5 * ui.page() - 1;
        if (algebraic_p entry = expression_variable_or_unit(index))
        {
            if (object_p value = tag::strip(rt.pop()))
            {
                if (symbol_p sym = entry->as<symbol>())
                    if (directory::store_here(sym, value))
                        if (ui.menu_refresh())
                            return OK;

                if (unit_g uvar = entry->as<unit>())
                {
                    if (symbol_g sym = symbol_p(uvar->value()))
                    {
                        if (algebraic_g sval = value->as_algebraic())
                        {
                            unit_g uval = value->as<unit>();
                            if (uval)
                            {
                                if (!uvar->convert(uval))
                                    return ERROR;
                            }

                            else
                            {
                                sval = unit::simple(sval, uvar->uexpr());
                            }

                            if (directory::store_here(sym, sval))
                                if (ui.menu_refresh())
                                    return OK;
                        }
                        else
                        {
                            rt.type_error();
                        }
                    }
                }
            }
        }
    }
    return ERROR;
}


INSERT_BODY(SolvingMenuStore)
// ----------------------------------------------------------------------------
//   Insert the name of a variable with `Store` after it
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    return ui.insert_softkey(key, " '", "' Store ", false);
}


COMMAND_BODY(SolvingMenuSolve)
// ----------------------------------------------------------------------------
//  Solve for a given variable
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (key >= KEY_F1 && key <= KEY_F6)
    {
        uint index = key - KEY_F1 + 5 * ui.page() - 1;
        if (symbol_g sym = expression_variable(index))
        {
            object_g value = directory::recall_all(sym, false);
            if (!value)
                value = integer::make(0);
            if (expression_g eq = expression::current_equation(true))
                if (value && is_well_defined(eq, sym))
                    if (algebraic_p var = expression_variable_or_unit(index))
                        if (algebraic_g result = solve(+eq, var, value))
                            if (directory::store_here(sym, result))
                                if (tag_p tagged = tagged_value(sym, result))
                                    if (rt.push(tagged))
                                        if (ui.menu_refresh())
                                            return OK;
        }
    }

    return ERROR;
}


INSERT_BODY(SolvingMenuSolve)
// ----------------------------------------------------------------------------
//   Insert the name of a variable
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    return ui.insert_softkey(key, " EQ '", "'  0 Root ", false);
}
