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
#include "settings.h"
#include "sim-eval.h"
#include "sim-install.h"
#include "sim-rpl.h"
#include "sim-window.h"
#include "sysmenu.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <QApplication>
#include <QByteArray>
#include <QDirIterator>
#include <QFont>
#include <QFontDatabase>
#include <QStandardPaths>
#include <QWindow>

RECORDER(options, 32, "Information about command line options");
RECORDER_TWEAK_DEFINE(rpl_objects_detail, 0, "Set to 1 to see object addresses")

bool   run_tests   = false;
bool   install     = false;
bool   no_beep     = false;
uint   memory_size = MEMORY; // Memory size in kilobytes
QDir   testDirectory;

extern bool clean_slate;

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


static void copy(const QString &fromName, const QString &toName)
// ----------------------------------------------------------------------------
//   Copy resource files to the target destination
// ----------------------------------------------------------------------------
{
    QDir from(fromName);
    QDir to(toName);
    if (!to.exists())
        to.mkpath(toName);

    for (QDirIterator it(fromName, QDirIterator::Subdirectories);
         it.hasNext();
         it.next())
    {
        const auto fi = it.fileInfo();
        if (!fi.isHidden())
        {
            QString relPath = from.relativeFilePath(fi.absoluteFilePath());
            QString absPath = to.filePath(relPath);
            if(fi.isDir())
            {
                // Create directory in target folder
                to.mkpath(absPath);
            }
            else if (fi.isFile())
            {
                auto perms = fi.permissions()
                    | QFile::WriteUser | QFile::WriteOwner;
                QFile::remove(absPath);
                sim_install_copy_file(fi.absoluteFilePath(), absPath,
                                      relPath, perms);
            }
        }
    }
    sim_install_regenerate_help_indices(toName);
}


// Ensure linker keeps debug code
extern cstring debug();


static void sim_test_traces(cstring suffix)
// ----------------------------------------------------------------------------
//   Configure est_* recorder traces for -O / -T
// ----------------------------------------------------------------------------
{
    static bool first = true;
    if (first)
    {
        recorder_trace_set("est_.*=0");
        first = false;
    }
    if (!suffix || !*suffix)
        return;

    char tname[256];
    if (strcmp(suffix, "all") == 0)
        strcpy(tname, "est_.*");
    else
        snprintf(tname, sizeof(tname) - 1, "est_%s", suffix);
    recorder_trace_set(tname);
}


static void sim_usage(FILE *out, cstring prog)
// ----------------------------------------------------------------------------
//   Print simulator command-line help (from SIMULATOR.md)
// ----------------------------------------------------------------------------
{
    fprintf(out,
            "Usage: %s [options] [setting=value ...]\n"
            "\n"
            "Environment variables:\n"
            "  DB48X_TRACES   Recorder trace filter "
            "(default: \".*(error|warn(ing)?)s?\")\n"
            "  DB48X_INSTALL  Same as the -I option\n"
            "\n"
            "Options:\n"
            "  -d[N]      Key delay in ms for tests (default: 0)\n"
            "  -e<cmd>    Evaluate RPL commands from argument at startup\n"
            "  -f<cmd>    Evaluate RPL commands from file at startup\n"
            "  -h         Show this help\n"
            "  -i[N]      Max wait for image match in tests (default: 500)\n"
            "  -k<map>    Load saved keymap\n"
            "  -l         Prefix -E / -F stack levels with level numbers\n"
            "  -m[N]      Memory size in kilobytes\n"
            "  -n         Enable beeps (override -H, -T or -N)\n"
            "  -r[N]      Screen refresh wait in ms (default: 20)\n"
            "  -s<N>      Window scaling factor\n"
            "  -t<trace>  Enable recorder trace (repeatable, regex ok)\n"
            "  -w[N]      Default test command wait in ms (default: 1000)\n"
            "  -C<file>   Create a screen snapshot to the given file\n"
            "  -D<pattrn> Recorder traces pattern on test failure\n"
            "  -E<cmd>    Similar to -e, printing stack to console\n"
            "  -F<cmd>    Similar to -f, printing stack to console\n"
            "  -H         Headless: no window, no sound, exit when done\n"
            "  -I         Initialize user environment (may overwrite)\n"
            "  -K         Simulate typing keys during tests\n"
            "  -N         Disable beeps\n"
            "  -O[test]   Configure test traces without running tests\n"
            "  -T[test]   Run tests (all or one suite, e.g. -Tmatrices)\n"
            "  -Z         Zero initial state (Clean slate mode)\n"
            "\n"
            "  -E and -F write to stdout with -H, to stderr otherwise.\n"
            "\n"
            "Arguments:\n"
            "  setting=value   Set initial RPL settings or flags "
            "(e.g. AngleMode=1)\n"
            "                  Values yes/true or positive integers\n"
            "\n",
            prog);
}


static bool sim_parse_args(int argc, char *argv[])
// ----------------------------------------------------------------------------
//   Parse command-line options; return false if an option is invalid
// ----------------------------------------------------------------------------
{
    for (int a = 1; a < argc; a++)
    {
        cstring as = argv[a];
        record(options, "  %u: %+s", a, as);
        if (as[0] != '-')
            continue;

        if (!strcmp(as, "-h") || !strcmp(as, "--help"))
        {
            sim_usage(stdout, file::basename(argv[0]));
            exit(0);
        }

        if (!as[1])
        {
            fprintf(stderr, "Unknown option: %s\n", as);
            return false;
        }

        cstring arg = as[2] ? as + 2 : a + 1 < argc ? argv[a + 1] : nullptr;
        uint nargs = 0;
#define check_arg()     if (!arg) goto missing_arg; nargs = 1;
        switch (as[1])
        {
        case 'C':
            check_arg();
            rplcmds.queue_snapshot(arg);
            break;
        case 'D':
            check_arg();
            tests::dump_on_fail = arg;
            break;

        case 'd':
            check_arg();
            tests::key_delay_time = atoi(arg);
            break;

        case 'e':
        case 'f':
        case 'E':
        case 'F':
            check_arg();
            if (!rplcmds.queue(arg, isupper(as[1]), tolower(as[1]) == 'f'))
                return false;
            break;

        case 'H':
            rplcmds.headless = true;
            no_beep = true;
            break;

        case 'I':
            install = true;
            break;

        case 'i':
            check_arg();
            tests::image_wait_time = atoi(arg);
            break;

        case 'K':
            tests::simulate_typing = true;
            break;

        case 'k':
            check_arg();
            load_saved_keymap(arg);
            break;

        case 'l':
            rplcmds.print_levels = true;
            break;

        case 'm':
            check_arg();
            memory_size = atoi(arg);
            break;

        case 'n':
            no_beep = false;
            break;

        case 'N':
            no_beep = true;
            break;

        case 'O':
            check_arg();
            sim_test_traces(arg);
            break;

        case 'r':
            check_arg();
            tests::refresh_delay_time = atoi(arg);
            break;

        case 's':
            check_arg();
            MainWindow::userScaling = atof(arg);
            break;

        case 't':
            check_arg();
            recorder_trace_set(arg);
            break;

        case 'T':
            check_arg();
            run_tests = true;
            no_beep = true;
            sim_test_traces(arg);
            break;

        case 'w':
            check_arg();
            tests::default_wait_time = atoi(arg);
            break;

        case 'Z':
            clean_slate = true;
            break;

        default:
            fprintf(stderr, "Unknown option: %s\n", as);
            return false;
        }

        if (nargs && !arg)
        {
        missing_arg:
            fprintf(stderr, "Option %s requires an argument\n", as);
            return false;
        }
        a += nargs;
    }
    return true;
}


static void configure_fonts(QApplication &app)
// ----------------------------------------------------------------------------
//   Set a concrete application font (offscreen needs an explicit family)
// ----------------------------------------------------------------------------
{
    QString family;
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
    {
#if defined(Q_OS_MAC)
        static const char *const paths[] = {
            "/System/Library/Fonts/Helvetica.ttc",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
            nullptr
        };
#elif defined(Q_OS_WIN)
        static const char *const paths[] = {
            "C:/Windows/Fonts/arial.ttf",
            nullptr
        };
#else
        static const char *const paths[] = {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            nullptr
        };
#endif
        for (uint i = 0; paths[i]; i++)
        {
            int id = QFontDatabase::addApplicationFont(paths[i]);
            if (id < 0)
                continue;
            QStringList families = QFontDatabase::applicationFontFamilies(id);
            if (!families.isEmpty())
            {
                family = families.first();
                break;
            }
        }
        if (family.isEmpty())
        {
#if defined(Q_OS_MAC)
            family = "Helvetica";
#elif defined(Q_OS_WIN)
            family = "Segoe UI";
#else
            family = "DejaVu Sans";
#endif
        }
    }
    else
    {
        family = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();
    }

    if (!family.isEmpty())
    {
        QFont::insertSubstitution("Sans Serif", family);
        QFont::insertSubstitution("Sans", family);
        app.setFont(QFont(family));
    }
}


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

    if (cstring doc = getenv("DB48X_DOCPATH"))
        tests::testing_path = std::string(doc);
    else
        tests::testing_path = QDir::current().absolutePath().toUtf8();

    record(options,
           "Simulator invoked as %+s with %d arguments", argv[0], argc - 1);

    if (!sim_parse_args(argc, argv))
    {
        fprintf(stderr, "Try '%s -h' for usage.\n", file::basename(argv[0]));
        exit(1);
    }

    for (int a = 1; a < argc; a++)
    {
        cstring as = argv[a];
        if (as[0] == '-')
            continue;

        if (cstring pos = strchr(as, '='))
        {
            size_t len   = pos - as;
            ularge value = strtoull(pos + 1, nullptr, 0);

#define ID(Name)                                                \
            const object::id ID_##Name = object::ID_##Name;     \
            (void) ID_##Name;
#include "ids.tbl"

            if (isalpha(pos[1]))
            {
                if (!strcasecmp(pos+1, "yes") || !strcasecmp(pos+1, "true"))
                    value = true;

#define ID(id)
#define SETTING_ENUM(Name, Alias, Base)         \
                if (sizeof(#Name) - 1 == len && \
                    (!strcasecmp(#Name, pos+1) ||       \
                     !strcasecmp(#Alias, pos+1)))       \
                    value = int(ID_##Name);
#include "ids.tbl"
            }

#define ID(id)
#define SETTING(Name, Low, High, Init)                  \
            if (sizeof(#Name) - 1 == len &&             \
                strncasecmp(#Name, as, len) == 0)       \
                Settings.Name((typeof(Init))value);     \
            else
#define FLAG(Enable, Disable)                           \
            if (sizeof(#Enable) - 1 == len &&           \
                strncasecmp(#Enable, as, len) == 0)     \
                Settings.Enable((bool) value);          \
            else if (sizeof(#Disable) - 1 == len &&     \
                strncasecmp(#Disable, as, len) == 0)    \
                Settings.Disable((bool) value);         \
            else

#include "ids.tbl"
            fprintf(stderr,
                    "Warning: Setting '%.*s' does not exist\n", int(len), as);
        }
    }

    // Indicate the first two-byte opcode
    if (!rplcmds.headless)
    {
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
    }
    else
    {
        qputenv("QT_QPA_PLATFORM", "offscreen");
        qputenv("QT_LOGGING_RULES", "*.warning=false");
    }

#if QT_VERSION < 0x060000
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif // QT version 6

    QCoreApplication::setOrganizationName("DB48X");
    QCoreApplication::setOrganizationDomain("48calc.org");
    QCoreApplication::setApplicationName("DB48X");

    QString files =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (getenv("DB48X_INSTALL") || !QDir(files).exists())
        install = true;
    if (install)
        copy(":/", files);
    QDir::setCurrent(files);
    QDir::current().mkdir("screens");

    QApplication a(argc, argv);
    configure_fonts(a);
    MainWindow w(nullptr, rplcmds.headless);
    w.show();

    int rc = a.exec();
    return rc;
}
