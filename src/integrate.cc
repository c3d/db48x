// ****************************************************************************
//  integrate.cc                                                 DB48X project
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

#include "integrate.h"

#include "algebraic.h"
#include "arithmetic.h"
#include "compare.h"
#include "equations.h"
#include "expression.h"
#include "functions.h"
#include "integer.h"
#include "recorder.h"
#include "settings.h"
#include "symbol.h"
#include "tag.h"

RECORDER(integrate, 16, "Numerical integration");
RECORDER(integrate_error, 16, "Numerical integrationsol");


NFUNCTION_BODY(Integrate)
// ----------------------------------------------------------------------------
//   Numerical integration
// ----------------------------------------------------------------------------
{
    if (arity != 4)
    {
        rt.internal_error();
        return nullptr;
    }
    algebraic_g &variable = args[0];
    algebraic_g &eqobj    = args[1];
    algebraic_g &high     = args[2];
    algebraic_g &low      = args[3];

    if (!eqobj->is_program())
    {
        rt.invalid_equation_error();
        return nullptr;
    }

    record(integrate, "Integrating %t for variable %t in range %t-%t",
           +eqobj, +variable, +low, +high);

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
    if ((eqty != ID_program && eqty != ID_expression) ||
        !name || !low->is_algebraic() || !high->is_algebraic())
    {
        rt.type_error();
        return nullptr;
    }

    // Actual integration
    program_g  eq = program_p(+eqobj);
    algebraic_g i = integrate(eq, name, algebraic_p(+low), algebraic_p(+high));
    if (!i)
        rt.command(object::static_object(object::ID_Integrate));
    return i;
}


algebraic_p integrate(program_g   eq,
                      symbol_g    name,
                      algebraic_g lx,
                      algebraic_g hx)
// ----------------------------------------------------------------------------
//   Romberg algorithm - The core of the integration function
// ----------------------------------------------------------------------------
//   The Romberg algorithm uses two buffers, one keeping the approximations
//   from the previous loop, called P, and one for the current loop, called C.
//   At each step, the size of C is one more than P.
//   In the implementation below, those arrays are on the stack, P above C.
{
    // We will run commands below, do not save stack while doing it
    settings::PrepareForProgramEvaluation wilLRunPrograms;

    // Check if the guess is an algebraic or if we need to extract one
    algebraic_g x, dx, dx2;
    algebraic_g y, dy, sy, sy2;
    algebraic_g one  = integer::make(1);
    algebraic_g two  = integer::make(2);
    algebraic_g four = integer::make(4);
    algebraic_g pow4;
    record(integrate, "Initial range %t-%t", +lx, +hx);

    // Set independent variable
    save<symbol_g *> iref(expression::independent, &name);
    int              prec = (Settings.Precision() -
                             Settings.IntegrationImprecision());
    algebraic_g      eps = decimal::make(1, -prec);

    // Select numerical computations (doing this with fraction is slow)
    settings::SaveNumericalResults snr(true);

    // Initial integration step and first trapezoidal step
    dx              = hx - lx;
    sy              = algebraic::evaluate_function(eq, lx);
    sy2             = algebraic::evaluate_function(eq, hx);
    sy              = (sy + sy2) * dx / two;
    if (!dx || !sy)
        return nullptr;

    // Loop for a maximum number of conversion iterations
    size_t loops = 1;
    uint   max   = Settings.IntegrationIterations();

    // Depth of the original stack, to return to after computation
    size_t depth = rt.depth();
    if (!rt.push(+sy))
        goto error;

    for (uint d = 0; d < max && !program::interrupted(); d++)
    {
        dx2 = dx / two;
        sy  = integer::make(0);
        x   = lx + dx2;
        if (!x || !sy || !dx)
            goto error;

        // Compute the sum of f(low + k*i)
        for (uint i = 0; i < loops; i++)
        {
            if (!algebraic::to_decimal_if_big(x))
                goto error;

            // Evaluate equation
            y  = algebraic::evaluate_function(eq, x);

            // Sum elements, and approximate when necessary
            sy = sy + y;
            if (!algebraic::to_decimal_if_big(sy))
                goto error;
            record(integrate, "[%u:%u] x=%t y=%t sum=%t", d, i, +x, +y, +sy);
            x = x + dx;
            if (!sy || !x)
                goto error;
        }

        // Get P[0]
        y   = algebraic_p(rt.stack(d));

        // Compute C[0]
        sy2 = dx2 * sy + y / two;
        if (!algebraic::to_decimal_if_big(sy2))
            goto error;
        if (!sy2 || !rt.push(+sy2))
            goto error;

        // Prepare 4^i for i=0
        pow4 = four;

        // Loop to compute C[i] for i > 0
        for (uint i = 0; i <= d; i++)
        {
            // Compute (C[i] * 4^(i+1) - P[i]) / (4^(i+1)-1)
            x = algebraic_p(rt.stack(d + 1)); // P[i]
            y = algebraic_p(rt.top());        // C[i]
            y = (y * pow4 - x) / (pow4 - one);

            // If we are starting to get really big numbers, approximate
            if (!algebraic::to_decimal_if_big(y))
                goto error;

            // Compute next power of 4
            pow4 = pow4 * four;

            // Save C[i]
            if (!y || !pow4 || !rt.push(+y))
                goto error;
        }

        // Check if we converged
        if (d > 0)
        {
            // Check if abs(P[i-1] - C[i]) < eps
            y = algebraic_p(rt.top());
            x = algebraic_p(rt.stack(d + 2));
            x = y - x;
            if (smaller_magnitude(x, y*eps) || d == max)
            {
                rt.drop(rt.depth() - depth);
                return y;
            }
        }

        // Copy C to P
        uint off_p = 2 * d + 2; // P[d+1], C[d+2], -1 to get end of array
        uint off_c = d + 1;
        for (size_t i = 0; i <= d + 1; i++)
            rt.stack(off_p - i, rt.stack(off_c - i));

        // Drop P
        rt.drop(off_c);

        // Twice as many items to evaluate in next loop
        loops += loops;
        dx = dx2;
    }

    rt.precision_loss_error();

error:
    rt.drop(rt.depth() - depth);
    return nullptr;
}
