// ****************************************************************************
//  cashflow.cc                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//     HP-17bii-style cash-flow modeling (CFLO): NPV and IRR.
//
//     The active cash-flow list lives in the reserved `CFData` variable, the
//     same way statistics use `ΣData`. Each entry is either a number (a single
//     flow) or a two-element list { amount count } describing `count` identical
//     consecutive flows (the HP "#TIMES" / Nj feature).
//
//     NPV and IRR also accept a list directly on the stack so they can be used
//     programmatically, e.g. `{ -20000 5000 7000 9000 } 10 NPV`.
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

#include "cashflow.h"

#include "arithmetic.h"
#include "compare.h"
#include "decimal.h"
#include "finance.h"
#include "integer.h"
#include "list.h"
#include "runtime.h"
#include "variables.h"


// ============================================================================
//
//   Cash-flow list access (the reserved `CFData` variable)
//
// ============================================================================

object_p CashFlowData::name()
// ----------------------------------------------------------------------------
//   Return the name object for the `CFData` variable
// ----------------------------------------------------------------------------
{
    return command::static_object(ID_CashFlowData);
}


list_p CashFlowData::load()
// ----------------------------------------------------------------------------
//   Recall the active cash-flow list (nullptr if undefined)
// ----------------------------------------------------------------------------
{
    if (object_p obj = directory::recall_all(name(), false))
        return obj->as_array_or_list();
    return nullptr;
}


bool CashFlowData::store(list_g cf)
// ----------------------------------------------------------------------------
//   Store the active cash-flow list in the `CFData` variable
// ----------------------------------------------------------------------------
{
    if (!cf)
        return false;
    if (directory *dir = rt.variables(0))
        return dir->store(name(), +cf);
    return false;
}


bool CashFlowData::is_empty(list_p cf)
// ----------------------------------------------------------------------------
//   True when the cash-flow list is missing or has no entries
// ----------------------------------------------------------------------------
{
    return !cf || cf->begin() == cf->end();
}



// ============================================================================
//
//   Cash-flow numerical helpers
//
// ============================================================================

static bool cf_entry(object_p el, algebraic_g &amount, ularge &count)
// ----------------------------------------------------------------------------
//   Decode a cash-flow list entry into an amount and a repetition count
// ----------------------------------------------------------------------------
//   An entry is either a number (count 1) or a list { amount count }
{
    count = 1;
    if (list_p sub = el->as_array_or_list())
    {
        list::iterator it  = sub->begin();
        list::iterator end = sub->end();
        if (it == end)
        {
            rt.type_error();
            return false;
        }
        amount = (*it++)->as_algebraic();
        if (!amount)
        {
            rt.type_error();
            return false;
        }
        if (it != end)
        {
            uint n = (*it)->as_uint32(0, true);
            if (rt.error())
                return false;
            if (n < 1)
            {
                rt.value_error();
                return false;
            }
            count = n;
        }
        return true;
    }

    amount = el->as_algebraic();
    if (!amount)
    {
        rt.type_error();
        return false;
    }
    return true;
}


static algebraic_g npv_fraction(list_p cf, algebraic_r rfrac)
// ----------------------------------------------------------------------------
//   NPV given a periodic rate expressed as a fraction (e.g. 0.10 for 10%)
// ----------------------------------------------------------------------------
//   Returns nullptr without an error when the rate is outside the domain
{
    algebraic_g one   = integer::make(1);
    algebraic_g onepr = one + rfrac;
    if (!onepr || onepr->is_zero(false) || onepr->is_negative(false))
        return nullptr;

    // Work in decimal to avoid blow-up of exact rational powers
    algebraic::to_decimal(onepr);

    algebraic_g sum      = integer::make(0);
    algebraic_g discount = one;                 // (1 + r) ^ t, t starts at 0
    for (object_p el : *cf)
    {
        algebraic_g amount;
        ularge      count;
        if (!cf_entry(el, amount, count))
            return nullptr;
        for (ularge i = 0; i < count; i++)
        {
            sum      = sum + amount / discount;
            discount = discount * onepr;
            if (!sum || !discount)
                return nullptr;
        }
    }
    return sum;
}


static algebraic_g npv_derivative(list_p cf, algebraic_r rfrac)
// ----------------------------------------------------------------------------
//   d(NPV)/dr = sum of -t * cf_t / (1 + r) ^ (t + 1)
// ----------------------------------------------------------------------------
{
    algebraic_g one   = integer::make(1);
    algebraic_g onepr = one + rfrac;
    if (!onepr || onepr->is_zero(false) || onepr->is_negative(false))
        return nullptr;
    algebraic::to_decimal(onepr);

    algebraic_g sum      = integer::make(0);
    algebraic_g discount = onepr;               // (1 + r) ^ (t + 1)
    ularge      t        = 0;
    for (object_p el : *cf)
    {
        algebraic_g amount;
        ularge      count;
        if (!cf_entry(el, amount, count))
            return nullptr;
        for (ularge i = 0; i < count; i++, t++)
        {
            if (t != 0)
            {
                algebraic_g tt = integer::make(t);
                sum = sum - tt * amount / discount;
                if (!sum)
                    return nullptr;
            }
            discount = discount * onepr;
            if (!discount)
                return nullptr;
        }
    }
    return sum;
}


static bool cf_has_mixed_signs(list_p cf)
// ----------------------------------------------------------------------------
//   IRR requires at least one positive and one negative cash flow
// ----------------------------------------------------------------------------
{
    bool pos = false;
    bool neg = false;
    for (object_p el : *cf)
    {
        algebraic_g amount;
        ularge      count;
        if (!cf_entry(el, amount, count))
            return false;
        if (amount->is_negative(false))
            neg = true;
        else if (!amount->is_zero(false))
            pos = true;
    }
    return pos && neg;
}


static algebraic_g irr_newton(list_p cf)
// ----------------------------------------------------------------------------
//   Newton-Raphson search for the IRR (as a fraction), or nullptr on failure
// ----------------------------------------------------------------------------
{
    algebraic_g r   = decimal::make(10, -2);    // initial guess: 0.10 (10%)
    algebraic_g tol = decimal::make(1, -12);
    algebraic_g one = integer::make(1);

    for (int iter = 0; iter < 60; iter++)
    {
        algebraic_g f = npv_fraction(cf, r);
        if (!f)
            return nullptr;
        if (f->is_zero(false) || smaller_magnitude(f, tol))
            return r;

        algebraic_g df = npv_derivative(cf, r);
        if (!df || df->is_zero(false))
            return nullptr;

        algebraic_g step = f / df;
        algebraic_g next = r - step;
        if (!next)
            return nullptr;

        algebraic_g onepn = one + next;
        if (onepn->is_zero(false) || onepn->is_negative(false))
            return nullptr;
        if (smaller_magnitude(step, tol))
            return next;
        r = next;
    }
    return nullptr;
}


static algebraic_g irr_bisection(list_p cf)
// ----------------------------------------------------------------------------
//   Robust fallback: bisection between -99.9% and 1000%
// ----------------------------------------------------------------------------
{
    algebraic_g lo  = decimal::make(-999, -3);  // -0.999  (-99.9%)
    algebraic_g hi  = integer::make(10);        //  10.0   (1000%)
    algebraic_g two = integer::make(2);
    algebraic_g tol = decimal::make(1, -12);

    algebraic_g flo = npv_fraction(cf, lo);
    algebraic_g fhi = npv_fraction(cf, hi);
    if (!flo || !fhi)
        return nullptr;
    if (flo->is_zero(false))
        return lo;
    if (fhi->is_zero(false))
        return hi;

    bool nlo = flo->is_negative(false);
    bool nhi = fhi->is_negative(false);
    if (nlo == nhi)
        return nullptr;                         // no sign change in range

    for (int iter = 0; iter < 200; iter++)
    {
        algebraic_g mid  = (lo + hi) / two;
        algebraic_g fmid = npv_fraction(cf, mid);
        if (!fmid)
            return nullptr;
        if (fmid->is_zero(false) || smaller_magnitude(fmid, tol))
            return mid;
        if (fmid->is_negative(false) == nlo)
            lo = mid;
        else
            hi = mid;
    }
    return (lo + hi) / two;
}


algebraic_p CashFlowData::npv(list_p cf, algebraic_r rate_percent)
// ----------------------------------------------------------------------------
//   Net present value for a periodic rate given as a percentage
// ----------------------------------------------------------------------------
{
    algebraic_g hundred = integer::make(100);
    algebraic_g rfrac   = rate_percent / hundred;
    algebraic_g one     = integer::make(1);
    algebraic_g onepr   = one + rfrac;
    if (!onepr || onepr->is_zero(false) || onepr->is_negative(false))
    {
        rt.domain_error();
        return nullptr;
    }
    return npv_fraction(cf, rfrac);
}


algebraic_p CashFlowData::irr(list_p cf)
// ----------------------------------------------------------------------------
//   Internal rate of return as a percentage, or nullptr on failure
// ----------------------------------------------------------------------------
{
    bool mixed = cf_has_mixed_signs(cf);
    if (rt.error())
        return nullptr;
    if (!mixed)
    {
        rt.no_irr_error();
        return nullptr;
    }

    algebraic_g r = irr_newton(cf);
    if (!r && !rt.error())
        r = irr_bisection(cf);
    if (!r)
    {
        if (!rt.error())
            rt.no_irr_error();
        return nullptr;
    }

    algebraic_g hundred = integer::make(100);
    return r * hundred;
}



// ============================================================================
//
//   Computation commands
//
// ============================================================================

COMMAND_BODY(NPV)
// ----------------------------------------------------------------------------
//   Net present value: [ {cashflows} ] rate% -> NPV
// ----------------------------------------------------------------------------
{
    object_p rate_obj = rt.top();
    if (!rate_obj)
        return ERROR;
    algebraic_g rate = rate_obj->as_algebraic();
    if (!rate)
    {
        rt.type_error();
        return ERROR;
    }

    bool   explicit_list = false;
    list_g cf;
    if (object_p below = rt.stack(1))
        if (list_p l = below->as_array_or_list())
        {
            cf            = l;
            explicit_list = true;
        }
    if (!cf)
        cf = CashFlowData::load();
    if (CashFlowData::is_empty(cf))
    {
        rt.empty_cash_flows_error();
        return ERROR;
    }

    algebraic_g value = CashFlowData::npv(cf, rate);
    if (!value)
        return ERROR;
    FinanceSolverMenu::round(value);

    if (!rt.drop(explicit_list ? 2 : 1))
        return ERROR;
    return rt.push(+value) ? OK : ERROR;
}


COMMAND_BODY(IRR)
// ----------------------------------------------------------------------------
//   Internal rate of return: [ {cashflows} ] -> IRR%
// ----------------------------------------------------------------------------
{
    bool   explicit_list = false;
    list_g cf;
    if (object_p top = rt.top())
        if (list_p l = top->as_array_or_list())
        {
            cf            = l;
            explicit_list = true;
        }
    if (!cf)
        cf = CashFlowData::load();
    if (CashFlowData::is_empty(cf))
    {
        rt.empty_cash_flows_error();
        return ERROR;
    }

    algebraic_g value = CashFlowData::irr(cf);
    if (!value)
        return ERROR;

    if (explicit_list && !rt.drop())
        return ERROR;
    return rt.push(+value) ? OK : ERROR;
}



// ============================================================================
//
//   Cash-flow list editing commands (operate on `CFData`)
//
// ============================================================================

COMMAND_BODY(CFAdd)
// ----------------------------------------------------------------------------
//   Append a single cash flow to the active list
// ----------------------------------------------------------------------------
{
    object_p obj = rt.top();
    if (!obj)
        return ERROR;
    algebraic_g amount = obj->as_algebraic();
    if (!amount)
    {
        rt.type_error();
        return ERROR;
    }

    list_g cf = CashFlowData::load();
    if (!cf)
        cf = list::make(ID_list, nullptr, 0);
    cf = cf ? cf->append(+amount) : nullptr;
    if (!cf || !CashFlowData::store(cf))
        return ERROR;
    rt.drop();
    return OK;
}


COMMAND_BODY(CFAddTimes)
// ----------------------------------------------------------------------------
//   Append a repeated cash flow { amount count } (the HP "#TIMES" feature)
// ----------------------------------------------------------------------------
{
    object_p count_obj  = rt.top();
    object_p amount_obj = rt.stack(1);
    if (!count_obj || !amount_obj)
        return ERROR;
    algebraic_g amount = amount_obj->as_algebraic();
    if (!amount)
    {
        rt.type_error();
        return ERROR;
    }
    uint count = count_obj->as_uint32(0, true);
    if (rt.error())
        return ERROR;
    if (count < 1)
    {
        rt.value_error();
        return ERROR;
    }

    integer_g cval  = integer::make(count);
    list_g    entry = list::make(ID_list, amount, cval);
    if (!entry)
        return ERROR;

    list_g cf = CashFlowData::load();
    if (!cf)
        cf = list::make(ID_list, nullptr, 0);
    cf = cf ? cf->append(+entry) : nullptr;
    if (!cf || !CashFlowData::store(cf))
        return ERROR;
    rt.drop(2);
    return OK;
}


COMMAND_BODY(CFDrop)
// ----------------------------------------------------------------------------
//   Remove the last cash-flow entry and return it
// ----------------------------------------------------------------------------
{
    list_g cf = CashFlowData::load();
    if (CashFlowData::is_empty(cf))
    {
        rt.empty_cash_flows_error();
        return ERROR;
    }

    size_t n = cf->items();
    object_g last    = cf->at(n - 1);
    object_g removed = rt.clone(last);
    list_g   rebuilt = cf->remove(n - 1, 1);
    if (!removed || !rebuilt || !CashFlowData::store(rebuilt))
        return ERROR;
    return rt.push(+removed) ? OK : ERROR;
}


COMMAND_BODY(CFInsert)
// ----------------------------------------------------------------------------
//   Insert a cash flow before a 1-based index: amount index ->
// ----------------------------------------------------------------------------
{
    object_p idx_obj = rt.top();
    object_p amt_obj = rt.stack(1);
    if (!idx_obj || !amt_obj)
        return ERROR;
    algebraic_g amount = amt_obj->as_algebraic();
    if (!amount)
    {
        rt.type_error();
        return ERROR;
    }
    uint index = idx_obj->as_uint32(0, true);
    if (rt.error())
        return ERROR;
    if (index < 1)
    {
        rt.value_error();
        return ERROR;
    }

    list_g cf = CashFlowData::load();
    if (!cf)
        cf = list::make(ID_list, nullptr, 0);

    size_t n   = cf->items();
    size_t pos = index - 1;                     // list::insert is 0-based
    if (pos > n)
        pos = n;                                // clamp to append at the end
    cf = cf ? cf->insert(+amount, pos) : nullptr;
    if (!cf || !CashFlowData::store(cf))
        return ERROR;
    rt.drop(2);
    return OK;
}


COMMAND_BODY(CFClear)
// ----------------------------------------------------------------------------
//   Clear the active cash-flow list
// ----------------------------------------------------------------------------
{
    list_g empty = list::make(ID_list, nullptr, 0);
    if (!empty || !CashFlowData::store(empty))
        return ERROR;
    return OK;
}


COMMAND_BODY(CFStore)
// ----------------------------------------------------------------------------
//   Save the active cash-flow list under a name: 'name' ->
// ----------------------------------------------------------------------------
{
    object_p nameobj = rt.top();
    if (!nameobj)
        return ERROR;
    id nty = nameobj->type();
    if (nty != ID_symbol && nty != ID_text)
    {
        rt.type_error();
        return ERROR;
    }

    list_g cf = CashFlowData::load();
    if (CashFlowData::is_empty(cf))
    {
        rt.empty_cash_flows_error();
        return ERROR;
    }

    if (!directory::store_here(nameobj, +cf))
        return ERROR;
    rt.drop();
    return OK;
}


COMMAND_BODY(CFRecall)
// ----------------------------------------------------------------------------
//   Load a named list as the active cash-flow list: 'name' ->
// ----------------------------------------------------------------------------
{
    object_p nameobj = rt.top();
    if (!nameobj)
        return ERROR;
    object_p val = directory::recall_all(nameobj, true);
    if (!val)
        return ERROR;
    list_p cf = val->as_array_or_list();
    if (!cf)
    {
        rt.type_error();
        return ERROR;
    }
    if (!CashFlowData::store(cf))
        return ERROR;
    rt.drop();
    return OK;
}



// ============================================================================
//
//   Cash-flow editor menu
//
// ============================================================================

MENU_BODY(CashFlowMenu)
// ----------------------------------------------------------------------------
//   Build the HP-17bii-style cash-flow editor menu
// ----------------------------------------------------------------------------
{
    uint items = menu::count("CF+",    ID_CFAdd,
                             "CFxN",   ID_CFAddTimes,
                             "CF-",    ID_CFDrop,
                             "Ins",    ID_CFInsert,
                             "Clr",    ID_CFClear,
                             "Data",   ID_CashFlowData,
                             "NPV",    ID_NPV,
                             "IRR",    ID_IRR,
                             "STO",    ID_CFStore,
                             "RCL",    ID_CFRecall);
    menu::items_init(mi, items);
    menu::items(mi, "CF+",    ID_CFAdd,
                    "CFxN",   ID_CFAddTimes,
                    "CF-",    ID_CFDrop,
                    "Ins",    ID_CFInsert,
                    "Clr",    ID_CFClear,
                    "Data",   ID_CashFlowData,
                    "NPV",    ID_NPV,
                    "IRR",    ID_IRR,
                    "STO",    ID_CFStore,
                    "RCL",    ID_CFRecall);
    return true;
}
