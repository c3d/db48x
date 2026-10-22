#ifndef CASHFLOW_H
#define CASHFLOW_H
// ****************************************************************************
//  cashflow.h                                                   DB48X project
// ****************************************************************************
//
//   File Description:
//
//      HP-17bii-style cash-flow modeling (CFLO): NPV and IRR over a list of
//      cash flows with optional repetition counts.
//
//      The active cash-flow list is stored in the reserved `CFData` variable,
//      mirroring the way statistics use `ΣData`. Each entry is either a plain
//      number (count 1) or a two-element list { amount count } for repeated
//      flows (the HP "#TIMES" / Nj feature).
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

#include "command.h"
#include "list.h"
#include "menu.h"


// Computation commands (operate on a list on the stack, or on `CFData`)
COMMAND_DECLARE(NPV, 1);                // [ {cashflows} ] rate% -> NPV
COMMAND_DECLARE(IRR, 0);               // [ {cashflows} ] -> IRR%

// Cash-flow list editing (operate on the `CFData` variable)
COMMAND_DECLARE(CFAdd, 1);             // amount -> (append, count 1)
COMMAND_DECLARE(CFAddTimes, 2);        // amount count -> (append {amount count})
COMMAND_DECLARE(CFDrop, 0);            // -> amount (remove and return last entry)
COMMAND_DECLARE(CFInsert, 2);          // amount index -> (insert before index)
COMMAND_DECLARE(CFClear, 0);           // -> (clear the active cash-flow list)
COMMAND_DECLARE(CFStore, 1);           // name -> (save active list under name)
COMMAND_DECLARE(CFRecall, 1);          // name -> (load named list as active)


struct CashFlowData : command
// ----------------------------------------------------------------------------
//   Helper to access the reserved `CFData` cash-flow list variable
// ----------------------------------------------------------------------------
{
    CashFlowData(id type = ID_CashFlowData) : command(type) {}

    static object_p name();             // The `CFData` variable name object
    static list_p   load();             // Recall the active list (may be null)
    static bool     store(list_g cf);   // Store the active list
    static bool     is_empty(list_p cf);// True if the list has no entries

    // Sum of cf_t / (1 + rate%)^t over the expanded list (t starts at 0)
    static algebraic_p npv(list_p cf, algebraic_r rate_percent);
    // Internal rate of return (percent), or null on failure (error is set)
    static algebraic_p irr(list_p cf);
};


struct CashFlowMenu : menu
// ----------------------------------------------------------------------------
//   HP-17bii-style cash-flow editor menu
// ----------------------------------------------------------------------------
{
    CashFlowMenu(id type = ID_CashFlowMenu) : menu(type) {}

public:
    OBJECT_DECL(CashFlowMenu);
    MENU_DECL(CashFlowMenu);
};

#endif // CASHFLOW_H
