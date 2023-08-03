#######################################
# target
######################################
TARGET = DB48X
PLATFORM = dmcp
VARIANT = DM42
SDK = dmcp
PGM = pgm

######################################
# building variables
######################################
OPT=release
# Alternatives (on the command line)
# OPT=debug	-g
# OPT=small	-Os
# OPT=fast	-O2
# OPT=faster	-O3
# OPT=fastest	-O4 -Ofast
# Experimentally, O2 performs best on DM42
# (see https://github.com/c3d/DB48X-on-DM42/issues/66)

# Warning: macOSX only
MOUNTPOINT=/Volumes/$(VARIANT)/
EJECT=hdiutil eject $(MOUNTPOINT)


#######################################
# pathes
#######################################
# Build path
BUILD = build/$(VARIANT)/$(OPT)

# Path to aux build scripts (including trailing /)
# Leave empty for scripts in PATH
TOOLS = tools

# CRC adjustment
CRCFIX = $(TOOLS)/forcecrc32/forcecrc32

FLASH=$(BUILD)/$(TARGET)_flash.bin
QSPI =$(BUILD)/$(TARGET)_qspi.bin

VERSION=$(shell git describe --dirty=Z --abbrev=5| sed -e 's/^v//g' -e 's/-g/-/g')
VERSION_H=src/$(PLATFORM)/version.h


#==============================================================================
#
#  Primary build rules
#
#==============================================================================

# default action: build all
all: $(TARGET).$(PGM) help/$(TARGET).md

dm32:	dm32-all
dm32-%:
	$(MAKE) PLATFORM=dmcp SDK=dmcp5/dmcp PGM=pg5 VARIANT=DM32 $*

# installation steps
install: install-pgm install-qspi install-help
	$(EJECT)
install-fast: install-pgm
	$(EJECT)
install-pgm: all
	cp -v $(TARGET).$(PGM) $(MOUNTPOINT)
install-qspi: all
	cp -v $(QSPI) $(MOUNTPOINT)
install-help: help/$(TARGET).md
	cp -v help/$(TARGET).md $(MOUNTPOINT)help/

sim: sim/simulator.mak
	cd sim; make -f $(<F)
sim/simulator.mak: sim/simulator.pro Makefile $(VERSION_H)
	cd sim; qmake $(<F) -o $(@F) CONFIG+=$(QMAKE_$(OPT))

sim:	sim/gcc111libbid.a	\
	recorder/config.h	\
	help/$(TARGET).md	\
	fonts/EditorFont.cc	\
	fonts/StackFont.cc	\
	fonts/HelpFont.cc	\
	.ALWAYS

clangdb: sim/simulator.mak .ALWAYS
	cd sim && rm -f *.o && compiledb make -f simulator.mak && mv compile_commands.json ..

QMAKE_debug=debug
QMAKE_release=release
QMAKE_small=release
QMAKE_fast=release
QMAKE_faster=release
QMAKE_fastest=release

TTF2FONT=$(TOOLS)/ttf2font/ttf2font
$(TTF2FONT): $(TTF2FONT).cpp $(TOOLS)/ttf2font/Makefile src/ids.tbl
	cd $(TOOLS)/ttf2font; $(MAKE)
sim/gcc111libbid.a: sim/gcc111libbid-$(shell uname)-$(shell uname -m).a
	cp $< $@

dist: all
	mv build/release/$(TARGET)_qspi.bin  .
	tar cvfz v$(VERSION).tgz $(TARGET).$(PGM) $(TARGET)_qspi.bin \
		help/*.md STATE/*.48S

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
fonts/HelpFont.cc: $(TTF2FONT) $(BASE_FONT)
	$(TTF2FONT) -s 18 -S 80 -y -3 HelpFont $(BASE_FONT) $@
help/$(TARGET).md: $(wildcard doc/*.md doc/calc-help/*.md doc/commands/*.md)
	mkdir -p help && cat $^ > $@

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


######################################
# System sources
######################################
C_INCLUDES += -I$(SDK)
C_SOURCES += $(SDK)/sys/pgm_syscalls.c
ASM_SOURCES = $(SDK)/startup_pgm.s

#######################################
# Custom section
#######################################

# Includes
C_INCLUDES += -Isrc/$(PLATFORM) -Isrc -Iinc

# C sources
C_SOURCES +=

# Floating point sizes
DECIMAL_SIZES=32 64
DECIMAL_SOURCES=$(DECIMAL_SIZES:%=src/decimal-%.cc)

# C++ sources
CXX_SOURCES +=				\
	src/$(PLATFORM)/target.cc	\
	src/$(PLATFORM)/sysmenu.cc	\
	src/$(PLATFORM)/main.cc		\
	src/user_interface.cc		\
	src/file.cc			\
	src/stack.cc			\
	src/util.cc			\
	src/renderer.cc			\
	src/settings.cc			\
	src/runtime.cc			\
	src/object.cc			\
	src/command.cc			\
	src/compare.cc			\
	src/logical.cc			\
	src/integer.cc			\
	src/bignum.cc			\
	src/fraction.cc			\
	src/complex.cc			\
	src/decimal128.cc		\
	$(DECIMAL_SOURCES)		\
	src/text.cc		        \
	src/symbol.cc			\
	src/algebraic.cc		\
	src/arithmetic.cc		\
	src/functions.cc		\
	src/variables.cc		\
	src/locals.cc			\
	src/catalog.cc			\
	src/menu.cc			\
	src/list.cc			\
	src/program.cc			\
	src/equation.cc			\
	src/array.cc			\
	src/loops.cc			\
	src/conditionals.cc		\
	src/font.cc			\
	src/graphics.cc			\
	fonts/HelpFont.cc		\
	fonts/EditorFont.cc		\
	fonts/StackFont.cc



# Generate the sized variants of decimal128
src/decimal-%.cc: src/decimal128.cc src/decimal-%.h
	sed -e s/decimal128.h/decimal-$*.h/g -e s/128/$*/g $< > $@
src/decimal-%.h: src/decimal128.h
	sed -e s/128/$*/g -e s/leb$*/leb128/g $< > $@

# ASM sources
#ASM_SOURCES += src/xxx.s

# Additional defines
#C_DEFS += -DXXX

# Intel library related defines
DEFINES += \
	DECIMAL_CALL_BY_REFERENCE \
	DECIMAL_GLOBAL_ROUNDING \
	DECIMAL_GLOBAL_ROUNDING_ACCESS_FUNCTIONS \
	DECIMAL_GLOBAL_EXCEPTION_FLAGS \
	DECIMAL_GLOBAL_EXCEPTION_FLAGS_ACCESS_FUNCTIONS \
	$(DEFINES_$(OPT))
DEFINES_debug=DEBUG
DEFINES_release=RELEASE
DEFINES_small=RELEASE
DEFINES_fast=RELEASE
DEFINES_faster=RELEASE
DEFINES_fastes=RELEASE

C_DEFS += $(DEFINES:%=-D%)

# Libraries
LIBS += lib/gcc111libbid_hard.a

# Recorder and dependencies
recorder/config.h: recorder/recorder.h recorder/Makefile
	cd recorder && $(MAKE)
$(BUILD)/recorder.o $(BUILD)/recorder_ring.o: recorder/config.h

# ---


#######################################
# binaries
#######################################
CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++
AS = arm-none-eabi-gcc -x assembler-with-cpp
OBJCOPY = arm-none-eabi-objcopy
AR = arm-none-eabi-ar
SIZE = arm-none-eabi-size
HEX = $(OBJCOPY) -O ihex
BIN = $(OBJCOPY) -O binary -S

#######################################
# CFLAGS
#######################################
# macros for gcc
AS_DEFS =
C_DEFS += -D__weak="__attribute__((weak))" -D__packed="__attribute__((__packed__))"
AS_INCLUDES =


CPUFLAGS += -mthumb -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16

# compile gcc flags
ASFLAGS = $(CPUFLAGS) $(AS_DEFS) $(AS_INCLUDES) $(ASFLAGS_$(OPT)) -Wall -fdata-sections -ffunction-sections
CFLAGS  = $(CPUFLAGS) $(C_DEFS) $(C_INCLUDES) $(CFLAGS_$(OPT)) -Wall -fdata-sections -ffunction-sections
CFLAGS += -Wno-misleading-indentation
DBGFLAGS = $(DBGFLAGS_$(OPT))
DBGFLAGS_debug = -g

CFLAGS_debug += -O0 -DDEBUG
CFLAGS_release += -O2
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
LDFLAGS = $(CPUFLAGS) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) \
	-Wl,-Map=$(BUILD)/$(TARGET).map,--cref \
	-Wl,--gc-sections \
	-Wl,--wrap=_malloc_r




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

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -Wno-packed-bitfield-compat

$(BUILD)/%.o: %.c Makefile | $(BUILD)/.exists
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD)/%.o: %.cc Makefile | $(BUILD)/.exists
	$(CXX) -c $(CXXFLAGS) -Wa,-a,-ad,-alms=$(BUILD)/$(notdir $(<:.cc=.lst)) $< -o $@

$(BUILD)/%.o: %.s Makefile | $(BUILD)/.exists
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
$(TARGET).$(PGM): $(BUILD)/$(TARGET).elf Makefile $(CRCFIX)
	$(OBJCOPY) --remove-section .qspi -O binary  $<  $(FLASH)
	$(OBJCOPY) --remove-section .qspi -O ihex    $<  $(FLASH:.bin=.hex)
	$(OBJCOPY) --only-section   .qspi -O binary  $<  $(QSPI)
	$(OBJCOPY) --only-section   .qspi -O ihex    $<  $(QSPI:.bin=.hex)
	$(TOOLS)/adjust_crc $(CRCFIX) $(QSPI)
	$(TOOLS)/check_qspi_crc $(TARGET) $(BUILD)/$(TARGET)_qspi.bin src/qspi_crc.h || ( $(MAKE) clean && exit 1)
	$(TOOLS)/add_pgm_chsum $(BUILD)/$(TARGET)_flash.bin $@
	$(SIZE) $<
	wc -c $@

$(OBJECTS): $(DECIMAL_SOURCES) $(VERSION_H)
sim: $(DECIMAL_SOURCES)

$(BUILD)/%.hex: $(BUILD)/%.elf | $(BUILD)
	$(HEX) $< $@

$(BUILD)/%.bin: $(BUILD)/%.elf | $(BUILD)
	$(BIN) $< $@

$(BUILD)/.exists:
	mkdir -p $(@D)
	touch $@


$(CRCFIX): $(CRCFIX).c $(dir $(CRCFIX))/Makefile
	cd $(dir $(CRCFIX)); $(MAKE)


#######################################
# clean up
#######################################
clean:
	-rm -fR .dep build sim/*.o


#######################################
# dependencies
#######################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

.PHONY: clean all
.ALWAYS:

# *** EOF ***
