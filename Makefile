#******************************************************************************
# Makefile.miq                                                    DB48X project
#******************************************************************************
#
#  File Description:
#
#     Build for all variants of DB48x, using make-it-quick
#
#
#
## Targets for DB48x:
##    make                : Build everything
##    make all            : Same
##    make dm42           : Build for SwissMicros DM42
##    make dm42n          : Build for SwissMicros DM42n (called db50x)
##    make dm32           : Build for SwissMicros DM32 (called db50x)
##    make android        : Build Android App Bundle for Google Play
##    make wasm           : Build WebAssembly (Emscripten: wasm/$(NAME).js)
##    make tools          : Build the tools (decimize, crc32, etc)
##    make help           : List all targets
##
## Optimization levels:
##    make opt            : Build optimized
##    make release        : Build for release
##    make debug          : Build for debugging
##
## Target prefix can be used to combine configurations:
##    make color-dm32-sim : Build color simulator for dm32 (db50x)
##    make sim-debug      : Build debug simulator
#
#******************************************************************************
#  (C) 2026 Christophe de Dinechin <christophe@dinechin.org>
#  This software is licensed under the terms outlined in LICENSE.txt
#******************************************************************************
#  This file is part of DB48X.
#
#  DB48X is free software: you can redistribute it and/or modify
#  it under the terms outlined in the LICENSE.txt file
#
#  DB48X is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#******************************************************************************

#------------------------------------------------------------------------------
#  Default targets
#------------------------------------------------------------------------------

# all: dm32 dm42 dm42-sim color-dm32-sim dm42-android color-dm32-android
all:		fw sims
sims:		dm42-sim color-dm32-sim
fw:		dm32 dm42
androids:	android color-dm32-android

# ------------------------------------------------------------------------------
# Package
# ------------------------------------------------------------------------------

PACKAGE_NAME ?= $(NAME)
PACKAGE_VERSION := $(shell git describe 2>/dev/null || echo "unknown")
PACKAGE_DESCRIPTION = A modern implementation of RPL in the spirit of the HP48
PACKAGE_URL = http://48calc.org
PACKAGE_BUGS = christophe@dinechin.org

# ------------------------------------------------------------------------------
# Build configuration
# ------------------------------------------------------------------------------

# Product name: db48x or db50x
NAME ?= db48x

# Build target: release, opt or debug
TARGET ?= release

# Build kind: fw, sim, android or wasm
KIND ?=

# Calculator model: dm32, dm42 or dm42n
MODEL ?= dm42

# Platform: dmcp
PLATFORM ?= dmcp

# SDK: dmcp or dmcp5
SDK ?= dmcp

# Program extension: pgm (DM42) or pg5 (DM32/DM42)
PGM ?= $(PGM_$(SDK))
PGM_dmcp = pgm
PGM_dmcp5 = pg5

# The variant as shown in teh top-level makefile
VARIANT=$(COLOR:%=%-)$(MODEL)$(KIND:%=-%)

# By default, auto-select build environment
BUILDENV ?= auto

# Mount base on a per-OS basis
HOST_OS_NAME:=$(shell uname -s)
MOUNTBASE ?= $(MOUNTBASE_$(HOST_OS_NAME))
MOUNTBASE_Darwin = /Volumes
MOUNTBASE_Linux = /run/media/$(USER)
DISK_NAME_dm32  ?= DM32
DISK_NAME_dm42  ?= DM42
DISK_NAME_dm42n ?= DM42N

# Host tools (built via recursive make; each tool has its own Makefile)
TTF2FONT = tools/ttf2font/ttf2font
DECIMIZE = tools/decimize/decimize
CRCFIX = tools/forcecrc32/forcecrc32
CRC32 = tools/crc32/crc32
BASE_FONT = fonts/FogSans-ddd.ttf

# Enable CHUCK feature (may lead to multiple iterations in FW build)
# CHUCK=yes

# ------------------------------------------------------------------------------
# Sources and products
# ------------------------------------------------------------------------------

CXX_STD = gnu++17

ifdef KIND
PRODUCTS ?= $(NAME)$(EXT_$(KIND))
EXT_fw = .exe
EXT_sim = .lib
EXT_android = .exe
EXT_wasm = .exe

CONFIG=$(CONFIG_$(KIND))
CONFIG_sim=							\
	sigaction						\
	<regex.h>						\
	<sys/mman.h>						\
	<signal.h>						\
	drand48							\
	libregex						\
	setlinebuf						\
	sigstksz						\
	strsignal
CONFIG_android=$(CONFIG_sim)
CONFIG_wasm=

SOURCES =							\
	src/$(PLATFORM)/target.cc				\
	src/$(PLATFORM)/sysmenu.cc				\
	src/$(PLATFORM)/main.cc					\
	fonts/EditorFont.cc					\
	fonts/HelpFont.cc					\
	fonts/ReducedFont.cc					\
	fonts/StackFont.cc					\
	src/algebraic.cc					\
	src/arithmetic.cc					\
	src/array.cc						\
	src/bignum.cc						\
	src/catalog.cc						\
	src/characters.cc					\
	src/command.cc						\
	src/comment.cc						\
	src/compare.cc						\
	src/complex.cc						\
	src/conditionals.cc					\
	src/constants.cc					\
	src/continued-fraction.cc				\
	src/custom.cc						\
	src/datetime.cc						\
	src/decimal.cc						\
	src/equations.cc					\
	src/expression.cc					\
	src/factor.cc						\
	src/file.cc						\
	src/files.cc						\
	src/finance.cc						\
	src/font.cc						\
	src/fraction.cc						\
	src/functions.cc					\
	src/graphics.cc						\
	src/grob.cc						\
	src/hwfp.cc						\
	src/integer.cc						\
	src/integrate.cc					\
	src/library.cc						\
	src/list.cc						\
	src/locals.cc						\
	src/logical.cc						\
	src/loops.cc						\
	src/menu.cc						\
	src/object.cc						\
	src/plot.cc						\
	src/polynomial.cc					\
	src/program.cc						\
	src/range.cc						\
	src/renderer.cc						\
	src/runtime.cc						\
	src/settings.cc						\
	src/solve.cc						\
	src/stack.cc						\
	src/stats.cc						\
	src/symbol.cc						\
	src/tag.cc						\
	src/text.cc						\
	src/unit.cc						\
	src/user_interface.cc					\
	src/util.cc						\
	src/variables.cc					\
	$(SOURCES_$(KIND))

SOURCES_fw =							\
	src/$(PLATFORM)/qspi_check.c				\
	$(SDK)/$(PLATFORM)/sys/pgm_syscalls.c			\
	$(SDK)/$(PLATFORM)/startup_pgm.s

SOURCES_sim =							\
	sim/dmcp.cpp						\
	sim/sim-screen.cpp					\
	sim/sim-window.cpp					\
	src/tests.cc						\
	recorder/recorder.c					\
	recorder/recorder_ring.c

SOURCES_wasm =							\
	sim/dmcp.cpp						\
	sim/sim-screen.cpp					\
	sim/sim-window.cpp					\
	src/tests.cc						\
	recorder/recorder.c					\
	recorder/recorder_ring.c

endif

# ------------------------------------------------------------------------------
# Includes and defines
# ------------------------------------------------------------------------------

INCLUDES ?= 	$(KIND)			\
		src/$(KIND)		\
		src/$(MODEL)		\
		src/$(PLATFORM) 	\
		src 			\
		$(SDK) 			\
		$(SDK)/$(PLATFORM)	\
		sim			\
		.

DEFINES =				\
	$(DEFINES_$(MODEL))		\
	$(DEFINES_$(TARGET))		\
	$(DEFINES_$(KIND))		\
	$(DEFINES_$(PLATFORM))		\
	DB48X_VERSION=\"$(VERSION)\"	\
	$(CHUCK:%=HAS_CHUCK)

DEFINES_debug = DEBUG
DEFINES_release = NDEBUG OPTIMIZED
DEFINES_opt = NDEBUG
DEFINES_small = NDEBUG
DEFINES_fast = NDEBUG
DEFINES_faster = NDEBUG
DEFINES_profile = NDEBUG

DEFINES_dm42 =	DM42					\
		MEMORY=100
DEFINES_dm32 = 	DM32					\
		LGAMMA_CRASHES				\
		CONFIG_FIXED_BASED_OBJECTS		\
		DEOPTIMIZE_CATALOG			\
		MEMORY=500
DEFINES_dm42n = DM42N					\
		LGAMMA_CRASHES				\
		CONFIG_FIXED_BASED_OBJECTS		\
		DEOPTIMIZE_CATALOG			\
		MEMORY=500
DEFINES_sim =	SIMULATOR				\
		USE_QT					\
		__packed=""				\
		HELPFILE_NAME=\"help/$(NAME).md\"	\
		HELPINDEX_NAME=\"help/$(NAME).idx\"
DEFINES_fw =	FIRMWARE				\
		__weak="__attribute__((weak))" 		\
		__packed="__attribute__((__packed__))"	\
		HELPFILE_NAME=\"/help/$(NAME).md\"	\
		HELPINDEX_NAME=\"/help/$(NAME).idx\"
DEFINES_android=ANDROID					\
		USE_QT					\
		HELPFILE_NAME=\"help/$(NAME).md\"	\
		HELPINDEX_NAME=\"help/$(NAME).idx\"
DEFINES_wasm =	SIMULATOR				\
		WASM					\
		__packed=""				\
		HELPFILE_NAME=\"help/$(NAME).md\"	\
		HELPINDEX_NAME=\"help/$(NAME).idx\"


# ------------------------------------------------------------------------------
# Include make-it-quick (first include; defines default targets)
# ------------------------------------------------------------------------------

TOP ?= ./
MIQ ?= $(TOP)recorder/make-it-quick/
$(MIQ)rules.mk:
	@git submodule update --init --recursive

# A little gmake trick to avoid mixing output from multiple rules at -j
GROUP_TARGETS ?= -Otarget
MAKEFLAGS += $(GROUP_TARGET) --no-print-directory

# We override installation
override DO_INSTALL=
override MIQ_TARBALL=

include $(MIQ)rules.mk

# We override installation
override DO_INSTALL=
override MIQ_TARBALL=


#------------------------------------------------------------------------------
#  make-it-quick settings overrides
#------------------------------------------------------------------------------

# Disable build time display
TIME=

# Suppress VLA warning for sim; must come after -Wall
CXXFLAGS_TARGET_$(TARGET) += $(CXXFLAGS_$(KIND))
CXXFLAGS_sim = -Wno-vla-cxx-extension
CXXFLAGS_wasm = -Wno-vla-cxx-extension
CXXFLAGS_fw = -Wa,-adhlns=$@.lst

# ------------------------------------------------------------------------------
# Default and variant targets
# ------------------------------------------------------------------------------

dm42:		dm42-fw-$(TARGET)
dm32:		dm32-fw-$(TARGET)
dm42n:		dm42n-fw-$(TARGET)
sim:		sim-$(TARGET)
android:	android-$(TARGET)
wasm:		wasm-$(TARGET)

fw-%:
	$(PRINT_COMMAND) $(MAKE) $* KIND=fw BUILDENV=arm-none-eabi
ifeq ($(filter $(KIND),sim wasm),)
sim-%:
	$(PRINT_COMMAND) $(MAKE) qt-$* KIND=sim BUILDENV=auto RECURSE=.config
endif
ifneq ($(KIND),android)
android-%:
	$(PRINT_COMMAND) $(MAKE) android-$* KIND=android RECURSE=.build
endif

# ------------------------------------------------------------------------------
# WebAssembly with Emscripten
# ------------------------------------------------------------------------------

# Installing the SDK
emsdk/emsdk:
	$(PRINT_COMMAND) git submodule update --init --recursive emsdk

emsdk: emsdk/emsdk
	@emcc --version >/dev/null 2>&1 || \
		( cd $(TOP)emsdk && ./emsdk install latest && ./emsdk activate latest )

.PHONY: emsdk

wasm-%: emsdk
	$(PRINT_COMMAND) ( EMSDK_QUIET=1 . $(TOP)emsdk/emsdk_env.sh && \
		$(MAKE) $* KIND=wasm BUILDENV=wasm MODEL=dm32 OUTPUT=wasm/ )

color-%:
	$(PRINT_COMMAND) $(MAKE) $* COLOR=color

dm42-%:
	$(PRINT_COMMAND) $(MAKE) $* MODEL=dm42 NAME=db48x SDK=dmcp
dm32-%:
	$(PRINT_COMMAND) $(MAKE) $* MODEL=dm32 NAME=db50x SDK=dmcp5
dm42n-%:
	$(PRINT_COMMAND) $(MAKE) $* MODEL=dm42n NAME=db50x SDK=dmcp5


# ------------------------------------------------------------------------------
# Building the tools
# ------------------------------------------------------------------------------

TOOLS_BUILDS=$(dir $(wildcard tools/*/Makefile))
TOOLS=$(foreach t,$(TOOLS_BUILDS),$t$(notdir $(t:%/=%)))
tools: $(TOOLS)
tools/%:
	$(PRINT_COMMAND) cd tools/$(*D) && $(MAKE) BUILDENV=auto TIME= DO_INSTALL= VARIANT=$(*D) OUTPUT=./

clangdb: clangdb-color-dm32-sim
clangdb-%: .ALWAYS
	@rm -rf .build && bear -- make v-$(TARGET) V=1 VERBOSE=1 $*


# ------------------------------------------------------------------------------
# Generated sources: fonts, decimals, version (after .recurse so tools exist)
# ------------------------------------------------------------------------------

MENUS_TREE   = doc/8-menus-tree-$(if $(filter dm42n,$(MODEL)),dm42,$(MODEL)).md
HELP_SOURCES = $(filter-out doc/8-menus-tree-%.md,$(wildcard doc/*.md doc/calc-help/*.md doc/commands/*.md)) $(MENUS_TREE)
PRODUCT_NAME = $(shell echo $(NAME) | tr "[:lower:]" "[:upper:]")
PRODUCT_MACHINE = $(if $(filter dm42n,$(MODEL)),DM42n,$(shell echo $(MODEL) | tr "[:lower:]" "[:upper:]"))
HELP_MACHINE = $(if $(filter dm42n,$(MODEL)),DM42,$(PRODUCT_MACHINE))
VERSION := $(shell git describe --dirty=Z --abbrev=4 2>/dev/null | sed -e 's/^v//g' -e 's/-g/-/g' | cut -c 1-16)
CHUCK_H = $(CHUCK:%=src/$(PLATFORM)/chuck-norris.h)
FONTS=Editor Help Reduced Stack

.prebuild:	$(FONTS:%=fonts/%Font.cc)			\
		src/decimal-pi.h src/decimal-e.h		\
		$(CHUCK_H)

fonts/EditorFont.cc: $(BASE_FONT) | $(TTF2FONT)
	$(PRINT_GENERATE) $(TTF2FONT) -s 48 -S 80 -y -10 EditorFont $(BASE_FONT) $@
fonts/StackFont.cc: $(BASE_FONT) | $(TTF2FONT)
	$(PRINT_GENERATE) $(TTF2FONT) -s 32 -S 80 -y -8 StackFont $(BASE_FONT) $@
fonts/ReducedFont.cc: $(BASE_FONT) | $(TTF2FONT)
	$(PRINT_GENERATE) $(TTF2FONT) -s 24 -S 80 -y -5 ReducedFont $(BASE_FONT) $@
fonts/HelpFont.cc: $(BASE_FONT) | $(TTF2FONT)
	$(PRINT_GENERATE) $(TTF2FONT) -s 18 -S 80 -y -3 HelpFont $(BASE_FONT) $@

src/decimal-pi.h: src/decimal-pi.txt | $(DECIMIZE)
	$(PRINT_GENERATE) $(DECIMIZE) < $< > $@ decimal_pi
src/decimal-e.h: src/decimal-e.txt | $(DECIMIZE)
	$(PRINT_GENERATE) $(DECIMIZE) < $< > $@ decimal_e

CHUCK_GIT_H=$(CHUCK:%=$(MIQ_OBJDIR)chuck-norris-$(VERSION).h)
$(CHUCK_H): $(CHUCK_GIT_H)
	@mkdir -p $(@D)
	$(PRINT_GENERATE) cp $< $@
$(CHUCK_GIT_H):
	@mkdir -p $(@D)
	$(PRINT_GENERATE) tools/generate-chuck.sh > $@

#------------------------------------------------------------------------------
#  Image comparison
#------------------------------------------------------------------------------

IMAGES=$(COLOR:%=color-)images
cmp-% compare-%:
	$(PRINT_COMMAND) compare $(IMAGES)/$*.png $(IMAGES)/bad/$*.png -compose src $*.png || true
	$(PRINT_COMMAND) open $*.png $(IMAGES)/bad/$*.png $(IMAGES)/$*.png
update-%:
	$(PRINT_COMMAND) mv $(IMAGES)/bad/$*.png $(IMAGES)/$*.png
	$(PRINT_COMMAND) rm -f $*.png
updates-%:
	$(MAKE) update-$* color-update-$*
cmps-% compares-%:
	$(MAKE) compare-$* color-compare-$*

BAD_IMAGES=$(wildcard $(IMAGES)/bad/*.png)
compare: $(BAD_IMAGES:$(IMAGES)/bad/%.png=cmp-%)
update: $(BAD_IMAGES:$(IMAGES)/bad/%.png=update-%)
updates: update color-update
compares: compare color-compare
.PHONY: compare update

# ------------------------------------------------------------------------------
# Help generation (lifted from Makefile)
# ------------------------------------------------------------------------------

.prebuild: help/$(NAME).md help/$(NAME).idx

GENERATE_HELP_MD=					\
	cat $^ |					\
	sed -e '/<!--- $(HELP_MACHINE) --->/,/<!--- !$(HELP_MACHINE) --->/s/$(HELP_MACHINE)/KEEP_IT/g' \
	    -e '/<!--- DM.* --->/,/<!--- !DM.* --->/d' 	\
	    -e '/<!--- KEEP_IT --->/d' 			\
	    -e '/<!--- !KEEP_IT --->/d' 		\
	    -e 's/KEEP_IT/$(PRODUCT_MACHINE)/g' 	\
	    -e 's/DB48X/$(PRODUCT_NAME)/g'	 	\
	    -e 's/db48x.md/$(NAME).md/g' 		\
	    -e 's/DM42/$(PRODUCT_MACHINE)/g' | tr -d '\r' > $@

help/$(NAME).md: $(HELP_SOURCES)
	$(PRINT_COMMAND) mkdir -p help
	$(PRINT_GENERATE) $(GENERATE_HELP_MD)
	$(PRINT_COMMAND) tr -d '\r' < $@ > $@.tmp && mv $@.tmp $@
	$(PRINT_COMMAND) cp doc/*.png help/ 2>/dev/null || true
	$(PRINT_COMMAND) mkdir -p help/img
	$(PRINT_COMMAND) rsync -a --delete doc/img/*.bmp help/img/ 2>/dev/null || true

doc/8-menus-tree-%.md: src/menu.cc src/ids.tbl tools/gen-menu-doc.py
	$(PRINT_GENERATE) python3 tools/gen-menu-doc.py --model $*

help/$(NAME).idx: help/$(NAME).md
	$(PRINT_GENERATE) grep -b '^#\|^\* `[^`]*`' $< | sed -e 's/:\(\* `[^`]*`\).*/:\1/g' | sort -k2 -t: > $@
	$(PRINT_COMMAND) [ "$$(cat $@ | wc -L)" -lt 80 ] || { echo "Some help header exceeds 80 bytes"; exit 2; }


# ------------------------------------------------------------------------------
#  Qt-hosted builds - Library built here, Qt-based app in sim/db48x.pro
# ------------------------------------------------------------------------------

QMAKE ?= $(shell which qmake6 2>/dev/null || which qmake)
QMAKE_opt = release
QMAKE_release = release
QMAKE_debug = debug
QMAKEFILE=sim/$(NAME)-$(KIND)-$(TARGET).mak

# Qt resource files
QRC_FILES=		sim/config.qrc		\
			sim/state.qrc		\
			sim/library.qrc		\
			sim/help-$(NAME).qrc	\
			sim/help/img.qrc

# Build Qt simulator directly with qmake
qt-$(TARGET): $(QMAKEFILE)
	$(PRINT_COMMAND) $(MAKE) -C $(<D) -f $(<F)
qt-%: $(QMAKEFILE)
	$(PRINT_COMMAND) $(MAKE) -C $(<D) -f $(<F) $*

$(QMAKEFILE): sim/$(NAME).pro $(QRC_FILES) $(MIQ_MAKEDEPS) $(CHUCK_H) .config
	$(PRINT_COMMAND) 					\
		DESTDIR="$(abspath $(or $(OUTPUT),.))";		\
		cd sim &&					\
		$(QMAKE_ENV)					\
		$(QMAKE) $(<F) -o $(@F) 			\
		$(QMAKE_SPECS:%=-spec %) 			\
		$(if $V,,CONFIG+=silent) 			\
		CONFIG+=$(QMAKE_$(TARGET)) 			\
		DEFINES+="DB48X_VERSION=\'\\\"$(VERSION)\\\"\'"	\
		DESTDIR="$$DESTDIR"				\
		OBJECTS_DIR=$(abspath $(MIQ_OBJDIR))		\
		RCC_DIR=$(abspath $(MIQ_OBJDIR))		\
		MOC_DIR=$(abspath $(MIQ_OBJDIR))		\
		UI_DIR=$(abspath $(MIQ_OBJDIR))

# Generation of Qt resource files
sim/help-$(NAME).qrc: help/$(NAME).md help/$(NAME).idx
sim/help-$(NAME).qrc: $(MIQ_MAKEDEPS)
	$(PRINT_GENERATE) (echo '<RCC>';				\
	 echo ' <qresource prefix="/help">';				\
	 for I in $(NAME).md $(NAME).idx; do				\
		J=$$(basename $$I);					\
		echo '  <file alias="'$$J'">../help/'$$J'</file>';	\
	 done;								\
	 echo ' </qresource>';						\
	 echo '</RCC>')							\
	> $@


sim/%.qrc: $(MIQ_MAKEDEPS)
	@mkdir -p $(@D)
	$(PRINT_GENERATE) (echo '<RCC>';			\
	 echo ' <qresource prefix="/'$*'">';			\
	 for I in $(wildcard $(QRC_EXT_$*:%=$*/%)); do		\
		J=$$(basename $$I);				\
		echo '  <file alias="'$$J'">../'$(QRC_DOT_$*)$*'/'$$J'</file>';	\
	 done;							\
	 echo ' </qresource>';					\
	 echo '</RCC>')						\
	> $@


QRC_EXT_config=*.csv *.cfg *.48k
QRC_EXT_help=$(NAME).md $(NAME).idx
QRC_EXT_help/img=*.bmp
QRC_DOT_help/img=../
QRC_EXT_library=*.48[sS]
QRC_EXT_state=*.48[sS]

keyboard:				\
	Keyboard-Layout.png 		\
	Keyboard-Cutout.png		\
	sim/keyboard-db48x.png 		\
	sim/keyboard-db48x-42like.png	\
	sim/keyboard-db48x-old.png	\
	help/keyboard.png		\
	doc/keyboard.png
Keyboard-Layout.png: DB48X-Keys/DB48X-Keys.001.png
	$(PRINT_COPY) cp $< $@
Keyboard-Cutout.png: DB48X-Keys/DB48X-Keys.002.png
	$(PRINT_COPY) cp $< $@
sim/keyboard-db48x.png: DB48X-Keys/DB48X-Keys.001.png
	$(PRINT_GENERATE) magick $< -crop 698x878+151+138 $@
sim/keyboard-db48x-42like.png: DB48X-Keys/DB48X-Keys.003.png
	$(PRINT_GENERATE) magick $< -crop 698x878+151+138 $@
sim/keyboard-db48x-old.png: DB48X-Keys/DB48X-Keys.005.png
	$(PRINT_GENERATE) magick $< -crop 698x878+151+138 $@
%/keyboard.png: sim/keyboard-db48x.png
	$(PRINT_COPY) cp $< $@

#------------------------------------------------------------------------------
# Android App Bundle for Google Play
#------------------------------------------------------------------------------
# Signing: if $(ANDROID_KEYSTORE) exists and ANDROID_KEYSTORE_PASS is non-empty,
# androiddeployqt signs the AAB; otherwise an unsigned bundle is produced (and
# a warning is printed). Override ANDROID_DEPLOY_QT / ANDROID_QT_BASE as needed.

ifeq ($(KIND),android)
ANDROID_SDK_ROOT ?= /opt/homebrew/share/android-commandlinetools
ANDROID_NDK_ROOT ?= $(ANDROID_SDK_ROOT)/ndk/26.1.10909125
ANDROID_QT_BASE ?= /Volumes/Qt/6.8.1
ANDROID_QT ?= $(ANDROID_QT_BASE)/android_arm64_v8a
ANDROID_QT_BIN ?= $(ANDROID_QT)/bin
# Host kit: macOS uses .../macos/bin; Linux CI and typical offline installs use gcc_64
ANDROID_QT_HOST_SUBDIR_Darwin = macos
ANDROID_QT_HOST_SUBDIR_Linux = gcc_64
ANDROID_QT_HOST_SUBDIR ?= $(or $(ANDROID_QT_HOST_SUBDIR_$(HOST_OS_NAME)),gcc_64)
ANDROID_DEPLOY_QT ?= $(ANDROID_QT_BASE)/$(ANDROID_QT_HOST_SUBDIR)/bin/androiddeployqt
ANDROID_KEYSTORE ?= $(HOME)/.local/android_release.keystore
ANDROID_JAVA_HOME ?= $(or $(JAVA_HOME),					    \
			$(shell /usr/libexec/java_home -v 17 2>/dev/null || \
				/usr/libexec/java_home -v 21 2>/dev/null || \
				true))
ANDROID_CAN_SIGN := $(and $(wildcard $(ANDROID_KEYSTORE)),$(strip $(ANDROID_KEYSTORE_PASS)))
ANDROID_DEPLOY_SIGN_FLAGS = $(if $(ANDROID_CAN_SIGN),\
	--sign $(ANDROID_KEYSTORE) $(NAME) --storepass '$(ANDROID_KEYSTORE_PASS)',)
QMAKE = $(ANDROID_QT_BIN)/qmake
QMAKE_SPECS = android-clang
QMAKE_ENV = 	export ANDROID_SDK_ROOT=$(ANDROID_SDK_ROOT) ;	\
		export ANDROID_NDK_ROOT=$(ANDROID_NDK_ROOT) ;	\
		export KEYSTORE_PATH=$(ANDROID_KEYSTORE) ;	\
		export JAVA_HOME=$(ANDROID_JAVA_HOME);
# make-it-quick defaults OUTPUT to the workspace root; keep Android bundles
# under android/ unless the caller overrides ANDROID_OUTPUT_DIR explicitly.
ANDROID_OUTPUT_DIR ?= android
AAB_FILE=$(ANDROID_OUTPUT_DIR:%=%/)$(NAME).aab

android-$(TARGET): $(AAB_FILE)
android-%: qt-%

# Additional dependencies for Android build
$(QMAKEFILE): sim/android/AndroidManifest.xml sim/android/build.gradle

# Deploy (and optionally sign) the AAB via androiddeployqt. androiddeployqt
# expects a build directory as --output and the .so staged under
# <output>/libs/arm64-v8a/, so we must run make install INSTALL_ROOT=<output>
# first. Normalize the final bundle to $(AAB_FILE) so workflows and helper
# scripts can upload a stable path.
$(AAB_FILE): $(QMAKEFILE) qt-$(TARGET)
	$(PRINT_COMMAND) 						\
		AAB="$(abspath $@)";					\
		OUTDIR="$(abspath $(dir $@))";				\
		mkdir -p "$$OUTDIR" &&					\
		cd sim && 						\
		$(MAKE) -f $(<F)  install INSTALL_ROOT="$$OUTDIR" &&	\
		$(QMAKE_ENV)						\
		$(ANDROID_DEPLOY_QT)					\
		  --input android-$(NAME)-deployment-settings.json	\
		  --output "$$OUTDIR" --gradle --aab			\
		  $(ANDROID_DEPLOY_SIGN_FLAGS) &&			\
		if [ ! -f "$$AAB" ]; then				\
			BUILT_AAB="$$(find "$$OUTDIR" -type f -name '*.aab' | sort | tail -1)"; \
			[ -n "$$BUILT_AAB" ] && cp "$$BUILT_AAB" "$$AAB"; \
		fi &&						\
		test -f "$$AAB"
	$(if $(ANDROID_CAN_SIGN),,$(PRINT_COMMAND) $(INFO) "[WARNING]" "Android AAB is UNSIGNED (need $(ANDROID_KEYSTORE) and ANDROID_KEYSTORE_PASS). Not for Play Store.")

endif


# ------------------------------------------------------------------------------
#   DMCP Custom link and post-build (override MIQ product rule)
# ------------------------------------------------------------------------------

ifeq ($(KIND),fw)
ELF_FILE = $(MIQ_OUTEXE)
BUILD_ID = $(shell $(TOP)tools/build_id 2>/dev/null || echo 0)
LDSCRIPT = src/$(MODEL)/stm32_program.ld
LDFLAGS += -T$(LDSCRIPT) -Wl,-Map=$(MIQ_OBJDIR)$(NAME).map,--cref

src/dmcp/qspi_check.c: .buildid
DEFINES_src/dmcp/qspi_check.c = BUILD_ID=$$($(TOP)tools/build_id)
.goodbye: .show-buildid
.buildid:
	$(PRINT_COMMAND) $(INFO) "[BUILD]" "$(shell $(TOP)tools/build_id)"
	@$(TOP)tools/build_id -u >/dev/null 2>&1 || true
	@touch src/dmcp/qspi_check.c
.show-buildid: .product
	$(PRINT_COMMAND) $(INFO) "[BUILD ID]" "$(shell $(TOP)tools/build_id)"

FLASH_BIN = $(MIQ_OBJDIR)$(NAME)_flash.bin
FLASH_HEX = $(FLASH_BIN:.bin=.hex)
QSPI_BIN  = $(OUTPUT)$(NAME)_qspi.bin
QSPI_HEX  = $(MIQ_OBJDIR)$(NAME)_qspi.hex
PGM_FILE  = $(OUTPUT)$(NAME).$(PGM)
QSPI_CRC  = src/$(MODEL)/qspi_crc.h

.postbuild: $(PGM_FILE) $(QSPI_BIN) $(QSPI_HEX) $(FLASH_BIN) $(FLASH_HEX)

$(PGM_FILE): $(FLASH_BIN) | $(LOGS).mkdir
	$(PRINT_GENERATE)$(INFO_NONL_CMD) "[SHA]" ; $(TOP)tools/add_pgm_chsum $< $@ | tee $(MIQ_BUILDLOG).sha1
	$(PRINT_COMMAND) $(INFO_NONL_CMD) "[BYTES]"; echo $$(cat $@ | wc -c)
	$(PRINT_COMMAND) $(INFO) "[SIZE]" "$(shell $(SIZE) $(ELF_FILE) | tail -1 | sed -e 's/^ //g')"

$(FLASH_BIN): $(ELF_FILE)
	$(PRINT_GENERATE) $(OBJCOPY) --remove-section .qspi -O binary $< $@

$(QSPI_BIN): $(ELF_FILE) | $(LOGS).mkdir $(CRCFIX) $(CRC32)
	$(PRINT_GENERATE) $(OBJCOPY) --only-section .qspi -O binary   $< $@
	$(PRINT_COMMAND) $(INFO) "[PATCH CRC]" "$(QSPI_BIN)"; $(TOP)tools/adjust_crc $(CRCFIX) $(QSPI_BIN) > $(MIQ_BUILDLOG).crc
	$(PRINT_COMMAND) $(INFO) "[CHECK CRC]" "$(QSPI_CRC)"; $(TOP)tools/check_qspi_crc "$(NAME)" "$@" "$(QSPI_CRC)" || ( echo "QSPI CRC changed, rebuilding" && $(MAKE) .postbuild )

$(FLASH_HEX): $(ELF_FILE)
	$(PRINT_GENERATE) $(OBJCOPY) --remove-section .qspi -O ihex   $< $@

$(QSPI_HEX): $(ELF_FILE)
	$(PRINT_GENERATE) $(OBJCOPY) --only-section .qspi -O ihex     $< $@


#------------------------------------------------------------------------------
#  Firmware install on physical calculators
#------------------------------------------------------------------------------

MOUNTPOINT ?= $(MOUNTBASE)/$(DISK_NAME)
DISK_NAME ?= $(DISK_NAME_$(MODEL))

SYNC=sync; sync; sync
EJECT = $(EJECT_$(HOST_OS_NAME))
EJECT_Darwin = hdiutil eject $(MOUNTPOINT)
EJECT_Linux = umount $(MOUNTPOINT)

TAR_OPTS = $(TAR_OPTS_$(HOST_OS_NAME))
TAR_OPTS_Darwin = --no-mac-metadata --no-fflags --no-xattrs --no-acls
TAR_FILES = $(PGM_FILE)					\
	    $(QSPI_BIN)					\
	    $(TAR_EXTRA_FILES)
TAR_EXTRA_FILES=	   				\
	    keymap.bin					\
	    help/$(NAME).md help/$(NAME).idx		\
	    help/*.bmp help/*/*.bmp			\
	    state/*.48[sSbB]				\
	    config/*.csv config/*.48k config/*.cfg	\
	    library/*.48[sSbB] library/*/*.48[sSbB]

PRINT_INSTALL=$(PRINT_COMMAND) $(INFO) "[INSTALL]" "$(NAME) => $(MOUNTPOINT)/" $(COLOR_FILTER);
PRINT_PACKAGE=$(PRINT_COMMAND) $(INFO) "[PACKAGE]" "$(NAME)-$(VERSION)" $(COLOR_FILTER);

fwinstall:
	$(PRINT_COMMAND) $(MAKE) MAKEFLAGS=-Onone RECURSE=.build do-fwinstall $(COLOR_FILTER)
do-fwinstall: $(TAR_FILES)
	$(PRINT_INSTALL) $(TAR) cf - $(TAR_OPTS) $(TAR_FILES) | $(TAR) xvf - -C $(MOUNTPOINT)
	$(PRINT_COMMAND) $(INFO) "[SYNC]" "$(MOUNTPOINT)" ; $(SYNC)  $(COLOR_FILTER)
	$(PRINT_COMMAND) $(INFO) "[EJECT]" "$(MOUNTPOINT)"; $(EJECT) $(COLOR_FILTER)
	$(PRINT_COMMAND) $(INFO) "[INSTALLED]" "$(VERSION)"          $(COLOR_FILTER)

fwdist:
	$(PRINT_COMMAND) $(MAKE) MAKEFLAGS=-Onone RECURSE=.build do-fwdist $(COLOR_FILTER)
do-fwdist: $(TAR_FILES)
	$(PRINT_PACKAGE) tar cvfz $(NAME)-v$(VERSION).tgz $(TAR_OPTS) $(TAR_FILES)
	$(PRINT_COMMAND) $(INFO) "[PACKAGED]" "$(NAME)-$(VERSION)" $(COLOR_FILTER)

else
install: dm32-fw-fwinstall dm42-fw-fwinstall
dist: dm32-fw-fwdist dm42-fw-fwdist
clean: sim-clean
endif
