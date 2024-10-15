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


// ============================================================================
//
//   Numerical solver engine
//
// ============================================================================

static inline void solver_command_error()
// ----------------------------------------------------------------------------
//   Report `EquationSolver` as the failing command
// ----------------------------------------------------------------------------
{
    rt.command(object::static_object(object::ID_EquationSolver));
}


static bool store(algebraic_r value)
// ----------------------------------------------------------------------------
//   Store the last computed value of the variable
// ----------------------------------------------------------------------------
{
    if (expression::independent && +value)
        return directory::store_here(*expression::independent, value);
    return false;
}


static algebraic_p recall(algebraic_r name)
// ----------------------------------------------------------------------------
//   Recall the current value for the given name
// ----------------------------------------------------------------------------
{
    if (object_p obj = directory::recall_all(name, true))
    {
        if (algebraic_p alg = obj->as_algebraic())
        {
            if (symbol_p sym = name->as_quoted<symbol>())
                alg = algebraic_p(tag::make(sym, alg));
            return alg;
        }
    }
    return nullptr;
}

algebraic_p Root::solve(program_r pgm, algebraic_r goal, algebraic_r guess)
// ----------------------------------------------------------------------------
//   The core of the solver
// ----------------------------------------------------------------------------
{
    // Check if the guess is an algebraic or if we need to extract one
    algebraic_g x, dx, lx, hx, nx, px;
    algebraic_g y, dy, ly, hy;
    id  gty = guess->type();
    save<bool>  nodates(unit::nodates, true);

    // Convert A=B+C into A-(B+C)
    program_g eq = pgm;
    if (eq->type() == ID_expression)
        if (expression_g diff = expression_p(+eq)->as_difference_for_solve())
            if (+diff != +eq)
                eq = +diff;

    // Check if low and hight values were given explicitly
    if (is_array_or_list(gty))
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
    unit_g      uname = unit::get(goal);
    algebraic_g uexpr;
    if (uname || lx->type() == ID_unit || hx->type() == ID_unit)
    {
        unit_g lu = unit::get(lx);
        unit_g hu = unit::get(hx);
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
        {
            store(x);
            return x;
        }

        // Evaluate equation
        y = algebraic::evaluate_function(eq, x);

        // If the function evaluates as 10^23 and eps=10^-18, use 10^(23-18)
        if (!i && y && !y->is_zero())
        {
            if (algebraic_g neps = abs::run(y) * yeps)
            {
                if (unit_p ru = unit::get(neps))
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
                store(x);
                return nullptr;
            }
            bad = true;
        }
        else
        {
            is_valid = true;
            if (unit_p yu = unit::get(y))
                dy = yu->value();
            else
                dy = y;
            if (dy->is_zero() || smaller_magnitude(dy, yeps))
            {
                record(solve, "[%u] Solution=%t value=%t", i, +x, +y);
                store(x);
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
            {
                store(x);
                return nullptr;
            }
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
                store(x);
                return x;
            }

            // Check the y interval
            dy = hy - ly;
            if (!dy)
            {
                store(x);
                return nullptr;
            }
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
                store(x);
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
                {
                    store(nx);
                    return nullptr;
                }
            }
            else
            {
                // Jitter value
                int s = (i & 2) - 1;
                if (x && x->is_complex())
                    dx = polar::make(integer::make(997 * s * i),
                                     integer::make(421 * s * i * i),
                                     ID_Deg);
                else
                    dx = integer::make(0x1081 * s * i);
                dx = dx * yeps;
                if (x && x->is_zero())
                    x = dx;
                else
                    x = x + x * dx;
                if (!x)
                {
                    store(dx);
                    return nullptr;
                }
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
    store(x);
    return nullptr;
}


algebraic_p Root::solve(algebraic_r eq,
                        algebraic_r var,
                        algebraic_r guess)
// ----------------------------------------------------------------------------
//   Internal solver code
// ----------------------------------------------------------------------------
//   This selects between three modes of operation:
//   1- Single expression, solving using simple Newton-Raphson in solve():
//       a) eq is an expression or program
//       b) var is a symbol
//       c) guess is a real, complex or an array/list [ low high ]
//   2- Multiple expressions, solving one variable at a time (HP's MES)
//       a) eq is an array/list of expressions or programs
//       b) var is an array/list of symbols
//       c) guess is an array/list of real, complex or [low,high] items
//       d) where we can solve one variable at a time.
//      Specifically, at any point, there is at least one variable where we can
//      find at least one equation where all other variables are defined,
//      meaning that we can find the value of the variable by invoking the
//      single-expression solver.
//   3- Multi-solver, similar to HP's MSLV, with conditions 2a, 2b and 2c.
//      In this case, we use a multi-dimensional Newton-Raphson.
{
    if (!eq || !var || !guess)
        return nullptr;

    record(solve, "Solving %t for variable %t with guess %t",
           +eq, +var, +guess);

    // Check if we have a list of variables to solve for
    if (list_g vars = var->as_array_or_list())
    {
        list_g eqs = eq->as_array_or_list();
        if (!eqs)
            eqs = list::make(ID_list, eq);
        list_g guesses = guess->as_array_or_list();
        if (!guesses)
            guesses = list::make(ID_list, guess);
        return algebraic_p(multiple_equation_solver(eqs, vars, guesses));
    }

    // Check that we have a variable name on stack level 1 and
    // a proram or equation on level 2
    program_g pgm   = program_p(+eq);
    id        pgmty = pgm->type();
    if (pgmty == ID_equation)
    {
        pgm = expression::get(equation_p(+pgm)->value());
        if (!pgm || !pgm->is_algebraic())
            return nullptr;
        pgmty = pgm->type();
    }

    if (pgmty != ID_program && pgmty != ID_expression)
    {
        rt.type_error();
        return nullptr;
    }

    if (!pgm->is_program())
    {
        rt.invalid_equation_error();
        return nullptr;
    }

    // Actual solving
    if (algebraic_g x = solve(pgm, var, guess))
    {
        if (symbol_p name = var->as_quoted<symbol>())
            x = algebraic_p(tag::make(name, +x));
        if (x && !rt.error())
            return x;
    }

    return nullptr;
}


list_p Root::multiple_equation_solver(list_r eqs,
                                      list_r names,
                                      list_r guesses)
// ----------------------------------------------------------------------------
//   Solve multiple equations in sequence (equivalent to HP's MES)
// ----------------------------------------------------------------------------
{
    if (!eqs || !names || !guesses)
        return nullptr;

    // Check that we have names as variables
    size_t vcount = 0;
    for (object_p obj : *names)
    {
        if (!obj->as_quoted<symbol>() && !unit::get(obj))
        {
            rt.type_error();
            return nullptr;
        }
        vcount++;
    }

    // Check that the list of guesses contains real or complex values
    size_t gcount = 0;
    for (object_p obj : *guesses)
    {
        id ty = obj->type();
        if (!is_real(ty) && !is_complex(ty))
        {
            rt.type_error();
            return nullptr;
        }
        gcount++;
    }

    // Number of guesses and variables should match
    if (gcount != vcount)
    {
        rt.dimension_error();
        return nullptr;
    }

    // Looks good: loop on equations trying to find one we can solve
    size_t ecount = eqs->items();
    list_g vars   = names;

    // While there are variables to solve for
    while (vcount && ecount)
    {
        list::iterator vi = vars->begin();
        list::iterator gi = guesses->begin();
        bool           found = false;

        // Loop on all variables, looking for one we can solve for
        for (size_t v = 0; !found && v < vcount; v++)
        {
            symbol_g var = (*vi)->as_quoted<symbol>();
            if (!var)
            {
                rt.type_error();
                return nullptr;
            }
            list::iterator ei = eqs->begin();
            for (size_t e = 0; !found && e < ecount; e++)
            {
                expression_g eq = expression::get(*ei);
                if (!eq)
                {
                    if (!rt.error())
                        rt.type_error();
                    return nullptr;
                }
                if (eq->is_well_defined(var, false))
                {
                    program_g   pgm    = +eq;
                    algebraic_g name   = +var;
                    algebraic_p guess  = algebraic_p(*gi);
                    algebraic_g solved = solve(pgm, name, guess);
                    if (!solved)
                    {
                        rt.no_solution_error();
                        return nullptr;
                    }

                    // That's one variable less to deal with
                    vars = vars->remove(v);
                    vcount--;
                    found = true;
                }
                ++ei;
            }
            ++vi;
            ++gi;
        }
    }

    list_g result = names->map(recall);
    return result;
}




// ============================================================================
//
//   User-level solver commands
//
// ============================================================================

NFUNCTION_BODY(Root)
// ----------------------------------------------------------------------------
//   Numerical solver
// ----------------------------------------------------------------------------
//   The DB48X numerical solver unifies what is `ROOT` and `MSLV` on HP50G.
//   - With a single variable, e.g. `ROOT(sin(X)=0.5;X;0.5)`, it solves for
//     the given variable
//   - With multiple variables in a list or array, it solves for the variables
//     in turn, in the
{
    algebraic_r eqobj    = args[2];
    algebraic_r variable = args[1];
    algebraic_r guess    = args[0];
    return solve(eqobj, variable, guess);
}


NFUNCTION_BODY(MultipleEquationsSolver)
// ----------------------------------------------------------------------------
//   Solve a set of equations one at a time
// ----------------------------------------------------------------------------
//   On DB48X, there is no difference between `MSLV` and `ROOT`
{
    return Root::evaluate(op, args, arity);
}


static algebraic_p check_name(algebraic_r x)
// ----------------------------------------------------------------------------
//   Check if the name exists, if so return its value, otherwise return 0
// ----------------------------------------------------------------------------
{
    if (symbol_p name = x->as_quoted<symbol>())
    {
        if (object_p value = directory::recall_all(name, false))
        {
            if (value->is_real() || value->is_complex())
                return algebraic_p(value);
        }
    }
    return integer::make(0);
}


COMMAND_BODY(MultipleEquationsRoots)
// ----------------------------------------------------------------------------
//   Apply the multiple equation solver to the contents of EQ
// ----------------------------------------------------------------------------
{
    if (list_g eqs = expression::current_equation(true, true))
        if (list_g vars = eqs->names())
            if (list_g values = vars->map(check_name))
                if (rt.push(+eqs) && rt.push(+vars) && rt.push(+values))
                    return run<Root>();
    if (!rt.error())
        rt.no_equation_error();
    return ERROR;
}



// ============================================================================
//
//   Solving menu
//
// ============================================================================

COMMAND_BODY(StEq)
// ----------------------------------------------------------------------------
//   Store expression in `Equation` variable
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        id objty = obj->type();
        if (is_array_or_list(objty))
        {
            for (object_p i: *list_p(obj))
            {
                objty = i->type();
                if (objty != ID_expression && objty != ID_polynomial &&
                    objty != ID_equation)
                    rt.type_error();
            }
        }
        if (objty != ID_expression &&
            objty != ID_polynomial &&
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
    if (list_p expr = expression::current_equation(false, true))
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

        if (is_array_or_list(ty))
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
    if (list_g eq = expression::current_equation(false, true))
    {
        expression_p expr = eq->as<expression>();
        if (expr && expr->is_well_defined())
        {
            // We will run programs, do not save stack, etc.
            settings::PrepareForFunctionEvaluation willEvaluateFunctions;
            save<bool> nodates(unit::nodates, true);

            expression_g left, right;
            if (expr->split_equation(left, right))
            {
                algebraic_g lv = left->evaluate();
                algebraic_g rv = right->evaluate();
                algebraic_g d = lv - rv;
                if (d && !d->is_zero(false))
                {
                    if (d->is_negative(false))
                        rv = expression::make(ID_sub, rv, -d);
                    else
                        rv = expression::make(ID_add, rv, d);
                }
                rv = expression::make(ID_TestEQ, lv, rv);
                if (rv && rt.push(+rv))
                    return OK;
            }
            else
            {
                algebraic_p value = expr->evaluate();
                if (value && rt.push(value))
                    return OK;
            }
        }
    }
    if (!rt.error())
        rt.no_equation_error();
    return ERROR;
}


MENU_BODY(SolvingMenu)
// ----------------------------------------------------------------------------
//   Process the MENU command for SolvingMenu
// ----------------------------------------------------------------------------
{
    bool   all    = Settings.AllEquationVariables();
    list_p expr   = expression::current_equation(all, false);
    list_g vars   = expr ? expr->names() : nullptr;
    size_t nitems = vars ? vars->items() : 0;
    items_init(mi, nitems + 1, 3, 1);
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

    if (list_p expr = expression::current_equation(false, false))
        ui.transient_object(expr);

    return true;
}


static symbol_p expression_variable(uint index)
// ----------------------------------------------------------------------------
//   Return the variable in EQ for a given index
// ----------------------------------------------------------------------------
{
    bool all = Settings.AllEquationVariables();
    if (list_p expr = expression::current_equation(all, true))
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
    bool all = Settings.AllEquationVariables();
    if (list_p expr = expression::current_equation(all, true))
    {
        if (list_g vars = expr->names(true))
        {
            if (object_p obj = vars->at(index))
            {
                if (symbol_p sym = obj->as<symbol>())
                    return sym;
                if (unit_p u = unit::get(obj))
                    return u;
            }
        }
    }
    return nullptr;
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
                if (tag_p tagged = tag::make(sym, value))
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

                if (unit_g uvar = unit::get(entry))
                {
                    if (symbol_g sym = symbol_p(uvar->value()))
                    {
                        if (algebraic_g sval = value->as_algebraic())
                        {
                            unit_g uval = unit::get(value);
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
        uint idx = key - KEY_F1 + 5 * ui.page() - 1;
        if (symbol_g sym = expression_variable(idx))
        {
            object_g obj = directory::recall_all(sym, false);
            algebraic_g value = obj ? obj->as_algebraic() : nullptr;
            if (!value)
                value = integer::make(0);
            if (list_g eql = expression::current_equation(false, true))
                if (program_g eq = eql->as<expression>())
                    if (value && expression_p(+eq)->is_well_defined(sym))
                        if (algebraic_p var = expression_variable_or_unit(idx))
                            if (algebraic_g res = Root::solve(eq, var, value))
                                if (directory::store_here(sym, res))
                                    if (tag_p tagged = tag::make(sym, res))
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
