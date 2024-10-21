#******************************************************************************
#  db48x.pro                                                      DB48X project
#******************************************************************************
#
#  File Description:
#
#    Configurations specific to a simulator build
#
#
#
#
#
#
#
#
#******************************************************************************
#  (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
#  (C) 2022 Claudio Lapilli and the newRPL team
#  This software is licensed under the terms described in LICENSE.txt
#******************************************************************************

QT += core gui quick widgets quickcontrols2 quickwidgets multimedia
TEMPLATE = app

CONFIG += debug

# Qt support code
SOURCES +=                                      \
	../recorder/recorder.c                  \
	../recorder/recorder_ring.c             \
        sim-main.cpp                            \
        sim-window.cpp                          \
	sim-screen.cpp                          \
	sim-rpl.cpp                             \
	dmcp.cpp                                \
        ../fonts/EditorFont.cc                  \
        ../fonts/HelpFont.cc                    \
        ../fonts/ReducedFont.cc                 \
        ../fonts/StackFont.cc                   \
        ../src/algebraic.cc                     \
        ../src/arithmetic.cc                    \
        ../src/array.cc                         \
        ../src/bignum.cc                        \
        ../src/catalog.cc                       \
        ../src/characters.cc                    \
        ../src/command.cc                       \
        ../src/comment.cc                       \
        ../src/compare.cc                       \
        ../src/complex.cc                       \
        ../src/conditionals.cc                  \
        ../src/constants.cc                     \
        ../src/custom.cc                        \
        ../src/datetime.cc                      \
        ../src/decimal.cc                       \
        ../src/dmcp/main.cc                     \
        ../src/dmcp/sysmenu.cc                  \
        ../src/dmcp/target.cc                   \
        ../src/equations.cc                     \
        ../src/expression.cc                    \
        ../src/file.cc                          \
        ../src/files.cc                         \
        ../src/font.cc                          \
        ../src/fraction.cc                      \
        ../src/functions.cc                     \
        ../src/graphics.cc                      \
        ../src/grob.cc                          \
        ../src/hwfp.cc                          \
        ../src/integer.cc                       \
        ../src/integrate.cc                     \
        ../src/library.cc                       \
        ../src/list.cc                          \
        ../src/locals.cc                        \
        ../src/logical.cc                       \
        ../src/loops.cc                         \
        ../src/menu.cc                          \
        ../src/object.cc                        \
        ../src/plot.cc                          \
        ../src/polynomial.cc                    \
        ../src/program.cc                       \
        ../src/renderer.cc                      \
        ../src/runtime.cc                       \
        ../src/settings.cc                      \
        ../src/solve.cc                         \
        ../src/stack.cc                         \
        ../src/stats.cc                         \
        ../src/symbol.cc                        \
        ../src/tag.cc                           \
        ../src/tests.cc                         \
        ../src/text.cc                          \
        ../src/unit.cc                          \
        ../src/user_interface.cc                \
        ../src/util.cc                          \
        ../src/variables.cc

HEADERS +=                                      \
	sim-window.h                            \
	sim-screen.h                            \
	sim-rpl.h


# User interface forms
FORMS    += sim-window.ui
RESOURCES += sim.qrc

# Indicate we are on simulator
DEFINES += SIMULATOR CONFIG_FIXED_BASED_OBJECTS

# Pass debug flag
debug:DEFINES += DEBUG

# For DMCP headers
DEFINES += __packed= USE_QT
macx:DEFINES += _WCHAR_T_DEFINED

# Configure help file
DEFINES += 	HELPFILE_NAME=\\\"help/db48x.md\\\"
DEFINES += 	HELPINDEX_NAME=\\\"help/db48x.idx\\\"

color:DEFINES += CONFIG_COLOR

# Additional external library HIDAPI linked statically into the code
INCLUDEPATH += ../src/dm42 ../src/dmcp ../src

win32:   LIBS += -lsetupapi
android: LIBS +=
freebsd: LIBS += -lthr -liconv
macx:    LIBS += -framework CoreFoundation -framework IOKit
macx:    QMAKE_CFLAGS += -fsanitize=address
macx:    LIBS += -fsanitize=address
clang:   QMAKE_CFLAGS   += -Wno-unknown-pragmas
clang:   QMAKE_CXXFLAGS += -Wno-unknown-pragmas

OBJECTS_DIR=db48x-build

ICON = db48x.icns
