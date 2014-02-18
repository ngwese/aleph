#
# Copyright (c) 2009-2010 Atmel Corporation. All rights reserved.
#
# edited by ezra buchla for monome-aleph project, 2012
#

# Path to top level ASF directory relative to this project directory.
PRJ_PATH = ../../../avr32/asf-3.1.3/

# Target CPU architecture: ap, ucr1, ucr2 or ucr3
ARCH = ucr2

# Target part: none, ap7xxx or uc3xxxxx
PART = uc3a0512

# Target device flash memory details (used by the avr32program programming
# tool: [cfi|internal]@address
FLASH = internal@0x80000000

# Clock source to use when programming; xtal, extclk or int
PROG_CLOCK = int

# Applicat	ion target name. Given with suffix .a for library and .elf for a
# standalone application.
TARGET = ui_test.elf

# List of C source files.
CSRCS = \
        avr32/drivers/flashc/flashc.c                      \
        avr32/drivers/gpio/gpio.c                          \
        avr32/drivers/intc/intc.c                          \
        avr32/drivers/tc/tc.c                          \
        avr32/drivers/usart/usart.c			   \
        avr32/services/delay/delay.c			   \
        common/services/clock/uc3a0_a1/sysclk.c            \
	./src/main.c \
	./src/init.c \
	./src/screen.c \
	./src/font.c \
	./src/timers.c \
	./src/events.c \
	./src/encoders.c
#        avr32/drivers/pm/pm.c                              \
        avr32/drivers/pm/pm_conf_clocks.c                  \
#       avr32/drivers/pm/power_clocks_lib.c                \
#	./src/encoder_test.c 
#    avr32/boards/evk1100/init.c                        \

# List of assembler source files.
ASSRCS = \
       avr32/drivers/intc/exception.S                     \
       avr32/utils/startup/startup_uc3.S                  \
       avr32/utils/startup/trampoline_uc3.S

# List of include paths.
INC_PATH = \
       avr32/boards                                       \
       avr32/boards/evk1100                               \
       avr32/drivers/flashc                               \
       avr32/drivers/gpio                                 \
       avr32/drivers/intc                                 \
       avr32/drivers/tc                                 \
       avr32/drivers/usart                                \
       avr32/drivers/cpu/cycle_counter/                                \
       avr32/utils                                        \
       avr32/utils/preprocessor                           \
       avr32/services/delay                               \
       common/boards                                      \
       common/services/clock                              \
       common/utils \
       ./ \
	../../tests/ui/avr32
#       avr32/drivers/pm                                   \

# Additional search paths for libraries.
LIB_PATH = 

# List of libraries to use during linking.
LIBS = 

# Path relative to top level directory pointing to a linker script.
LINKER_SCRIPT = avr32/utils/linker_scripts/at32uc3a/0512/gcc/link_uc3a0512.lds

# Additional options for debugging. By default the common Makefile.in will
# add -g3.
DBGFLAGS = 

# Application optimization used during compilation and linking:
# -O0, -O1, -O2, -O3 or -Os
OPTIMIZATION = -O2

# Extra flags to use when archiving.
ARFLAGS = 

# Extra flags to use when assembling.
ASFLAGS = 

# Extra flags to use when compiling.
CFLAGS = 

# Extra flags to use when preprocessing.
#
# Preprocessor symbol definitions
#   To add a definition use the format "-D name[=definition]".
#   To cancel a definition use the format "-U name".
#
# The most relevant symbols to define for the preprocessor are:
#   BOARD      Target board in use, see boards/board.h for a list.
#   EXT_BOARD  Optional extension board in use, see boards/board.h for a list.
CPPFLAGS = \
       -D BOARD=EVK1100

# Extra flags to use when linking
LDFLAGS = \
       -nostartfiles -Wl,-e,_trampoline