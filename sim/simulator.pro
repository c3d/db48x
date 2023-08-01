#******************************************************************************
# simulator.pri                                                   DB48X project
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

QT += core gui quick widgets quickcontrols2 quickwidgets
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
	../src/dmcp/target.cc                   \
        ../src/dmcp/sysmenu.cc                  \
        ../src/dmcp/main.cc                     \
	../src/util.cc                          \
	../src/renderer.cc                      \
	../src/user_interface.cc                \
	../src/file.cc                          \
	../src/stack.cc                         \
	../src/settings.cc                      \
        ../src/object.cc                        \
        ../src/command.cc                       \
        ../src/compare.cc                       \
        ../src/logical.cc                       \
        ../src/integer.cc                       \
        ../src/bignum.cc                        \
        ../src/fraction.cc                      \
        ../src/complex.cc                       \
        ../src/decimal128.cc                    \
        ../src/decimal-64.cc                    \
        ../src/decimal-32.cc                    \
        ../src/runtime.cc                       \
        ../src/text.cc                          \
        ../src/symbol.cc                        \
        ../src/algebraic.cc                     \
        ../src/arithmetic.cc                    \
        ../src/functions.cc                     \
        ../src/variables.cc                     \
        ../src/locals.cc                        \
        ../src/catalog.cc                       \
        ../src/menu.cc                          \
        ../src/list.cc                          \
        ../src/program.cc                       \
        ../src/equation.cc                      \
        ../src/array.cc                         \
        ../src/loops.cc                         \
	../fonts/EditorFont.cc	                \
	../fonts/HelpFont.cc	                \
	../fonts/StackFont.cc			\
	../src/font.cc				\
	../src/tests.cc

HEADERS +=                                      \
	sim-window.h                            \
	sim-screen.h                            \
	sim-rpl.h


# User interface forms
FORMS    += sim-window.ui
RESOURCES += sim.qrc

# Indicate we are on simulator
DEFINES += SIMULATOR

# Pass debug flag
debug:DEFINES += DEBUG

# For DMCP headers
DEFINES += __packed=
macx:DEFINES += _WCHAR_T_DEFINED

# COnfigure Intel Decimal Floating Point Library
DEFINES += 	DECIMAL_CALL_BY_REFERENCE                       \
		DECIMAL_GLOBAL_ROUNDING                         \
		DECIMAL_GLOBAL_ROUNDING_ACCESS_FUNCTIONS        \
		DECIMAL_GLOBAL_EXCEPTION_FLAGS                  \
		DECIMAL_GLOBAL_EXCEPTION_FLAGS_ACCESS_FUNCTIONS

# Additional external library HIDAPI linked statically into the code
INCLUDEPATH += ../src/dmcp ../src ../inc

LIBS += gcc111libbid.a

win32:   LIBS += -lsetupapi
android: LIBS +=
freebsd: LIBS += -lthr -liconv
macx:    LIBS += -framework CoreFoundation -framework IOKit
macx:    QMAKE_CFLAGS += -fsanitize=address
macx:    LIBS += -fsanitize=address
