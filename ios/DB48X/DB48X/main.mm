// ****************************************************************************
//  main.mm                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Main iOS entry point for DB48X
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
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

#include "object.h"
#include "recorder.h"
#include "version.h"
#include "tests.h"


#import <UIKit/UIKit.h>
#import "AppDelegate.h"


RECORDER(options, 32, "Information about command line options");

bool run_tests = false;
bool noisy_tests = false;
bool db48x_keyboard = true;
#ifndef MEMORY
#define MEMORY 70
#endif /* MEMORY */
uint memory_size = MEMORY;           // Memory size in kilobytes



size_t recorder_render_object(intptr_t tracing,
                              const char *UNUSED format,
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
            tmp[sz] = 0;
            result = snprintf(buffer, size, "%p[%lu] %s[%s]",
                              (void *) value,
                              value->size(),
                              value->fancy(),
                              tmp);
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


#if DEBUG
// Ensure linker keeps debug code
extern cstring debug();
#endif // DEBUG


int main(int argc, char * argv[])
// ----------------------------------------------------------------------------
//   Boilerplate application code
// ----------------------------------------------------------------------------
{
    const char *traces = getenv("DB48X_TRACES");
    recorder_trace_set(".*(error|warning)s?");
    if (traces)
        recorder_trace_set(traces);
    recorder_dump_on_common_signals(0, 0);
    recorder_configure_type('t', recorder_render_object);

#ifdef DEBUG
    // This is just to link otherwise unused code intended for use in debugger
    if (traces && traces[0] == char(0xFF))
        if (cstring result = debug())
            record(options, "Strange input %s", result);
#endif // DEBUG\
    
    // Indicate the first two-byte opcode
    fprintf(stderr,
            "DB48X version %s\n"
            "Last single-byte opcode is %s\n"
            "First two byte opcode is %s\n",
            DB48X_VERSION,
            object::name(object::id(127)),
            object::name(object::id(128)));

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
            case 'k':
                db48x_keyboard = true;
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
            }
        }
    }

    @autoreleasepool
    {
        return UIApplicationMain(argc, argv, nil,
                                 NSStringFromClass([AppDelegate class]));
    }
}



// ============================================================================
//
//   State file interface
//
// ============================================================================
//   iOS requires this to be wrapped in an NSFileCoordinator if we want
//   to be able to access files in iCloud

int ui_wrap_io(file_sel_fn_t callback, const char *path, void *data, bool writing)
// ----------------------------------------------------------------------------
//   Wrap I/O operations in an NSFileCoordinator
// ----------------------------------------------------------------------------
{
    int __block result = 0;
    NSFileCoordinator *coordinator = [[NSFileCoordinator alloc] init];
    auto accessor = ^(NSURL *newURL)
    {
        NSURL *filePathURL = [newURL filePathURL];
        NSString *fileFullPath = filePathURL.path;
        NSArray<NSString *> *documentsPath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsDirectory = [[documentsPath lastObject] stringByAppendingString: @"/"];
        NSString *filePath = [fileFullPath stringByReplacingOccurrencesOfString:documentsDirectory withString:@"" options:NSAnchoredSearch range:NSMakeRange(0, [fileFullPath length]) ];
        cstring   path = [filePath cStringUsingEncoding:NSUTF8StringEncoding];
        cstring   name = path;
        for (cstring p = path; *p; p++)
            if (*p == '/' || *p == '\\')
                name = p + 1;
        result = callback(path, name, data);
    };

    NSString *nspath = [NSString stringWithCString:path encoding:NSUTF8StringEncoding];
    NSURL *url = [NSURL fileURLWithPath:nspath];
    if (writing)
    {
        [coordinator coordinateWritingItemAtURL:url
                                        options:0
                                          error:nil
                                     byAccessor:accessor];
    }
    else
    {
        [coordinator coordinateReadingItemAtURL:url
                                        options:0
                                          error:nil
                                     byAccessor:accessor];
    }

    return result;
}
