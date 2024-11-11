// ****************************************************************************
//  main.cpp                                                      DB48X project
// ****************************************************************************
//
//   File Description:
//
//     DM42 simulator for the DB48 project
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

#include "main.h"
#include "object.h"
#include "recorder.h"
#include "sim-rpl.h"
#include "sim-window.h"
#include "sysmenu.h"
#include "version.h"

#include <QApplication>
#include <QWindow>

RECORDER(options, 32, "Information about command line options");
RECORDER_TWEAK_DEFINE(rpl_objects_detail, 0, "Set to 1 to see object addresses")

bool run_tests = false;
bool noisy_tests = false;
uint memory_size = 100;           // Memory size in kilobytes

size_t recorder_render_object(intptr_t tracing,
                              const char *UNUSED /* format */,
                              char *buffer, size_t size,
                              uintptr_t arg)
// ----------------------------------------------------------------------------
//   Render a value during a recorder dump (%t format)
// ----------------------------------------------------------------------------
{
    object_p value = object_p(arg);
    size_t result = 0;
    if (tracing)
    {
        if (value)
        {
            char tmp[80];
            size_t sz =  value->render(tmp, sizeof(tmp)-1);
            if (sz >= sizeof(tmp))
                sz = sizeof(tmp)-1;
            tmp[sz] = 0;
            if (RECORDER_TWEAK(rpl_objects_detail))
                result = snprintf(buffer, size, "%p[%lu] %s[%s]",
                                  (void *) value,
                                  value->size(),
                                  value->fancy(),
                                  tmp);
            else
                result = snprintf(buffer, size, "%s", tmp);

        }
        else
        {
            result = snprintf(buffer, size, "0x0 <NULL>");
        }
    }
    else
    {
        result = snprintf(buffer, size, "%p", (void *) value);
    }
    return result;
}


// Ensure linker keeps debug code
extern cstring debug();


int main(int argc, char *argv[])
// ----------------------------------------------------------------------------
//   Main entry point for the simulator
// ----------------------------------------------------------------------------
{
    const char *traces = getenv("DB48X_TRACES");
    recorder_trace_set(".*(error|warn(ing)?)s?");
    if (traces)
        recorder_trace_set(traces);
    recorder_dump_on_common_signals(0, 0);
    recorder_configure_type('t', recorder_render_object);

    // This is just to link otherwise unused code intended for use in debugger
    if (traces && traces[0] == char(0xFF))
        if (cstring result = debug())
            record(options, "Strange input %s", result);

    // Indicate the first two-byte opcode
    fprintf(stderr,
            "%s version %s\n"
            "Last single-byte opcode is %s\n"
            "First two byte opcode is %s\n"
            "Total of %u opcodes\n"
            "Help file name is %s\n",
            PROGRAM_NAME,
            DB48X_VERSION,
            object::name(object::id(127)),
            object::name(object::id(128)),
            uint(object::NUM_IDS),
            HELPFILE_NAME);

    record(options,
           "Simulator invoked as %+s with %d arguments", argv[0], argc-1);
    for (int a = 1; a < argc; a++)
    {
        record(options, "  %u: %+s", a, argv[a]);
        if (argv[a][0] == '-')
        {
            switch(argv[a][1])
            {
            case 't':
                recorder_trace_set(argv[a]+2);
                break;
            case 'n':
                noisy_tests = true;
                break;

            case 'T':
                run_tests = true;
                // fall-through
            case 'O':
                if (argv[a][2])
                {
                    static bool first = true;
                    if (first)
                    {
                        recorder_trace_set("est_.*=0");
                        first = false;
                    }
                    char tname[256];
                    if (strcmp(argv[a]+2, "all") == 0)
                        strcpy(tname, "est_.*");
                    else
                        snprintf(tname, sizeof(tname)-1, "est_%s", argv[a]+2);
                    recorder_trace_set(tname);
                }
                break;
            case 'D':
                if (argv[a][2])
                    tests::dump_on_fail = argv[a]+2;
                else if (a < argc)
                    tests::dump_on_fail = argv[++a];
                break;

            case 'k':
                if (argv[a][2])
                    keymap_filename = argv[a] + 2;
                else if (a < argc)
                    keymap_filename = argv[++a];
                break;
            case 'w':
                if (argv[a][2])
                    tests::default_wait_time = atoi(argv[a]+2);
                else if (a < argc)
                    tests::default_wait_time = atoi(argv[++a]);
                break;
            case 'd':
                if (argv[a][2])
                    tests::key_delay_time = atoi(argv[a]+2);
                else if (a < argc)
                    tests::key_delay_time = atoi(argv[++a]);
                break;
            case 'r':
                if (argv[a][2])
                    tests::refresh_delay_time = atoi(argv[a]+2);
                else if (a < argc)
                    tests::refresh_delay_time = atoi(argv[++a]);
                break;
            case 'i':
                if (argv[a][2])
                    tests::image_wait_time = atoi(argv[a]+2);
                else if (a < argc)
                    tests::image_wait_time = atoi(argv[++a]);
                break;
            case 'm':
                if (argv[a][2])
                    memory_size = atoi(argv[a]+2);
                else if (a < argc)
                    memory_size = atoi(argv[++a]);
                break;
            case 's':
                if (argv[a][2])
                    MainWindow::devicePixelRatio = atof(argv[a]+2);
                else if (a < argc)
                    MainWindow::devicePixelRatio = atof(argv[++a]);
                break;

            }
        }
    }

#if QT_VERSION < 0x060000
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif // QT version 6

    QCoreApplication::setOrganizationName("DB48X Project");
    QCoreApplication::setOrganizationDomain("48calc.org");
    QCoreApplication::setApplicationName("DB48X");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
