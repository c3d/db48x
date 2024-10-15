#######################################
# Target
######################################
TARGET = db48x
PLATFORM = dmcp
VARIANT = dm42
SDK = dmcp/dmcp
PGM = pgm
PGM_TARGET = $(TARGET).$(PGM)

######################################
# Building variables
######################################
OPT=release
# Alternatives (on the command line)
# OPT=debug	-g
# OPT=small	-Os
# OPT=fast	-O2
# OPT=faster	-O3
# OPT=fastest	-O4 -Ofast
# Experimentally, O2 performs best on DM42
# (see https://github.com/c3d/db48x/issues/66)

# Warning: macOSX only
MOUNTPOINT=/Volumes/$(VARIANT)/
EJECT=sync; sync; sync; hdiutil eject $(MOUNTPOINT)
PRODUCT_NAME=$(shell echo $(TARGET) | tr "[:lower:]" "[:upper:]")
PRODUCT_MACHINE=$(shell echo $(VARIANT) | tr "[:lower:]" "[:upper:]")


#######################################
# Pathes
#######################################
# Build path
BUILD = build/$(VARIANT)/$(OPT)

# Path to aux build scripts (including trailing /)
# Leave empty for scripts in PATH
TOOLS = tools

# CRC adjustment
CRCFIX = $(TOOLS)/forcecrc32/forcecrc32

# Decimal mantissa encoding
DECIMIZE = $(TOOLS)/decimize/decimize

FLASH=$(BUILD)/$(TARGET)_flash.bin
QSPI =$(BUILD)/$(TARGET)_qspi.bin

VERSION=$(shell git describe --dirty=Z --abbrev=4 | sed -e 's/^v//g' -e 's/-g/-/g' | cut -c 1-16)
VERSION_H=src/$(PLATFORM)/version.h


#==============================================================================
#
#  Primary build rules
#
#==============================================================================

# default action: build all
all: $(PGM_TARGET) help/$(TARGET).md help/$(TARGET).idx
	@echo "# Built $(VERSION)"

dm32:	dm32-all
dm32-%:
	$(MAKE) PLATFORM=dmcp SDK=dmcp5/dmcp PGM=pg5 VARIANT=dm32 TARGET=db50x $*
color-%:
	$(MAKE) COLOR=color $*

sim: sim/$(TARGET).mak
	cd sim; $(MAKE) -f $(<F) TARGET=$(shell awk '/^TARGET/ { print $$3; }' sim/$(TARGET).mak)
sim/$(TARGET).mak: sim/$(TARGET).pro Makefile $(VERSION_H)
	cd sim; qmake $(<F) -o $(@F) CONFIG+=$(QMAKE_$(OPT)) $(COLOR:%=CONFIG+=color)

sim:	recorder/config.h	\
	help/$(TARGET).md	\
	help/$(TARGET).idx	\
	fonts/EditorFont.cc	\
	fonts/StackFont.cc	\
	fonts/ReducedFont.cc	\
	fonts/HelpFont.cc	\
	keyboard		\
	.ALWAYS

WASM_TARGET=wasm/$(TARGET).js
wasm: emsdk $(WASM_TARGET) $(WASM_HTML)

emsdk: emsdk/emsdk
	emcc --version > /dev/null || \
	(cd emsdk && ./emsdk install latest && ./emsdk activate latest)

emsdk/emsdk:
	git submodule update --init --recursive

clangdb: sim/$(TARGET).mak .ALWAYS
	cd sim && rm -f *.o && compiledb make -f $(TARGET).mak && mv compile_commands.json ..

IMAGES=$(COLOR:%=color-)images
cmp-% compare-%:
	compare $(IMAGES)/$*.png $(IMAGES)/bad/$*.png -compose src $*.png || true
	open $*.png $(IMAGES)/bad/$*.png $(IMAGES)/$*.png
	echo mv -f $(IMAGES)/bad/$*.png $(IMAGES)/$*.png
update-%:
	mv $(IMAGES)/bad/$*.png $(IMAGES)/$*.png

BAD_IMAGES=$(wildcard $(IMAGES)/bad/*.png)
compare: $(BAD_IMAGES:$(IMAGES)/bad/%.png=cmp-%)
update: $(BAD_IMAGES:$(IMAGES)/bad/%.png=update-%)

keyboard: Keyboard-Layout.png Keyboard-Cutout.png sim/keyboard-db48x.png help/keyboard.png doc/keyboard.png
Keyboard-Layout.png: DB48X-Keys/DB48X-Keys.001.png
	cp $< $@
Keyboard-Cutout.png: DB48X-Keys/DB48X-Keys.002.png
	cp $< $@
sim/keyboard-db48x.png: DB48X-Keys/DB48X-Keys.001.png
	magick $< -crop 698x878+151+138 $@
%/keyboard.png: sim/keyboard-db48x.png
	cp $< $@

# Magick conversion of a BMP with dithering:
# magick  $F -colors 2 +dither -type bilevel BMP3:${F/png/bmp}

QMAKE_debug=debug
QMAKE_release=release
QMAKE_small=release
QMAKE_fast=release
QMAKE_faster=release
QMAKE_fastest=release

TTF2FONT=$(TOOLS)/ttf2font/ttf2font
$(TTF2FONT): $(TTF2FONT).cpp $(TOOLS)/ttf2font/Makefile src/ids.tbl
	cd $(TOOLS)/ttf2font; $(MAKE) TARGET=opt

TAR_OPTS=$(TAR_OPTS_$(shell uname))
TAR_OPTS_Darwin=--no-mac-metadata --no-fflags --no-xattrs --no-acls
TAR_FILES=	$(TARGET).$(PGM)		\
		$(TARGET)_qspi.bin		\
		keymap.bin			\
		help/$(TARGET).md		\
		help/$(TARGET).idx		\
		help/*.bmp help/*/*.bmp		\
		state/*.48[sSbB]		\
		config/*.csv			\
		library/*.48[sSbB]

# installation steps
COPY=cp
install: all
	cp $(BUILD)/$(TARGET)_qspi.bin  .
	tar cf - $(TAR_OPTS) $(TAR_FILES) | tar xvf - -C $(MOUNTPOINT)
	$(EJECT)
	@echo "# Installed $(VERSION)"

dist: all
	cp $(BUILD)/$(TARGET)_qspi.bin  .
	tar cvfz $(TARGET)-v$(VERSION).tgz $(TAR_OPTS) $(TAR_FILES)
	@echo "# Distributing $(VERSION)"

$(VERSION_H): $(BUILD)/version-$(VERSION).h
	cp $< $@
$(BUILD)/version-$(VERSION).h: $(BUILD)/.exists Makefile
	echo "#define DB48X_VERSION \"$(VERSION)\"" > $@


#BASE_FONT=fonts/C43StandardFont.ttf
BASE_FONT=fonts/FogSans-ddd.ttf
fonts/EditorFont.cc: $(TTF2FONT) $(BASE_FONT)
	$(TTF2FONT) -s 48 -S 80 -y -10 EditorFont $(BASE_FONT) $@
fonts/StackFont.cc: $(TTF2FONT) $(BASE_FONT)
	$(TTF2FONT) -s 32 -S 80 -y -8 StackFont $(BASE_FONT) $@
fonts/ReducedFont.cc: $(TTF2FONT) $(BASE_FONT)
	$(TTF2FONT) -s 24 -S 80 -y -5 ReducedFont $(BASE_FONT) $@
fonts/HelpFont.cc: $(TTF2FONT) $(BASE_FONT)
	$(TTF2FONT) -s 18 -S 80 -y -3 HelpFont $(BASE_FONT) $@
help/$(TARGET).md: $(wildcard doc/*.md doc/calc-help/*.md doc/commands/*.md)
	mkdir -p help && \
	cat $^ | \
	sed -e '/<!--- $(PRODUCT_MACHINE) --->/,/<!--- !$(PRODUCT_MACHINE) --->/s/$(PRODUCT_MACHINE)/KEEP_IT/g' \
	    -e '/<!--- DM.* --->/,/<!--- !DM.* --->/d' \
	    -e '/<!--- KEEP_IT --->/d' \
	    -e '/<!--- !KEEP_IT --->/d' \
	    -e 's/KEEP_IT/$(PRODUCT_MACHINE)/g' \
	    -e 's/DB48X/$(PRODUCT_NAME)/g' \
	    -e 's/db48x.md/$(TARGET).md/g' \
            -e 's/DM42/$(PRODUCT_MACHINE)/g' > $@
	cp doc/*.png help/
	mkdir -p help/img
	rsync -av --delete doc/img/*.bmp help/img/
help/$(TARGET).idx: help/$(TARGET).md
	grep -b '^#' $< | sort -k2 -t: > $@
	[ "$$(cat $@ | wc -L)" -lt 80 ]

check-ids: help/$(TARGET).md
	@for I in $$(cpp -xc++ -D'ID(n)=n' src/ids.tbl | 		\
		   sed -e 's/##//g' | sed -e 's/^#.*//g');		\
	do								\
	  if  ! grep -q $$I src/ignored_menus.csv ; then		\
	    grep -q "ID_$$I" src/menu.cc || 				\
	      echo "$$I not in menus";					\
	  fi;								\
	  if  ! grep -q $$I src/ignored_help.csv ; then			\
	    grep -q "^#.*[[:<:]]$$I[[:>:]]" help/$(TARGET).md ||	\
	      echo "$$I not in help";					\
	  fi;								\
	done

debug-%:
	$(MAKE) $* OPT=debug
release-%:
	$(MAKE) $* OPT=release
small-%:
	$(MAKE) $* OPT=small
fast-%:
	$(MAKE) $* OPT=fast
faster-%:
	$(MAKE) $* OPT=faster
fastest-%:
	$(MAKE) $* OPT=fastest


#######################################
# Custom section
#######################################

SOURCES=$(C_SOURCES) $(CXX_SOURCES)

# Includes
C_INCLUDES += -Isrc/$(VARIANT) -Isrc/$(PLATFORM) -Isrc

# C sources
C_SOURCES +=

# C++ sources
CXX_SOURCES +=				\
	src/$(PLATFORM)/target.cc	\
	src/$(PLATFORM)/sysmenu.cc	\
	src/$(PLATFORM)/main.cc		\
	fonts/EditorFont.cc		\
	fonts/HelpFont.cc		\
	fonts/ReducedFont.cc		\
	fonts/StackFont.cc		\
	src/algebraic.cc		\
	src/arithmetic.cc		\
	src/array.cc			\
	src/bignum.cc			\
	src/catalog.cc			\
	src/characters.cc		\
	src/command.cc			\
	src/comment.cc		        \
	src/compare.cc			\
	src/complex.cc			\
	src/conditionals.cc		\
	src/constants.cc		\
	src/datetime.cc			\
	src/decimal.cc			\
	src/equations.cc		\
	src/expression.cc		\
	src/file.cc			\
	src/files.cc			\
	src/font.cc			\
	src/fraction.cc			\
	src/functions.cc		\
	src/graphics.cc			\
	src/grob.cc			\
	src/hwfp.cc			\
	src/integer.cc			\
	src/integrate.cc		\
	src/library.cc			\
	src/list.cc			\
	src/locals.cc			\
	src/logical.cc			\
	src/loops.cc			\
	src/menu.cc			\
	src/object.cc			\
	src/plot.cc			\
	src/polynomial.cc		\
	src/program.cc			\
	src/renderer.cc			\
	src/runtime.cc			\
	src/settings.cc			\
	src/solve.cc			\
	src/stack.cc			\
	src/stats.cc			\
	src/symbol.cc			\
	src/tag.cc			\
	src/text.cc		        \
	src/unit.cc			\
	src/user_interface.cc		\
	src/util.cc			\
	src/variables.cc		\
	$(PLATFORM_SOURCES)

# ASM sources
#ASM_SOURCES += src/xxx.s

# Additional defines
#C_DEFS += -DXXX

# Defines for the code
DEFINES +=					\
	$(DEFINES_$(OPT))			\
	$(DEFINES_$(VARIANT))			\
	HELPFILE_NAME=\"/help/$(TARGET).md\"	\
	HELPINDEX_NAME=\"/help/$(TARGET).idx\"
DEFINES_debug=DEBUG
DEFINES_release=NDEBUG
DEFINES_small=NDEBUG
DEFINES_fast=NDEBUG
DEFINES_faster=NDEBUG
DEFINES_fastes=NDEBUG
DEFINES_dm32 = 	DM32 				\
		CONFIG_FIXED_BASED_OBJECTS	\
		DEOPTIMIZE_CATALOG

DEFINES_dm42 = DM42
DEFINES_wasm = $(DEFINES_dm32) SIMULATOR WASM

C_DEFS += $(DEFINES:%=-D%)

# Recorder and dependencies
recorder/config.h: recorder/recorder.h recorder/Makefile
	cd recorder && $(MAKE) TARGET=opt
$(BUILD)/recorder.o $(BUILD)/recorder_ring.o: recorder/config.h

# ---


#######################################
# binaries
#######################################

#------------------------------------------------------------------------------
ifeq ($(VARIANT),wasm)
#------------------------------------------------------------------------------
CC = emcc
CXX = emcc -x c++ -std=gnu++17
PLATFORM_SOURCES=src/wasp/dmcp.cc src/wasm/sim-screen.cc src/wasm/sim-window.cc
C_SOURCES += recorder/recorder.c recorder/recorder_ring.c
CFLAGS += 	-O3 -pthread
LDFLAGS +=	-s MODULARIZE=0				\
		-s RESERVED_FUNCTION_POINTERS=20	\
		-s PTHREAD_POOL_SIZE=4			\
		--bind -pthread

#------------------------------------------------------------------------------
else
#------------------------------------------------------------------------------
CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++
AS = arm-none-eabi-gcc -x assembler-with-cpp
OBJCOPY = arm-none-eabi-objcopy
AR = arm-none-eabi-ar
SIZE = arm-none-eabi-size
HEX = $(OBJCOPY) -O ihex
BIN = $(OBJCOPY) -O binary -S
CPUFLAGS += -mthumb -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16
PLATFORM_FLAGS = -Wno-packed-bitfield-compat -Wall -fdata-sections -ffunction-sections
CXX_PLATFORM_FLAGS = -fno-rtti
C_LIST_FLAGS = -Wa,-a,-ad,-alms=$(BUILD)/$(notdir $(<:.c=.lst))
CXX_LIST_FLAGS = -Wa,-a,-ad,-alms=$(BUILD)/$(notdir $(<:.cc=.lst))
ASM_SOURCES = $(SDK)/startup_pgm.s
LINK_OPTS=					\
	-Wl,-Map=$(BUILD)/$(TARGET).map,--cref	\
	-Wl,--gc-sections			\
	-Wl,--wrap=_malloc_r

# System sources
C_INCLUDES += -I$(SDK)
C_SOURCES += $(SDK)/sys/pgm_syscalls.c

#------------------------------------------------------------------------------
endif
#------------------------------------------------------------------------------

#######################################
# CFLAGS
#######################################
# macros for gcc
AS_DEFS =
C_DEFS += -D__weak="__attribute__((weak))" -D__packed="__attribute__((__packed__))"
AS_INCLUDES =

# compile gcc flags
ASFLAGS = $(CPUFLAGS) $(PLATFORM_FLAGS) $(AS_DEFS) $(AS_INCLUDES) $(ASFLAGS_$(OPT))
CFLAGS += $(CPUFLAGS) $(PLATFORM_FLAGS)  $(C_DEFS) $(C_INCLUDES) $(CFLAGS_$(OPT))
CFLAGS += -Wno-misleading-indentation
DBGFLAGS = $(DBGFLAGS_$(OPT))
DBGFLAGS_debug = -g

CFLAGS_debug += -Os -DDEBUG
CFLAGS_release += $(CFLAGS_release_$(VARIANT))
CFLAGS_release_dm42 = -Os
CFLAGS_release_dm32 = -O2
CFLAGS_small += -Os
CFLAGS_fast += -O2
CFLAGS_faster += -O3
CFLAGS_fastest += -O4

CFLAGS  += $(DBGFLAGS)
LDFLAGS += $(DBGFLAGS)

# Generate dependency information
CFLAGS += -MD -MP -MF .dep/$(@F).d

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = src/$(VARIANT)/stm32_program.ld
LIBDIR =
LDFLAGS += $(CPUFLAGS) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) $(LINK_OPTS)


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# C++ sources
OBJECTS += $(addprefix $(BUILD)/,$(notdir $(CXX_SOURCES:.cc=.o)))
vpath %.cc $(sort $(dir $(CXX_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

CXXFLAGS = $(CFLAGS) -fno-exceptions $(CXX_PLATFORM_FLAGS)

$(BUILD)/%.o: %.c Makefile | $(BUILD)/.exists
	$(CC) -c $(CFLAGS) $(C_LIST_FLAGS) $< -o $@

$(BUILD)/%.o: %.cc Makefile | $(BUILD)/.exists
	$(CXX) -c $(CXXFLAGS) $(CXX_LIST_FLAGS) $< -o $@

$(BUILD)/%.o: %.cpp Makefile | $(BUILD)/.exists
	$(CXX) -c $(CXXFLAGS) $(CXX_LIST_FLAGS) $< -o $@

$(BUILD)/%.o: %.s Makefile | $(BUILD)/.exists
	$(AS) -c $(CFLAGS) $< -o $@

ifeq ($(VARIANT),wasm)

$(WASM_TARGET): $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

else

$(WASM_TARGET): $(SOURCES) Makefile
	(source emsdk/emsdk_env.sh && \
	 make VARIANT=wasm PGM=js PGM_TARGET=wasm/$(TARGET).js SDK=sim )

$(BUILD)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
$(TARGET).$(PGM): $(BUILD)/$(TARGET).elf Makefile $(CRCFIX)
	$(OBJCOPY) --remove-section .qspi -O binary  $<  $(FLASH)
	$(OBJCOPY) --remove-section .qspi -O ihex    $<  $(FLASH:.bin=.hex)
	$(OBJCOPY) --only-section   .qspi -O binary  $<  $(QSPI)
	$(OBJCOPY) --only-section   .qspi -O ihex    $<  $(QSPI:.bin=.hex)
	$(TOOLS)/adjust_crc $(CRCFIX) $(QSPI)
	$(TOOLS)/check_qspi_crc $(TARGET) $(BUILD)/$(TARGET)_qspi.bin src/$(VARIANT)/qspi_crc.h || ( rm -rf build/$(VARIANT) && exit 1)
	$(TOOLS)/add_pgm_chsum $(BUILD)/$(TARGET)_flash.bin $@
	$(SIZE) $<
	wc -c $@

endif

DECIMAL_CONSTANTS=pi e
DECIMAL_SOURCES=$(DECIMAL_CONSTANTS:%=src/decimal-%.h)
src/decimal-%.h: src/decimal-%.txt $(DECIMIZE)
	$(DECIMIZE) < $< > $@ decimal_$*

$(OBJECTS): $(DECIMAL_SOURCES) $(VERSION_H)

all sim: $(DECIMAL_SOURCES)

$(BUILD)/%.hex: $(BUILD)/%.elf | $(BUILD)
	$(HEX) $< $@

$(BUILD)/%.bin: $(BUILD)/%.elf | $(BUILD)
	$(BIN) $< $@

$(BUILD)/.exists:
	mkdir -p $(@D)
	touch $@


$(CRCFIX): $(CRCFIX).c $(dir $(CRCFIX))/Makefile
	cd $(dir $(CRCFIX)); $(MAKE)
$(DECIMIZE): $(DECIMIZE).cpp $(dir $(DECIMIZE))/Makefile
	cd $(dir $(DECIMIZE)); $(MAKE) TARGET=opt


#######################################
# clean up
#######################################
clean:
	-rm -fR .dep build sim/*.o sim/*/*.o


#######################################
# dependencies
#######################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

.PHONY: clean all
.ALWAYS:

# *** EOF ***
