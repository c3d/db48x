// ****************************************************************************
//  finance.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of finance operations
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2025 Christophe de Dinechin <christophe@dinechin.org>
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

#include "finance.h"

#include "equations.h"
#include "expression.h"
#include "solve.h"
#include "variables.h"
#include "user_interface.h"
#include "tag.h"


expression_p FinanceSolverMenu::equation()
// ----------------------------------------------------------------------------
//   Return the equation for the Time Value of Money module
// ----------------------------------------------------------------------------
{
    bool beg = Settings.TVMPayAtBeginningOfPeriod();
    cstring name = beg ? "TVMBeg" : "TVMEnd";
    if (equation_p eq = equation::lookup(name))
        if (object_p value = eq->value())
            if (expression_p expr = value->as<expression>())
                return expr;
    if (!rt.error())
        rt.invalid_tvm_equation_error();
    return nullptr;
}


bool FinanceSolverMenu::active()
// ----------------------------------------------------------------------------
//   Return true if the Finance solver is active
// ----------------------------------------------------------------------------
{
    if (menu_p m = ui.menu())
        if (m->type() == ID_FinanceSolverMenu)
            return true;
    return false;
}


bool FinanceSolverMenu::round(algebraic_g &value)
// ----------------------------------------------------------------------------
//   Apply finance rounding to TVM values
// ----------------------------------------------------------------------------
{
    if (value && value->is_decimal())
    {
        value = decimal_p(+value)->round(-large(Settings.FinanceRounding()));
        return value;
    }
    return false;
}


static cstring tvm_vars[] = { "PYr", "n", "I%Yr", "Pmt", "FV", "PV" };
static uint tvm_value[] = { 12, 12, 3, 0, 0, 0 };


static void tvm_init()
// ----------------------------------------------------------------------------
//   Initial value for TVM variables
// ----------------------------------------------------------------------------
{
    for (uint i = 0; i < sizeof(tvm_vars)/sizeof(tvm_vars[0]); i++)
        if (symbol_g sym = symbol::make(tvm_vars[i]))
            if (!directory::recall_all(sym, false))
                if (integer_g ival = integer::make(tvm_value[i]))
                    directory::store_here(sym, ival);
}


static algebraic_p tvm_variable(cstring name)
// ----------------------------------------------------------------------------
//   Helper to recall a variable by name
// ----------------------------------------------------------------------------
{
    if (symbol_p sym = symbol::make(name))
        if (object_p exist = directory::recall_all(sym, true))
            if (algebraic_p value = exist->as_algebraic())
                return value;
    return nullptr;
}


COMMAND_BODY(TVMAmort)
// ----------------------------------------------------------------------------
//   Amortize payments
// ----------------------------------------------------------------------------
{
    object_p num = rt.top();
    uint n = num->as_uint32(0, true);
    if (!rt.error())
    {
        algebraic_g iyr = tvm_variable("I%Yr");
        algebraic_g pyr = tvm_variable("PYr");
        algebraic_g pmt = tvm_variable("Pmt");
        algebraic_g pv  = tvm_variable("PV");
        algebraic_g ipp = integer::make(100);
        ipp = iyr / pyr / ipp;

        if (!rt.error())
        {
            algebraic_g principal = integer::make(0);
            algebraic_g interest  = integer::make(0);
            algebraic_g balance   = pv;

            algebraic_g mi, mp;

            for (uint i = 0; i < n; i++)
            {
                if (i || Settings.TVMPayAtEndOfPeriod())
                    mi = -balance * ipp; // Monthly interest
                else
                    mi = integer::make(0);
                mp = pmt - mi;

                interest = interest + mi;
                principal = principal + mp;
                balance = balance + mp;
            }

            FinanceSolverMenu::round(principal);
            FinanceSolverMenu::round(interest);
            FinanceSolverMenu::round(balance);

            tag_g ptag = tag::make("Principal", +principal);
            tag_g itag = tag::make("Interest", +interest);
            tag_g btag = tag::make("Balance", +balance);

            if (ptag && itag && btag)
                if (rt.top(+ptag) && rt.push(+itag) && rt.push(+btag))
                    return OK;
        }
    }

    return ERROR;
}


COMMAND_BODY(TVMAmortTable)
// ----------------------------------------------------------------------------
//   Amortize payments table
// ----------------------------------------------------------------------------
{
    object_p num = rt.top();
    int first = 1, count = 1, step = 1;
    if (list_g args = num->as_array_or_list())
    {
        list::iterator it  = args->begin();
        list::iterator end = args->end();
        if (it != end)
        {
            if (algebraic_g fst = (*it++)->as_algebraic())
                first = fst->as_int32(1, false);
            if (it != end && !rt.error())
            {
                if (algebraic_g cnt = (*it++)->as_algebraic())
                    count = cnt->as_int32(1, false);
                if (it != end && !rt.error())
                {
                    if (algebraic_g stp = (*it++)->as_algebraic())
                        step = stp->as_int32(1, false);
                }
            }

        }
    }
    else
    {
        count = num->as_uint32(1, true);
    }

    // If the last is zero, use the value of the `n` variabe
    if (count < 0 && !rt.error())
    {
        if (algebraic_p nval = tvm_variable("n"))
            count = nval->as_int32(1, true);
    }
    if (!rt.error())
    {
        algebraic_g iyr = tvm_variable("I%Yr");
        algebraic_g pyr = tvm_variable("PYr");
        algebraic_g pmt = tvm_variable("Pmt");
        algebraic_g pv  = tvm_variable("PV");
        algebraic_g ipp = integer::make(100);
        ipp = iyr / pyr / ipp;

        if (!rt.error())
        {
            algebraic_g principal = integer::make(0);
            algebraic_g interest  = integer::make(0);
            algebraic_g balance   = pv;

            algebraic_g mi, mp;
            scribble    ascr;

            int         last = first + count;
            int         s    = 1;
            for (int i = 1; i < last; i++)
            {
                if (i || Settings.TVMPayAtEndOfPeriod())
                    mi = -balance * ipp; // Monthly interest
                else
                    mi = integer::make(0);
                mp = pmt - mi;

                interest = interest + mi;
                principal = principal + mp;
                balance = balance + mp;

                if (i >= first)
                {
                    if (++s > step)
                    {
                        algebraic_g p = principal;
                        algebraic_g i = interest;
                        algebraic_g b = balance;
                        FinanceSolverMenu::round(p);
                        FinanceSolverMenu::round(i);
                        FinanceSolverMenu::round(b);
                        array_p row = array_p(list::make(ID_array, i, p, b));
                        rt.append(row);
                        s = 1;
                        interest = integer::make(0);
                        principal = integer::make(0);
                    }
                }
            }

            array_p result = array_p(list::make(ID_array,
                                                ascr.scratch(), ascr.growth()));
            if (result && rt.top(result))
                return OK;
        }
    }

    return ERROR;
}


COMMAND_BODY(TVMRoot)
// ----------------------------------------------------------------------------
//    Solve for a given variable
// ----------------------------------------------------------------------------
{
    symbol_g sym = rt.top()->as_quoted<symbol>();
    if (!sym)
    {
        rt.type_error();
        return ERROR;
    }

    for (uint i = 0; i < sizeof(tvm_vars)/sizeof(tvm_vars[0]); i++)
    {
        if (sym->matches(tvm_vars[i]))
        {
            if (program_g eq = FinanceSolverMenu::equation())
            {
                tvm_init();
                algebraic_g name     = +sym;
                algebraic_g value;
                if (object_p exist = directory::recall_all(sym, false))
                    if (algebraic_p eval = exist->as_algebraic())
                        value = eval;
                if (!value)
                    value = integer::make(0);
                value = Root::solve(eq, name, value);
                FinanceSolverMenu::round(value);
                value = assignment::make(name, value);
                return value && rt.top(value) ? OK : ERROR;
            }
        }
    }

    rt.invalid_tvm_variable_error();
    return ERROR;
}


COMMAND_BODY(TVMEquation)
// ----------------------------------------------------------------------------
//    Return the equation used for solving
// ----------------------------------------------------------------------------
{
    if (expression_p eq = FinanceSolverMenu::equation())
        if (rt.push(eq))
            return OK;
    return ERROR;
}


MENU_BODY(FinanceSolverMenu)
// ----------------------------------------------------------------------------
//   Process the MENU command for FinanceSolverMenu
// ----------------------------------------------------------------------------
{
    tvm_init();
    if (expression_p eq = FinanceSolverMenu::equation())
        return SolvingMenu::build(mi, eq, false);
    return false;
}
