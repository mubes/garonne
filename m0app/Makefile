#                                       ++++++++++++++++++
#                                  +++++++++++++++++++++++++++++
#                              +++++++                      +++++++++
#                          +++++++                               +++++++++++++
#         ++++++++++++++++++++                                         ++++++++++
#    +++++++++++++++++++++                                                     +++
#   +++++                                                                       +++
#  +++         ######### ######### ########  #########  #########   +++++++      ++
#  +++  +++++ ####  #### ######## ####  #### ##### #### #### ####  +++  ++++    +++
#  +++   ++++ ###     ## ###      ###    ### ###    ### ###    ### ++++++++   +++
#   ++++ ++++ ########## ###      ########## ###    ### ###    ### ++++    +++++
#    +++++++   ###### ## ###       ########  ###     ## ##     ###  ++++++++++
#

#VERBOSE=1
DEBUG=1
CROSS_COMPILE = $(HOME)/bin/armgcc/bin/arm-none-eabi-

##########################################################################
# User configuration and firmware specific object files	
##########################################################################

# Overall system defines for compilation
ifdef DEBUG
GCC_DEFINE=-DDEBUG 
OPT_LEVEL =
else
GCC_DEFINE=
OPT_LEVEL = -O2
endif


GCC_DEFINE+= -std=gnu99 -DCORE_M0
DOXCONFIG   = config/doxygen.cfg
EXPORT = llb_v2_m0app.tgz

# The target, flash and ram of the microprocessor.
TARGET = LPC4370
CORTEX_TYPE=m0

CFILES =
SFILES =
OLOC = ofiles
INCLUDE_PATHS = -I..
DEBUG_OPTS = -g3 -gdwarf-2 -ggdb

##########################################################################
# Generic multi-project files 
##########################################################################

# LPCOpen Files
# =============
LPCOpen_DIR=thirdparty/lpcopen/lpc_chip_43xx_m0
INCLUDE_PATHS += $(patsubst %,-I%,$(shell find $(LPCOpen_DIR) -name "*.h" -exec dirname {} \; | uniq ))
CFILES += $(shell find $(LPCOpen_DIR) -name "*.c" -print)

##########################################################################
# Project-specific files 
##########################################################################

# Main Files
# ==========
App_DIR=App
INCLUDE_PATHS += -IInc
INCLUDE_PATHS += $(patsubst %,-I%,$(shell find $(App_DIR) -name "*.h" -exec dirname {} \; | uniq ))
INCLUDE_PATHS += $(patsubst %,-I%,$(shell find $(OLOC) -name "*.h" -exec dirname {} \; | uniq ))
CFILES += $(shell find $(App_DIR) -name "*.c" -print)

##########################################################################
# Startup code
##########################################################################


##########################################################################
# GNU GCC compiler prefix and location
##########################################################################

ASTYLE = astyle
AS = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
GDB = $(CROSS_COMPILE)gdb
SIZE = $(CROSS_COMPILE)size
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
LPCRC = Tools/lpcrc/lpcrc
MAKE = make
OUTFILE = llb_v2_m0app

##########################################################################
# Startup files
#########################################################################

LD_SCRIPT = config/lpc4370_m0app_ram.ld
CPU_TYPE = cortex-$(CORTEX_TYPE)

##########################################################################
# Quietening
##########################################################################

ifdef VERBOSE
cmd = $1
Q :=
else
cmd = @$(if $(value 2),echo "$2";)$1
Q := @
endif

ifdef DEBUG
HOST=--specs=rdimon.specs -lgcc -lc -lrdimon 
else
HOST=-lc -lc -lnosys
endif

##########################################################################
# Compiler settings, parameters and flags
##########################################################################

# Options for specific architecture
ARCH_FLAGS=-mthumb -mcpu=$(CPU_TYPE) 

# Link for code size
GC=-Wl,--gc-sections

# Create map file
MAP=-Wl,-Map=$(OLOC)/$(OUTFILE).map,--cref

USE_NANO=--specs=nano.specs

CFLAGS =  $(ARCH_FLAGS) $(STARTUP_DEFS) $(OPT_LEVEL) $(DEBUG_OPTS) -DTARGET=$(TARGET) -D$(TARGET) \
		-ffunction-sections -fdata-sections -Wdouble-promotion -Wall $(INCLUDE_PATHS)  $(GCC_DEFINE)
ASFLAGS = -c $(DEBUG_OPTS) $(INCLUDE_PATHS) $(ARCH_FLAGS) $(GCC_DEFINE) \
          -x assembler-with-cpp
LDFLAGS = $(CFLAGS) $(USE_NANO) $(ARCH_FLAGS) -Wl,--no-wchar-size-warning,--gc-sections $(MAP) $(HOST)

OCFLAGS = --strip-unneeded

OBJS =  $(patsubst %.c,%.o,$(CFILES)) $(patsubst %.s,%.o,$(SFILES))
POBJS = $(patsubst %,$(OLOC)/%,$(OBJS))
PDEPS =$(POBJS:.o=.d)

all : build 

$(OLOC)/%.o : %.c
	$(Q)mkdir -p $(basename $@)
	$(call cmd, \$(CC) -c $(CFLAGS) -MMD -o $@ $< ,\
	Compiling $<)

$(OLOC)/%.o : %.s
	$(Q)mkdir -p $(basename $@)
	$(call cmd, \$(AS) $(ASFLAGS) -o  $@ $< ,\
	Assembling $<)

$(OLOC)/%.o : %.S
	$(Q)mkdir -p $(basename $@)
	$(call cmd, \$(AS) $(ASFLAGS) -o  $@ $< ,\
	Assembling $<)

build: $(POBJS) $(SYS_OBJS)
	$(Q)$(LD) $(LDFLAGS) -T $(LD_SCRIPT) -o $(OLOC)/$(OUTFILE).elf $(MAP) $(POBJS) $(LDLIBS)
	$(Q)$(SIZE) $(OLOC)/$(OUTFILE).elf
	$(Q)$(OBJCOPY) $(OCFLAGS) -O binary $(OLOC)/$(OUTFILE).elf $(OLOC)/$(OUTFILE).bin
	$(Q)$(LPCRC) $(OLOC)/$(OUTFILE).bin > /dev/null
	$(Q)$(OBJCOPY) $(OCFLAGS) -O ihex $(OLOC)/$(OUTFILE).elf $(OLOC)/$(OUTFILE).hex 

tags:
	-@etags $(CFILES) 2> /dev/null

clean:
	-$(call cmd, \rm -f $(POBJS) $(LD_TEMP) $(OUTFILE).elf $(OUTFILE).bin $(OUTFILE).hex $(OUTFILE).map $(EXPORT) ,\
	Cleaning )
	$(Q)-rm -rf SourceDoc/*
	$(Q)-rm -rf *~ core
	$(Q)-rm -rf $(OLOC)/*
	$(Q)-rm -rf config/*~
	$(Q)-rm -rf TAGS

flash:	build
	$(TOOLSROOT)/bin/openocd -f $(OPENOCDCONFIG) -c "init; reset halt; halt 5 ;  flash write_image erase $(OLOC)/$(OUTFILE).elf ; reset run; shutdown"

usbf:	build
	$(Q)dd conv=nocreat,notrunc if=ofiles/$(OUTFILE).bin of=/media/$(USER)/CRP\ DISABLD/$(OUTFILE).bin 

gdb-pipe:
	$(GDB) -x config/gdb-pipe.cfg 

doc:
	doxygen $(DOXCONFIG)

export:	clean
	$(call cmd \tar -cvzf $(EXPORT) Makefile config $(VPATH) ,\
	Exporting )

print-%:
	@echo $* is $($*)

pretty:
	$(Q)-$(ASTYLE) --options=config/astyle.conf "*.h" "*.c"

-include $(PDEPS)
