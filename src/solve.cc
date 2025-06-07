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
#include "finance.h"
#include "functions.h"
#include "integer.h"
#include "list.h"
#include "recorder.h"
#include "settings.h"
#include "stats.h"
#include "symbol.h"
#include "tag.h"
#include "unit.h"
#include "variables.h"

RECORDER(msolve, 16, "Multiple-equation numerical solver");
RECORDER(jsolve, 16, "Jacobian numerical solver");
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


static algebraic_p recall(algebraic_r namer)
// ----------------------------------------------------------------------------
//   Recall the current value for the given name
// ----------------------------------------------------------------------------
{
    algebraic_g name = namer;
    while (unit_p u = unit::get(+name))
        name = u->value();
    if (object_p obj = directory::recall_all(name, true))
    {
        if (algebraic_p alg = obj->as_algebraic())
        {
            if (symbol_p sym = name->as_quoted<symbol>())
                alg = assignment::make(sym, alg);
            return alg;
        }
    }
    return nullptr;
}

static algebraic_p difference_for_solve(algebraic_r eq)
// ----------------------------------------------------------------------------
//   Transform equations into proper differences
// ----------------------------------------------------------------------------
{
    if (expression_p eqeq = expression::get(eq))
        if (expression_g diff = eqeq->as_difference_for_solve())
            return diff;
    return eq;
}

algebraic_p Root::solve(program_r pgm, algebraic_r goal, algebraic_r guess)
// ----------------------------------------------------------------------------
//   The core of the solver, numerical solving for a single variable
// ----------------------------------------------------------------------------
{
    // Check if the guess is an algebraic or if we need to extract one
    algebraic_g x, dx, lx, hx;  // Current, delta, low, high for x
    algebraic_g y, dy, ly, hy;  // Current, delta, low, high for f(x)
    algebraic_g nx, px;         // x where f(x) is negative and positive
    algebraic_g sy;
    id          gty = guess->type();
    save<bool>  nodates(unit::nodates, true);

    // Convert A=B+C into A-(B+C)
    program_g eq = pgm;
    if (expression_p eqeq = expression::get(eq))
        if (expression_g diff = eqeq->as_difference_for_solve())
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

    // Check if we need to work in the complex plane
    bool is_complex = lx->is_complex() || hx->is_complex();

    // Check if low and hight are identical, if so pick hx=1.01*lx
    if (algebraic_p diff = hx - lx)
    {
        if (diff->is_zero(false))
        {
            algebraic_g delta =
                +fraction::make(integer::make(1234), integer::make(997));
            if (!hx->is_zero(false))
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
    record(solve, "Solving %t for %t with guess %t", +pgm, +goal, +guess);
    record(solve, "Initial range for %t is %t to %t", +goal, +lx, +hx);

    // We will run programs, do not save stack, etc.
    settings::PrepareForSolveFunctionEvaluation willEvaluateForSolve;
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
    int              impr        = Settings.SolverImprecision();
    algebraic_g      yeps        = algebraic::epsilon(impr);
    algebraic_g      xeps        = (lx + hx) * yeps;
    bool             is_constant = true;
    bool             is_valid    = false;
    uint             max         = Settings.SolverIterations();
    algebraic_g      two         = integer::make(2);
    algebraic_g      maxscale    = integer::make(63);
    int              degraded    = 0;

    // Check if we can isolate the variable algebraically
    if (Settings.SymbolicSolver())
    {
        if (expression_p eqeq = eq->as<expression>())
        {
            if (expression_p isol = isolate(eqeq, name))
            {
                expression_g left, right;
                if (isol->split_equation(left, right))
                {
                    algebraic_g la = +left;
                    if (unit_p lu = left->as_quoted<unit>())
                        if (algebraic_p lua = lu->value()->as_algebraic())
                            la = lua;
                    if (symbol_p lname = la->as_quoted<symbol>())
                    {
                        if (lname->is_same_as(name))
                        {
                            settings::SaveComplexResults scr(is_complex);
                            algebraic_g value = right->evaluate();
                            if (value)
                            {
                                if (uexpr)
                                    unit_p(+lx)->convert(value);
                                else if (unit_g uv = value->as<unit>())
                                    if (algebraic_p num = uv->convert_to_real())
                                        value = num;
                                store(value);
                            }
                            return value;
                        }
                    }
                }
            }
        }
    }

    for (uint i = 0; i < max; i++)
    {
        if (program::interrupted())
        {
            rt.interrupted_error();
            break;              // This will keep current values
        }

        // If we failed during evaluation of x, break
        if (!x)
        {
            if (!rt.error())
                rt.bad_guess_error();
            return nullptr;
        }

        // If we are starting to use really big numbers, switch to decimal
        if (!algebraic::to_decimal_if_big(x))
        {
            store(x);
            return nullptr;
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
        record(solve, "[%u] x=%t [%t, %t]  y=%t [%t, %t] err=%t",
               i, +x, +lx, +hx, +y, +ly, +hy, +yeps);
        if (!y)
        {
            // Error on last function evaluation, try again
            record(solve_error, "Got error %+s", rt.error());
            if (!ly || !hy)
            {
                if (!rt.error())
                    rt.bad_guess_error();
                store(x);
                return nullptr;
            }

            // If we got an error, we probably ran out of the domain.
            // Chances are that the domain is between known good values
            // if we had them, outside if we had none.
            if (!degraded)
                degraded = hy && ly ? 1 : -1;
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
                record(solve, "Setting low %t=f(%t)", +y, +x);
                if (y->is_negative(false))
                    nx = x;
                else
                    px = x;
                ly = y;
                lx = x;
                x  = hx;
                continue;
            }
            else if (smaller_magnitude(y, ly))
            {
                // Smaller than the smallest
                record(solve, "Smallest %t=f(%t) [%t, %t]", +y, +x, +lx, +hx);
                hx = lx;
                hy = ly;
                lx = x;
                ly = y;
                degraded = 0;
            }
            else if (!hy)
            {
                record(solve, "Setting high %t=f(%t)", +y, +x);
                hy = y;
                hx = x;
            }
            else if (smaller_magnitude(y, hy))
            {
                // Between smaller and biggest
                record(solve, "Better %t=f(%t) [%t, %t]", +y, +x, +lx, +hx);
                hx = x;
                hy = y;
                degraded = 0;
            }
            else if (smaller_magnitude(hy, y))
            {
                // y became bigger, and all have the same sign:
                // try to get closer to low
                record(solve, "Worse %t=f(%t) [%t, %t]", +y, +x, +lx, +hx);
                is_constant = false;
                if (!degraded)
                {
                    // Look ouside if x is beteen lx and hx, inside otherwise
                    dx = (hx - x) * (lx - x);
                    degraded = dx && dx->is_negative(false) ? -1 : 1;
                }
            }
            else
            {
                // y is constant - Try a random spot
                record(solve, "Value seems constant %t=f(%t) [%t, %t]",
                       +y, +x, +lx, +hx);
                if (!degraded)
                    degraded = -1;                      // Look outside
            }

            if (!degraded)
            {
                // This was an improvement over at least one end
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
                           "[%u] Unmoving %t [%t, %t]", +hy, +lx, +hx);
                    degraded = 1;                       // Look inwards
                }
                else
                {
                    // Strong slope: Interpolate to find new position
                    record(solve, "[%u] Moving to %t - %t * %t / %t",
                           i, +lx, +y, +dx, +dy);
                    is_constant = false;

                    sy = y / dy;
                    if (!sy || smaller_magnitude(maxscale, sy))
                    {
                        // Very weak slope: Avoid going deep into the woods
                        record(solve, "[%u] Slow moving from %t scale %t",
                               i, +x, + sy);
                        sy = sy && sy->is_negative() ? -two : two;
                    }
                    x = lx - sy * dx;
                    record(solve, "[%u] Moved to %t [%t, %t]", i, +x, +lx, +hx);
                }
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

        // If we have some issue improving things, move to degraded mode
        // When degraded > 0, x is ouside [lx, hx], and we move inwards
        // When degraded < 0, x is inside [lx, hx], and we move outwards
        // We move relative to hx, reducing/enlarging x-hx
        if (degraded)
        {
            // Check if we crossed and the new x does not look good
            if (nx && px)
            {
                // Try to bisect
                x = (nx + px) / two;
                if (!x)
                {
                    store(nx);
                    return nullptr;
                }
            }
            else
            {
                // Find the next step
                dx = x - hx;
                dy = +decimal::e();
                if (is_complex)
                    dy = polar::make(dy, dy, ID_Deg);
                if (degraded > 0)
                {
                    dx = dx / dy;
                    degraded++;
                }
                else
                {
                    dx = dx * dy;
                    degraded--;
                }
                x = hx + dx;
                if (!x)
                {
                    solver_command_error();
                    store(lx);
                    return nullptr;
                }
                record(solve, "Degraded %d x=%t in [%t,%t]",
                       degraded, +x, +lx, +hx);
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
    store(lx);
    return nullptr;
}


algebraic_p Root::solve(algebraic_g &eq,
                        algebraic_g &var,
                        algebraic_g &guess)
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

    // Check that we have a program or equation as input
    if (equation_p libeq = eq->as_quoted<equation>())
    {
        object_p value = libeq->value();
        if (!value || !value->is_extended_algebraic())
            return nullptr;
        eq = algebraic_p(value);
    }

    // Check if input arguments are wrapped in equations (algebraic case)
    if (list_p eql = eq->as_quoted<list, array>())
        eq = eql;
    if (list_p varl = var->as_quoted<list, array>())
        var = varl;
    if (list_p guessl = guess->as_quoted<list, array>())
        guess = guessl;

    // Check if we have a list of variables or equations to solve for
    if (eq->is_array_or_list() || var->is_array_or_list())
    {
        list_g vars    = var->as_array_or_list();
        list_g eqs     = eq->as_array_or_list();
        list_g guesses = guess->as_array_or_list();
        if (!eqs)
            eqs = list::make(ID_list, eq);
        bool onevar = !vars;
        if (onevar)
            vars = list::make(ID_list, var);
        if (!guesses)
            guesses = list::make(ID_list, guess);
        algebraic_g r = multiple_equation_solver(eqs, vars, guesses);
        if (r && onevar)
            if (list_p lst = r->as_array_or_list())
                if (lst->items() == 1)
                    r = algebraic_p(lst->head());
        return r;
    }

    // Otherwise we expect a regular program
    if (program_g pgm = eq->as_quoted<program, expression>())
    {
        // Actual solving
        if (algebraic_g x = solve(pgm, var, guess))
        {
            if (FinanceSolverMenu::active())
                FinanceSolverMenu::round(x);
            if (symbol_p name = var->as_quoted<symbol>())
                x = assignment::make(name, x);
            if (x && !rt.error())
                return x;
        }
    }
    else
    {
        rt.invalid_equation_error();
    }

    return nullptr;
}


list_p Root::multiple_equation_solver(list_r eqs, list_r names, list_r guesses)
// ----------------------------------------------------------------------------
//   Solve multiple equations in sequence (equivalent to HP's MES)
// ----------------------------------------------------------------------------
{
    if (!eqs || !names || !guesses)
        return nullptr;

    save<bool>  nodates(unit::nodates, true);
    record(msolve, "Solve equations %t for names %t with guesses %t",
           +eqs, +names, +guesses);

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
    scribble scr;
    size_t   gcount   = 0;
    bool     computed = false;
    for (object_p obj : *guesses)
    {
        id ty = obj->type();
        if (ty == ID_expression || ty == ID_constant || ty == ID_equation)
        {
            settings::SaveNumericalResults snr(true);
            obj = algebraic_p(obj)->evaluate();
            if (!obj)
                return nullptr;
            ty = obj->type();
            computed = true;
        }
        rt.append(obj);
        if (ty == ID_unit)
        {
            obj = unit_p(obj)->value();
            ty = obj->type();
        }
        if (!is_real(ty) && !is_complex(ty))
        {
            rt.type_error();
            return nullptr;
        }
        gcount++;
    }
    list_g gvalues = computed
        ? list::make(guesses->type(), scr.scratch(), scr.growth())
        : +guesses;
    scr.clear();

    // Number of guesses and variables should match
    if (gcount != vcount)
    {
        rt.dimension_error();
        return nullptr;
    }

    // Looks good: loop on equations trying to find one we can solve
    size_t ecount = eqs->items();
    list_g vars   = names;
    list_g eqns   = eqs;

    // While there are variables to solve for
    while (vcount && ecount)
    {
        if (program::interrupted())
        {
            rt.interrupted_error();
            break;              // This will keep current values
        }

        list::iterator vi    = vars->begin();
        list::iterator gi    = gvalues->begin();
        bool           found = false;

        // Loop on all variables, looking for one we can solve for
        for (size_t v = 0; !found && v < vcount; v++)
        {
            algebraic_g varobj = algebraic_p(*vi);
            algebraic_g name = varobj;
            algebraic_g uexpr;
            while (unit_p u = unit::get(varobj))
            {
                varobj = u->value();
                uexpr = u->uexpr();
            }
            symbol_g var = varobj->as_quoted<symbol>();
            if (!var)
            {
                rt.type_error();
                return nullptr;
            }
            record(msolve, "Trying to solve for %t", +var);

            // The variable must be well-defined in exactly one equation
            list::iterator ei = eqns->begin();
            expression_g   def;
            size_t         defidx = 0;
            uint           defs   = 0;
            for (size_t e = 0; !found && e < ecount && defs < 2; e++)
            {
                expression_g eq = expression::get(*ei);
                if (!eq)
                {
                    if (!rt.error())
                        rt.type_error();
                    return nullptr;
                }
                bool defined = eq->is_well_defined(var, false, vars);
                record(msolve, "For %t %+s %t",
                       +var, defined ? "ok" : "ko", +eq);
                if (defined)
                {
                    if (!defs++)
                    {
                        def = eq;
                        defidx = e;
                    }
                }
                ++ei;
            }

            if (defs >= 1 && defs <= 1 + ecount - vcount)
            {
                program_g   pgm    = +def;
                algebraic_p guess  = algebraic_p(*gi);
                algebraic_g solved = solve(pgm, name, guess);
                record(msolve, "Solved %t as %t", +pgm, +solved);
                if (!solved)
                {
                    solver_command_error();
                    if (!rt.error())
                        rt.no_solution_error();
                    return nullptr;
                }

                // That's one variable less to deal with
                vars    = vars->remove(v);
                gvalues = gvalues->remove(v);
                eqns    = eqns->remove(defidx);
                vcount--;
                ecount--;
                found = true;
            }
            ++vi;
            ++gi;
        }

        // This algorithm does not apply, some variables were not found
        if (!found && ecount >= vcount)
        {
            record(msolve, "Could not solve for %t in %t", +vars, +eqns);
            eqns = eqns->map(difference_for_solve);
            found = jacobi_solver(eqns, vars, gvalues);
            ecount = 0;
            vcount = 0;
        }
        if (!found)
        {
            rt.multisolver_variable_error();
            solver_command_error();
            return nullptr;
        }
    }

    list_g result = names->map(recall);
    return result;
}


bool Root::jacobi_solver(list_g &eqs, list_g &vars, list_g &guesses)
// ----------------------------------------------------------------------------
//  Compute a Jacobian when there is cross-talk between variables
// ----------------------------------------------------------------------------
{
    size_t n = vars->items();
    ASSERT("We need more equations than variables" && n <= eqs->items());
    ASSERT("Variable count must match guess count" && n == guesses->items());

    // Memorise depth
    size_t depth = rt.depth();

    // Compute the desired precision
    int            impr  = Settings.SolverImprecision();
    algebraic_g    eps   = algebraic::epsilon(impr);
    algebraic_g    oeps  = decimal::make(101,-2);
    uint           max   = Settings.SolverIterations();
    uint           iter  = 0;
    int            errs  = 0;
    bool           back  = false; // Go backwards
    array_g        j, v, d;
    algebraic_g    magnitude, last, forward;

    record(jsolve, "Solve for %t in %t guesses %t", +vars, +eqs, +guesses);

    while (iter++ < max)
    {
        if (program::interrupted())
        {
            rt.interrupted_error();
            break;              // This will keep current values
        }

        // Set all variables to current value of guesses
        list::iterator gi = guesses->begin();
        for (object_p varo : *vars)
        {
            object_p valo = *gi;
            if (errs)
            {
                if (algebraic_g valg = valo->as_algebraic())
                {
                    // Shuffle slightly around current position
                    algebraic_g pw = pow(oeps, errs) + eps;
                    if (!pw)
                        goto error;
                    valo = pw;
                }
            }
            if (!directory::store_here(varo, valo))
                goto error;
            ++gi;
        }

        // Evaluate all equations at current values of variables
        size_t neqs = 0;
        magnitude = nullptr;
        for (object_p eqo : *eqs)
        {
            expression_p eq = expression::get(eqo);
            if (!eq)
                goto error;
            algebraic_p value = eq->evaluate();
            if (!value)
            {
                // Possibly a transient domain error, try shuffling around
                if (errs++ == 0)
                {
                    if (last)
                    {
                        // Return to a known good position
                        v = v + d;
                        guesses = +v;
                    }
                }
                // Try shuffling around the known good position a few times
                if (errs < 5)
                    continue;
            }
            if (!value || !(neqs >= n || rt.push(value)))
                goto error;
            while (unit_p u = value->as<unit>())
                value = u->value();
            algebraic_g absval = abs::run(value);
            magnitude = magnitude ? magnitude + absval : absval;
            neqs++;
        }
        record(jsolve, "Magnitude is %t", +magnitude);

        // Check if we already found a solution, if so exit
        if (smaller_magnitude(magnitude, eps))
            break;

        // Check if we are going in the wrong direction
        if (last && smaller_magnitude(last, magnitude))
        {
            if (!back)
            {
                record(jsolve, "Worse than  %t, try backwards", +last);
                v = v + d;
                v = v + d;
                guesses = +v;
                forward = magnitude;
                back = true;
                continue;
            }
            back = false;
            last = nullptr;
            if (smaller_magnitude(forward, magnitude))
            {
                // Pick up forward direction if it was better
                record(jsolve, "Forward worse, resume forward", +last);
                v = v - d;
                v = v - d;
                guesses = +v;
                continue;
            }
        }
        last = magnitude;

        // Compute the Jacobi matrix by shifting each variable
        gi = guesses->begin();
        size_t column = 0;
        for (object_g varo : *vars)
        {
            // Put (1+eps)*value into the variable
            object_p valo = *gi;
            algebraic_g val = valo->as_algebraic();
            if (!val)
                goto error;
            algebraic_g dx = val;
            val = val * oeps;
            if (val->is_same_as(*gi))
                val = val + oeps;
            dx = dx - val;
            if (!directory::store_here(varo, +val))
                goto error;

            // Evaluate each expression and subtract current value
            neqs = 0;
            for (object_p eqo : *eqs)
            {
                if (neqs >= n)
                    break;
                expression_p eq = expression::get(eqo);
                if (!eq)
                    goto error;
                algebraic_g now = eq->evaluate();
                algebraic_g dydx = rt.stack(column + n - 1)->as_algebraic();
                dydx = (dydx - now) / dx;
                if (!dydx || !rt.push(+dydx))
                    goto error;
                neqs++;
            }

            // We added a column
            column += n;

            // Restore original value
            valo = *gi;
            if (!directory::store_here(varo, valo))
                goto error;
            ++gi;
        }

        // It's a bit inefficient to create arrays here, but save code space
        j = array::from_stack(n, n, true);
        v = array::from_stack(n, 0);
        d = v / j;
        record(jsolve, "Jacobian %t values %t delta %t", +j, +v, +d);
        v = array_p(+guesses);
        v = v - d;
        if (!v)
            goto error;

        // This is the new guesses
        guesses = +v;
    } // while (iter < max)

    rt.drop(rt.depth() - depth);
    return iter < max;

error:
    rt.drop(rt.depth() - depth);
    return false;
}


expression_p Root::isolate(expression_p eq, symbol_p name)
// ----------------------------------------------------------------------------
//   Attempt to isolate without emitting an error
// ----------------------------------------------------------------------------
{
    settings::SavePrincipalSolution sps(true);
    error_save ers;
    return eq->isolate(name);
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
    algebraic_g &eqobj    = args[2];
    algebraic_g &variable = args[1];
    algebraic_g &guess    = args[0];
    algebraic_g result = solve(eqobj, variable, guess);
    if (!result)
        solver_command_error();
    return result;
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


COMMAND_BODY(MultipleEquationsSolver)
// ----------------------------------------------------------------------------
//   Solve a set of equations one at a time (MROOT)
// ----------------------------------------------------------------------------
//   On DB48X, the underlying engine for `Root` knows how to deal with multiple
//   equations. However, the MROOT command starts with the `EQ` variable
//   instead of the stack (compatibility with HP's implementaiton)
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


COMMAND_BODY(MultipleVariablesSolver)
// ----------------------------------------------------------------------------
//   Solve multiple equations with multiple variables (MSLV)
// ----------------------------------------------------------------------------
//   On DB48x, the underlying engine for `Root` knows how to deal with
//   multiple simultaneous variables, including the case where variables
//   are simultaneously present in multiple equations. On HP50G, this is dealt
//   with by a separate command, `MSLV`, that behaves almost like `ROOT`,
//   except that it leaves equations and variables on the stack.
//   As a result, on DB48x, the only real difference between `ROOT` and `MSLV`
//   is that the latter leaves its input on the stack, and as a result is a
//   command and not an N-ary function
{
    object_p    eo      = rt.stack(2);
    object_p    vo      = rt.stack(1);
    object_p    go      = rt.stack(0);

    algebraic_g eqs     = eo->as_algebraic_or_list();
    algebraic_g vars    = vo->as_algebraic_or_list();
    algebraic_g guesses = go->as_algebraic_or_list();

    if (eqs && vars && guesses)
    {
        algebraic_g result = Root::solve(eqs, vars, guesses);
        if (!result)
            solver_command_error();
        if (rt.top(+result))
            return OK;
    }
    else
    {
        rt.type_error();
    }
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
        if (equation_p eq = obj->as_quoted<equation>())
            obj = eq->value();

        if (list_p lst = obj->as_array_or_list())
        {
            size_t sz = 0;
            if (lst->expand_without_size(&sz))
            {
                rt.roll(sz);
                list_g now = list::list_from_stack(sz);
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
                        rv = expression::make(ID_subtract, rv, -d);
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
    return build(mi, expr);
}


bool SolvingMenu::build(menu_info &mi, list_p expr, bool withcmds)
// ----------------------------------------------------------------------------
//   Build the solving menu for a given expression
// ----------------------------------------------------------------------------
{
    list_g vars   = expr ? expr->names() : nullptr;
    size_t nitems = vars ? vars->items() : 0;
    items_init(mi, nitems + int(withcmds), 3, 1);
    if (!vars)
        return false;

    uint skip = mi.skip;

    // First row: Store variables
    mi.plane  = 0;
    mi.planes = 1;
    if (withcmds)
        menu::items(mi, "EvalEq", ID_EvalEq);
    for (auto name : *vars)
        if (symbol_p sym = name->as<symbol>())
            menu::items(mi, sym, menu::ID_SolvingMenuStore);

    // Second row: Solve for variables
    mi.plane  = 1;
    mi.planes = 2;
    mi.skip   = skip;
    mi.index  = mi.plane * ui.NUM_SOFTKEYS;
    if (withcmds)
        menu::items(mi, "NextEq", ID_NextEq);
    for (auto name : *vars)
        if (symbol_p sym = name->as<symbol>())
            menu::items(mi, sym, menu::ID_SolvingMenuSolve);

    // Third row: Recall variable
    mi.plane  = 2;
    mi.planes = 3;
    mi.skip   = skip;
    mi.index  = mi.plane * ui.NUM_SOFTKEYS;
    if (withcmds)
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
    for (uint k = withcmds ? 1 : 0; k < ui.NUM_SOFTKEYS - (mi.pages > 1); k++)
    {
        ui.marker(k + 0 * ui.NUM_SOFTKEYS, L'▶', true);
        ui.marker(k + 1 * ui.NUM_SOFTKEYS, L'?', false);
        ui.marker(k + 2 * ui.NUM_SOFTKEYS, L'▶', false);
    }

    if (withcmds)
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


static uint solver_menu_index(int key)
// ----------------------------------------------------------------------------
//   Return the menu index in the solver menu
// ----------------------------------------------------------------------------
{
    return key - KEY_F1 + 5 * ui.page() - (FinanceSolverMenu::active() ? 0 : 1);
}


COMMAND_BODY(SolvingMenuRecall)
// ----------------------------------------------------------------------------
//   Recall a variable from the SolvingMenu
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (key >= KEY_F1 && key <= KEY_F6)
    {
        uint index = solver_menu_index(key);
        if (symbol_g sym = expression_variable(index))
            if (object_p value = directory::recall_all(sym, true))
                if (algebraic_p a = value->as_algebraic())
                    if (assignment_p asn = assignment::make(+sym, a))
                        if (rt.push(asn))
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


static bool assign(symbol_r name, algebraic_p value)
// ----------------------------------------------------------------------------
//   Assign a value to a variable and push the corresponding assignment
// ----------------------------------------------------------------------------
{
    if (algebraic_p evalue = algebraic_p(directory::store_here(name, value)))
        if (assignment_p asn = assignment::make(+name, evalue))
            if (rt.push(+asn))
                if (ui.menu_refresh())
                    return true;
    return false;
}


COMMAND_BODY(SolvingMenuStore)
// ----------------------------------------------------------------------------
//   Store a variable from the SolvingMenu
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (key >= KEY_F1 && key <= KEY_F6)
    {
        uint index = solver_menu_index(key);
        if (algebraic_p entry = expression_variable_or_unit(index))
        {
            if (object_p obj = strip(rt.pop()))
            {
                algebraic_g value = obj->as_algebraic();
                if (!value)
                {
                    rt.type_error();
                    return ERROR;
                }

                symbol_g name = entry->as_quoted<symbol>();
                unit_g u = unit::get(entry);

                if (u)
                {
                    name = u->value()->as_quoted<symbol>();
                }
                else if (name)
                {
                    if (object_p var = directory::recall_all(name, false))
                        if (unit_p vu = var->as<unit>())
                            u = vu;
                }

                if (!name)
                {
                    rt.type_error();
                    return ERROR;
                }

                if (u)
                {
                    unit_g vu = unit::get(value);
                    if (vu)
                    {
                        if (!u->convert(vu))
                            return ERROR;
                    }
                    else
                    {
                        value = unit::simple(value, u->uexpr());
                    }
                }

                if (value && assign(name, value))
                    return OK;
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


HELP_BODY(SolvingMenuStore)
// ----------------------------------------------------------------------------
//   Help for storing in a solver variable
// ----------------------------------------------------------------------------
{
    size_t len = 0;
    if (utf8 topic = ui.label_for_function_key(&len))
    {
        static char buf[64];
        snprintf(buf, sizeof(buf), "`%.*s`", int(len), topic);
        return utf8(buf);
    }
    return nullptr;
}


COMMAND_BODY(SolvingMenuSolve)
// ----------------------------------------------------------------------------
//  Solve for a given variable
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (key >= KEY_F1 && key <= KEY_F6)
    {
        uint idx = solver_menu_index(key);
        if (symbol_g sym = expression_variable(idx))
        {
            object_g    obj   = directory::recall_all(sym, false);
            algebraic_g value = obj ? obj->as_algebraic() : nullptr;
            if (!value)
                value = integer::make(0);
            if (algebraic_g eql = expression::current_equation(true, true))
                if (algebraic_g var = expression_variable_or_unit(idx))
                    if (algebraic_g res = Root::solve(eql, var, value))
                        if (rt.push(+res))
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
