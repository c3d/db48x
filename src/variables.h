#ifndef VARIABLES_H
#define VARIABLES_H
// ****************************************************************************
//  variables.h                                                   DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Operations on variables
//
//    Global variables are stored in directory objects
//    Local variables are stored just above the stack
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
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
//
// Payload format:
//
//   A directory is represented in memory as follows:
//   - The type ID (one byte, ID_directory)
//   - The total length of the directory
//   - For each entry:
//     * An object for the name, normally an ID_symbol
//     * An object for the content
//
//   This organization makes it possible to put names or values from directories
//   directly on the stack.
//
//   Unlike the HP48, the names can be something else than symbols.
//   This is used notably
//
//   Searching through a directory is done using a linear search, but given the
//   small number of objects typically expected in a calculator, this should be
//   fine. Note that local variables, which are more important for the
//   performance of programs.
//
//   Catalogs are the only mutable RPL objects.
//   They can change when objects are stored or purged.

#include "list.h"
#include "runtime.h"
#include "command.h"
#include "menu.h"


GCP(directory);

struct directory : list
// ----------------------------------------------------------------------------
//   Representation of a directory
// ----------------------------------------------------------------------------
{
    directory(id type = ID_directory): list(type, nullptr, 0)
    {}

    directory(id type, gcbytes bytes, size_t len): list(type, bytes, len)
    { }

    static size_t required_memory(id i)
    {
        return leb128size(i) + leb128size(0);
    }

    static size_t required_memory(id i, gcbytes UNUSED bytes, size_t len)
    {
        return text::required_memory(i, bytes, len);
    }

    static list *make(byte_p bytes, size_t len)
    {
        return (list *) text::make(bytes, len);
    }

    bool store(object_g name, object_g value);
    // ------------------------------------------------------------------------
    //    Store an object in the directory
    // ------------------------------------------------------------------------

    static bool update(object_p name, object_p value);
    // ------------------------------------------------------------------------
    //    Update an existing name
    // ------------------------------------------------------------------------

    object_p recall(object_p name) const;
    // ------------------------------------------------------------------------
    //    Check if a name exists in the directory, return value ptr if it does
    // ------------------------------------------------------------------------

    static object_p recall_all(object_p name, bool report_missing);
    // ------------------------------------------------------------------------
    //    Check if a name exists in the directory, return value ptr if it does
    // ------------------------------------------------------------------------

    static bool store_here(object_p name, object_p value);
    // ------------------------------------------------------------------------
    //    Store in the current directory, or fail if it does not exist
    // ------------------------------------------------------------------------

    object_p lookup(object_p name) const;
    // ------------------------------------------------------------------------
    //    Check if a name exists in the directory, return name ptr if it does
    // ------------------------------------------------------------------------

    size_t purge(object_p name);
    // ------------------------------------------------------------------------
    //   Purge an entry from the directory, return purged size
    // ------------------------------------------------------------------------

    static size_t purge_all(object_p name);
    // ------------------------------------------------------------------------
    //   Purge an entry from the directory and parents
    // ------------------------------------------------------------------------

    size_t count() const
    // ------------------------------------------------------------------------
    //   Return the number of variables in the directory
    // ------------------------------------------------------------------------
    {
        return enumerate(nullptr, nullptr);
    }

    object_p name(uint element) const;
    // ------------------------------------------------------------------------
    //   Return the n-th name in the directory
    // ------------------------------------------------------------------------

    object_p value(uint element) const;
    // ------------------------------------------------------------------------
    //   Return the n-th value in the directory
    // ------------------------------------------------------------------------

    bool find(uint element, object_p &name, object_p &value) const;
    // ------------------------------------------------------------------------
    //   Return the n-th value in the directory
    // ------------------------------------------------------------------------


    typedef bool (*enumeration_fn)(object_p name, object_p obj, void *arg);
    size_t enumerate(enumeration_fn callback, void *arg) const;
    // ------------------------------------------------------------------------
    //   Enumerate all the variables in the directory, return count of true
    // ------------------------------------------------------------------------

    static bool render_name(object_p name, object_p obj, void *renderer_ptr);
    // ------------------------------------------------------------------------
    //   Render an entry in the directory
    // ------------------------------------------------------------------------

    static list_p path(id type = ID_list);
    // ------------------------------------------------------------------------
    //   Return the current directory path
    // ------------------------------------------------------------------------

    result enter() const;
    // ------------------------------------------------------------------------
    //   Enter a directory
    // ------------------------------------------------------------------------

    bool is_valid_name(object_p obj) const;
    // ------------------------------------------------------------------------
    //   Check if something is a valid symbol
    // ------------------------------------------------------------------------

public:
    OBJECT_DECL(directory);
    PARSE_DECL(directory);
    RENDER_DECL(directory);

private:
    static void adjust_sizes(directory_r dir, int delta);
};


COMMAND_DECLARE(Sto, 2);
COMMAND_DECLARE(Rcl, 1);
COMMAND_DECLARE(StoreAdd, 2);
COMMAND_DECLARE(StoreSub, 2);
COMMAND_DECLARE(StoreMul, 2);
COMMAND_DECLARE(StoreDiv, 2);
COMMAND_DECLARE(RecallAdd, 2);
COMMAND_DECLARE(RecallSub, 2);
COMMAND_DECLARE(RecallMul, 2);
COMMAND_DECLARE(RecallDiv, 2);
COMMAND_DECLARE(Increment, 1);
COMMAND_DECLARE(Decrement, 1);
COMMAND_DECLARE_SPECIAL(Copy, command, 2,
                        PREC_DECL(WHERE);
                        static bool can_be_symbolic(uint a) { return a==0; });
COMMAND_DECLARE(Purge,1);
COMMAND_DECLARE(PurgeAll,1);
COMMAND_DECLARE(Vars, 0);
COMMAND_DECLARE(TVars, 1);

COMMAND_DECLARE(Mem,0);
COMMAND_DECLARE(FreeMemory,0);
COMMAND_DECLARE(SystemMemory,0);
COMMAND_DECLARE(GarbageCollect,0);

COMMAND_DECLARE(home,0);                // Return to home directory
COMMAND_DECLARE(CurrentDirectory,0);    // Return the current directory
COMMAND_DECLARE(path,0);                // Return a list describing current path
COMMAND_DECLARE(crdir,1);               // Create a directory
COMMAND_DECLARE(updir,0);               // Move one directory up
COMMAND_DECLARE(pgdir,1);               // Purge directory


struct VariablesMenu : menu
// ----------------------------------------------------------------------------
//   The variables menu is a bit special
// ----------------------------------------------------------------------------
//   The VariablesMenu shows variables in the current menu
//   For each variable, the function key evaluates it, shift recalls it,
//   and xshift stores it. In program mode, the function key shows the name
//   for evaluation purpose, and shifted, shows it between quotes
{
    VariablesMenu(id type = ID_VariablesMenu) : menu(type) {}

    static uint count_variables();
    static void list_variables(info &mi);

public:
    OBJECT_DECL(VariablesMenu);
    MENU_DECL(VariablesMenu);
};

COMMAND_DECLARE_INSERT(VariablesMenuExecute,~0);
COMMAND_DECLARE_INSERT(VariablesMenuRecall,0);
COMMAND_DECLARE_INSERT(VariablesMenuStore,1);



// ============================================================================
//
//   Flag commands
//
// ============================================================================

COMMAND_DECLARE(SetFlag,1);
COMMAND_DECLARE(ClearFlag,1);
COMMAND_DECLARE(FlipFlag,1);
COMMAND_DECLARE(TestFlagSet,1);
COMMAND_DECLARE(TestFlagClear,1);
COMMAND_DECLARE(TestFlagClearThenClear,1);
COMMAND_DECLARE(TestFlagClearThenSet,1);
COMMAND_DECLARE(TestFlagSetThenClear,1);
COMMAND_DECLARE(TestFlagSetThenSet,1);
COMMAND_DECLARE(FlagsToBinary,0);
COMMAND_DECLARE(BinaryToFlags,1);

#endif // VARIABLES_H
