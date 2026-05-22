// ****************************************************************************
//  sim-rpl.cpp                                                   DB48X project
// ****************************************************************************
//
//   File Description:
//
//     The thread running the simulator
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

#include "sim-rpl.h"

#include "dmcp.h"
#include "tests.h"

extern int key_remaining();

RPLThread::RPLThread(QObject *parent) : QThread(parent)
// ----------------------------------------------------------------------------
//   Constructor
// ----------------------------------------------------------------------------
{}


RPLThread::~RPLThread()
// ----------------------------------------------------------------------------
//   Destructor
// ----------------------------------------------------------------------------
{
    if (isFinished())
        return;
    key_push(tests::EXIT_PGM);
    wait();
}


void RPLThread::run()
// ----------------------------------------------------------------------------
//   Thread entry point
// ----------------------------------------------------------------------------
{
    program_main();
}
