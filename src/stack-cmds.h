#ifndef STACK_CMDS_H
#define STACK_CMDS_H
// ****************************************************************************
//  stack-cmds.h                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//     RPL Stack commands
//
//
//
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

#include "command.h"
#include "dmcp.h"

COMMAND(Dup,1)
// ----------------------------------------------------------------------------
//   Implement the RPL "dup" command, duplicate top of stack
// ----------------------------------------------------------------------------
{
    if (object_g top = rt.top())
        if (rt.push(top))
            return OK;
    return ERROR;
}


COMMAND(DupDup,1)
// ----------------------------------------------------------------------------
//   Implement the RPL "dupdup" command, duplicate top of stack twice
// ----------------------------------------------------------------------------
{
    if (object_g top = rt.top())
        if (rt.push(top) && rt.push(top))
            return OK;
    return ERROR;
}


COMMAND(Dup2,2)
// ----------------------------------------------------------------------------
//   Implement the RPL "dup2" command, duplicate two elements at top of stack
// ----------------------------------------------------------------------------
{
    if (object_g y = rt.stack(1))
        if (object_g x = rt.stack(0))
            if (rt.push(y))
                if (rt.push(x))
                    return OK;
    return ERROR;
}


COMMAND(DupN,~1)
// ----------------------------------------------------------------------------
//   Implement the RPL "DUPN" command, duplicate N elements at top of stack
// ----------------------------------------------------------------------------
{
    uint32_t depth = uint32_arg();
    if (!rt.error() && rt.args(depth+1) && rt.pop())
    {
        for (uint i = 0; i < depth; i++)
            if (object_p obj = rt.stack(depth-1))
                if (!rt.push(obj))
                    return ERROR;
        return OK;
    }
    return ERROR;
}


COMMAND(NDupN,2)
// ----------------------------------------------------------------------------
//   Implement the RPL "NDUPN" command, duplicate an element N times
// ----------------------------------------------------------------------------
{
    uint32_t depth = uint32_arg();
    if (!rt.error())
    {
        object_g count = rt.pop();
        object_g value = rt.pop();
        for (uint i = 0; i < depth; i++)
            if (!rt.push(value))
                return ERROR;
        if (rt.push(count))
            return OK;
    }
    return ERROR;
}


COMMAND(Drop,1)
// ----------------------------------------------------------------------------
//   Implement the RPL "drop" command, remove top of stack
// ----------------------------------------------------------------------------
{
    if (rt.drop())
        return OK;
    return ERROR;
}


COMMAND(Drop2,2)
// ----------------------------------------------------------------------------
//   Implement the Drop2 command, remove two elements from the stack
// ----------------------------------------------------------------------------
{
    if (rt.drop(2))
        return OK;
    return ERROR;
}


COMMAND(DropN,~1)
// ----------------------------------------------------------------------------
//   Implement the DropN command, remove N elements from the stack
// ----------------------------------------------------------------------------
{
    uint32_t depth = uint32_arg();
    if (!rt.error())
        if (rt.args(depth+1))
            if (rt.pop())
                if (rt.drop(depth))
                    return OK;
    return ERROR;
}


COMMAND(Over,2)
// ----------------------------------------------------------------------------
//   Implement the Over command, getting object from level 2
// ----------------------------------------------------------------------------
{
    if (object_p o = rt.stack(1))
        if (rt.push(o))
            return OK;
    return ERROR;
}


COMMAND(Pick,1)
// ----------------------------------------------------------------------------
//   Implement the Pick command, getting from level N
// ----------------------------------------------------------------------------
//  Note that both on HP50G and HP48, LastArg after Pick only returns the
//  pick value, not the picked value (inconsistent with DupN for example)
{
    uint32_t depth = uint32_arg();
    if (!rt.error())
        if (object_p obj = rt.stack(depth))
            if (rt.top(obj))
                return OK;
    return ERROR;
}


COMMAND(Roll,1)
// ----------------------------------------------------------------------------
//   Implement the Roll command, moving objects from high stack level down
// ----------------------------------------------------------------------------
{
    uint32_t depth = uint32_arg();
    if (!rt.error())
        if (rt.pop())
            if (rt.roll(depth))
                return OK;
    return ERROR;
}


COMMAND(RollD,1)
// ----------------------------------------------------------------------------
//   Implement the RollD command, moving objects from first level up
// ----------------------------------------------------------------------------
{
    uint32_t depth = uint32_arg();
    if (!rt.error())
        if (rt.pop())
            if (rt.rolld(depth))
                return OK;
    return ERROR;
}


COMMAND(Rot,3)
// ----------------------------------------------------------------------------
//   Implement the rot command, rotating first three levels of the stack
// ----------------------------------------------------------------------------
{
    if (rt.roll(3))
        return OK;
    return ERROR;
}


COMMAND(UnRot,3)
// ----------------------------------------------------------------------------
//   Implement the unrot command, rotating the first three levels of stack
// ----------------------------------------------------------------------------
{
    if (rt.rolld(3))
        return OK;
    return ERROR;
}


COMMAND(UnPick,2)
// ----------------------------------------------------------------------------
//   Unpick command "pokes" into the stack with 2nd level object
// ----------------------------------------------------------------------------
{
    if (uint32_t depth = uint32_arg())
        if (object_p y = rt.stack(1))
            if (rt.drop(2))
                if (rt.stack(depth, y))
                    return OK;
    return ERROR;
}


COMMAND(Swap,2)
// ----------------------------------------------------------------------------
//   Implement the RPL "swap" command, swap the two top elements
// ----------------------------------------------------------------------------
{
    object_p x = rt.stack(0);
    object_p y = rt.stack(1);
    if (x && y)
    {
        rt.stack(0, y);
        rt.stack(1, x);
        return OK;
    }
    return ERROR;
}


COMMAND(Nip,2)
// ----------------------------------------------------------------------------
//   Implement the RPL "nip" command, remove level 2 of the stack
// ----------------------------------------------------------------------------
{
    if (object_p x = rt.stack(0))
        if (rt.stack(1, x))
            if (rt.drop())
                return OK;
    return ERROR;
}


COMMAND(Pick3,3)
// ----------------------------------------------------------------------------
//   Implement the RPL "pick3" command, duplicating level 3
// ----------------------------------------------------------------------------
{
    if (object_p x = rt.stack(2))
        if (rt.push(x))
            return OK;
    return ERROR;
}


COMMAND(Depth,0)
// ----------------------------------------------------------------------------
//   Return the depth of the stack
// ----------------------------------------------------------------------------
{
    uint depth = rt.depth();
    if (integer_p ti = rt.make<integer>(ID_integer, depth))
        if (rt.push(ti))
            return OK;
    return ERROR;
}


COMMAND(ClearStack,0)
// ----------------------------------------------------------------------------
//   Clear the stack
// ----------------------------------------------------------------------------
{
    if (rt.drop(rt.depth()))
        return OK;
    return ERROR;
}


COMMAND(Clone, 1)
// ----------------------------------------------------------------------------
//   Create a new object of the object on the stack
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
        if (object_p clone = rt.clone(obj))
            if (rt.top(clone))
                return OK;
    return ERROR;
}

#endif // STACK_CMDS_H
