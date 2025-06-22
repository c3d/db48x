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
#include "runtime.h"
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

    // Evaluate upper and lower bound for performance
    {
        error_save ers;
        low = low->evaluate();
        if (!low)
            return nullptr;
        high = high->evaluate();
        if (!high)
            return nullptr;
    }


    // Check if we have symbolic evaluation
    if (Settings.SymbolicIntegration())
    {
        if (expression_g eq = expression::get(+eqobj))
        {
            error_save ers;
            if (expression_g prim = eq->primitive(name))
            {
                algebraic_g lop = prim->substitute(name, low);
                algebraic_g hip = prim->substitute(name, high);
                if (algebraic_p diff = hip - lop)
                    if (algebraic_p eval = diff->evaluate())
                        return eval;
            }
            if (low->is_symbolic() || high->is_symbolic())
                return expression::make(ID_Integrate,
                                        args, arity, ID_expression, true);
        }
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
//
//   For reasons explained in the HP15C advanced function handbook, we perform
//   two changes of variables:
//   1) We change the a-b interval to -1..1 to enable the second change
//         x = (b-a)(u+1)/2 + a
//        dx = (b-a)/2 du
//   2) We avoid sampling uniformly with another variable change:
//         u = 3/2 v - 1/2 v^3
//        du = 3/2 (1 - v^2) dv
{
    // We will run commands below, do not save stack while doing it
    settings::PrepareForProgramEvaluation wilLRunPrograms;

    // Check if the guess is an algebraic or if we need to extract one
    algebraic_g u, du, du2;
    algebraic_g v, dv, dv2, lv, hv;
    algebraic_g x, dx, dx2;
    algebraic_g y, dy, sy, sy2;
    algebraic_g half = decimal::make(5, -1);
    algebraic_g zero = integer::make(0);
    algebraic_g one  = integer::make(1);
    algebraic_g two  = integer::make(2);
    algebraic_g four = integer::make(4);
    algebraic_g pow4;
    record(integrate, "Initial range %t-%t", +lx, +hx);

    // Set independent variable
    save<symbol_g *> iref(expression::independent, &name);
    int              impr = Settings.IntegrationImprecision();
    algebraic_g      eps = algebraic::epsilon(impr);

    // Check initializations happened correctly
    if (!half || !zero || !one || !two  || !four || !eps)
        return nullptr;

    // Select numerical computations (doing this with fraction is slow)
    settings::SaveNumericalResults snr(true);

    // Initial integration step and first trapezoidal step
    dv = two;
    algebraic_g hl2 = (hx - lx) * half;
    sy = zero;
    lv = -one;
    hv = one;
    if (!hl2 || !lv)
        return nullptr;

    // Loop for a maximum number of conversion iterations
    size_t loops = 1;
    int    max   = Settings.IntegrationIterations();

    // Depth of the original stack, to return to after computation
    size_t depth = rt.depth();
    for (int d = -1; d < max && !program::interrupted(); d++)
    {
        sy  = zero;
        dv2 = dv;
        dv = dv * half;
        v   = lv + dv;
        if (!v || !sy || !dv)
            goto error;

        // Compute the sum of f(low + k*i)
        for (uint i = 0; i < loops; i++)
        {
            u = ((v + v + v) - v * v * v) * half; // 3/2v - 1/2v^3
            du = (one - v * v) * dv;              // (1-v^2) dv
            du = (du + du + du) * half;           // 3/2 (1-v^2) * dv
            x = hl2 * (u + one) + lx;             // (b-a) * (u+1) / 2
            dx = hl2 * du;                        // (b-a)/2 du

            // Evaluate equation
            y  = algebraic::evaluate_function(eq, x);

            // Sum elements, and approximate when necessary
            sy = sy + y * dx;
            record(integrate, "[%u:%u] x=%t y=%t sum=%t", d, i, +x, +y, +sy);
            v = v + dv2;
            if (!sy || !x)
                goto error;
        }

        // First iteration simply initializes the first approximation
        if (d < 0)
        {
            if (!rt.push(+sy))
                goto error;
            loops += loops;
            continue;
        }

        // Get P[0]
        y = algebraic_p(rt.stack(d));

        // Compute C[0]
        sy2 = sy + y * half;
        if (!sy2 || !rt.push(+sy2))
            goto error;

        // Prepare 4^i for i=0
        pow4 = four;

        // Loop to compute C[i] for i > 0
        for (int i = 0; i <= d; i++)
        {
            // Compute (C[i] * 4^(i+1) - P[i]) / (4^(i+1)-1)
            x = algebraic_p(rt.stack(d + 1)); // P[i]
            y = algebraic_p(rt.top());        // C[i]
            y = (y * pow4 - x) / (pow4 - one);

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
        for (int i = 0; i <= d + 1; i++)
            rt.stack(off_p - i, rt.stack(off_c - i));

        // Drop P
        rt.drop(off_c);

        // Twice as many items to evaluate in next loop
        loops += loops;
    }

    rt.precision_loss_error();

error:
    rt.drop(rt.depth() - depth);
    return nullptr;
}
