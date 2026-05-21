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
#include "sim-rpl.h"
#include "sim-window.h"
#include "sysmenu.h"
#include "version.h"

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
bool   noisy_tests = false;
bool   no_beep     = false;
uint   memory_size = MEMORY; // Memory size in kilobytes
QDir   testDirectory;

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
                QFile::copy(fi.absoluteFilePath(), absPath);
                QFile::setPermissions(absPath, perms);
            }
        }
    }
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
            "  -D<pattern>  Recorder traces on test failure\n"
            "  -E<cmd>    Like -e, print stack after each command "
            "(repeatable)\n"
            "  -H         Headless: no window, exit when done "
            "(use with -E)\n"
            "  -I         Initialize user environment (may overwrite)\n"
            "  -K         Simulate typing keys during tests\n"
            "  -N         Disable beeps\n"
            "  -O[test]   Configure test traces without running tests\n"
            "  -T[test]   Run tests (all or one suite, e.g. -Tmatrices)\n"
            "  -d[N]      Key delay in ms for tests (default: 0)\n"
            "  -e<cmd>    Evaluate RPL command line at startup (repeatable)\n"
            "  -h         Show this help\n"
            "  -i[N]      Max wait for image match in tests (default: 500)\n"
            "  -k<map>    Load saved keymap\n"
            "  -l         With -E, prefix stack lines with level numbers\n"
            "  -m[N]      Memory size in kilobytes\n"
            "  -n         Enable beeps during tests\n"
            "  -r[N]      Screen refresh wait in ms (default: 20)\n"
            "  -s<N>      Window scaling factor\n"
            "  -t<trace>  Enable recorder trace (repeatable, regex ok)\n"
            "  -w[N]      Default test command wait in ms (default: 1000)\n"
            "\n"
            "  -E writes stack to stderr with the GUI, to stdout with -H.\n"
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
            sim_usage(stdout, argv[0]);
            exit(0);
        }

        if (!as[1])
        {
            fprintf(stderr, "Unknown option: %s\n", as);
            return false;
        }

        switch (as[1])
        {
        case 'D':
            if (as[2])
                tests::dump_on_fail = as + 2;
            else if (a + 1 < argc)
                tests::dump_on_fail = argv[++a];
            else
            {
                fprintf(stderr, "Option %s requires an argument\n", as);
                return false;
            }
            break;

        case 'd':
            if (as[2])
                tests::key_delay_time = atoi(as + 2);
            else if (a + 1 < argc)
                tests::key_delay_time = atoi(argv[++a]);
            else
            {
                fprintf(stderr, "Option %s requires an argument\n", as);
                return false;
            }
            break;

        case 'E':
            if (as[2])
                sim_eval_console_commands.emplace_back(as + 2);
            else if (a + 1 < argc)
                sim_eval_console_commands.emplace_back(argv[++a]);
            else
            {
                fprintf(stderr, "Option %s requires an argument\n", as);
                return false;
            }
            break;

        case 'e':
            if (as[2])
                sim_eval_commands.emplace_back(as + 2);
            else if (a + 1 < argc)
                sim_eval_commands.emplace_back(argv[++a]);
            else
            {
                fprintf(stderr, "Option %s requires an argument\n", as);
                return false;
            }
            break;

        case 'H':
            sim_eval_headless = true;
            break;

        case 'I':
            install = true;
            break;

        case 'i':
            if (as[2])
                tests::image_wait_time = atoi(as + 2);
            else if (a + 1 < argc)
                tests::image_wait_time = atoi(argv[++a]);
            else
            {
                fprintf(stderr, "Option %s requires an argument\n", as);
                return false;
            }
            break;

        case 'K':
            tests::simulate_typing = true;
            break;

        case 'k':
            if (as[2])
                load_saved_keymap(as + 2);
            else if (a + 1 < argc)
                load_saved_keymap(argv[++a]);
            else
            {
                fprintf(stderr, "Option %s requires an argument\n", as);
                return false;
            }
            break;

        case 'l':
            sim_eval_print_levels = true;
            break;

        case 'm':
            if (as[2])
                memory_size = atoi(as + 2);
            else if (a + 1 < argc)
                memory_size = atoi(argv[++a]);
            else
            {
                fprintf(stderr, "Option %s requires an argument\n", as);
                return false;
            }
            break;

        case 'n':
            noisy_tests = true;
            break;

        case 'N':
            no_beep = true;
            break;

        case 'O':
            if (as[2])
                sim_test_traces(as + 2);
            break;

        case 'r':
            if (as[2])
                tests::refresh_delay_time = atoi(as + 2);
            else if (a + 1 < argc)
                tests::refresh_delay_time = atoi(argv[++a]);
            else
            {
                fprintf(stderr, "Option %s requires an argument\n", as);
                return false;
            }
            break;

        case 's':
            if (as[2])
                MainWindow::userScaling = atof(as + 2);
            else if (a + 1 < argc)
                MainWindow::userScaling = atof(argv[++a]);
            else
            {
                fprintf(stderr, "Option %s requires an argument\n", as);
                return false;
            }
            break;

        case 't':
            recorder_trace_set(as + 2);
            break;

        case 'T':
            run_tests = true;
            if (as[2])
                sim_test_traces(as + 2);
            break;

        case 'w':
            if (as[2])
                tests::default_wait_time = atoi(as + 2);
            else if (a + 1 < argc)
                tests::default_wait_time = atoi(argv[++a]);
            else
            {
                fprintf(stderr, "Option %s requires an argument\n", as);
                return false;
            }
            break;

        default:
            fprintf(stderr, "Unknown option: %s\n", as);
            return false;
        }
    }
    return true;
}


static void sim_select_platform(bool headless)
// ----------------------------------------------------------------------------
//   Choose the Qt platform plugin for headless runs
// ----------------------------------------------------------------------------
//   The offscreen plugin has no real "Sans Serif" font on macOS and spends
//   ~100 ms building alias tables (qt.qpa.fonts warning). On macOS, keep the
//   native platform and simply do not show the window. Linux headless CI still
//   needs offscreen when there is no display server.
{
    if (!headless || !qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        return;
#if defined(Q_OS_LINUX)
    qputenv("QT_QPA_PLATFORM", "offscreen");
#endif
}


static void sim_configure_application_font(QApplication &app)
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
        fprintf(stderr, "Try '%s -h' for usage.\n", argv[0]);
        sim_usage(stderr, argv[0]);
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
    if (!sim_eval_headless)
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

    sim_select_platform(sim_eval_headless);

    QApplication a(argc, argv);
    sim_configure_application_font(a);

    MainWindow w(nullptr, sim_eval_headless);
    if (!sim_eval_headless)
        w.show();

    int rc = a.exec();
    return sim_eval_headless ? 0 : rc;
}
