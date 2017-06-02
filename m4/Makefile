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
# Copyright 2017 Technolution BV  opensource@technolution.eu
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
# associated documentation files (the "Software"), to deal in the Software without restriction, 
# including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
# and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all copies or substantial 
# portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
# LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# 

#VERBOSE=1
DEBUG=1

# Define these to include the specified driver
#INCLUDE_ETHERNET=1
INCLUDE_USB=1
#INCLUDE_CAN=1
#INCLUDE_SDMMC=1

# Define which target is being built
#TARGET_LPC4370=1
TARGET_LPC4367=1

CROSS_COMPILE ?= $(HOME)/bin/armgcc/bin/arm-none-eabi-

##########################################################################
# User configuration and firmware specific object files	
##########################################################################
# Overall system defines for compilation
ifdef DEBUG
GCC_DEFINE=-DDEBUG 
OPT_LEVEL = 
#-O2
else
GCC_DEFINE= -DUSE_I2C_2V8
OPT_LEVEL = 
#-O2
endif

GCC_DEFINE+= -std=gnu99 -DCORE_M4
DOXCONFIG   = config/doxygen.cfg
EXPORT = Garonne.tgz

# The target, flash and ram of the microprocessor.
ifdef TARGET_LPC4370
TARGET = LPC4370
#LD_SCRIPT = config/lpc4370_flash.ld
LD_SCRIPT = config/lpc4330_spitoram.ld
endif

ifdef TARGET_LPC4367
TARGET = LPC4367
LD_SCRIPT = config/lpc4367_flash.ld
endif

CORTEX_TYPE=m4

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
LPCOpen_DIR=thirdparty/lpcopen/lpc_chip_43xx
INCLUDE_PATHS += $(patsubst %,-I%,$(shell find $(LPCOpen_DIR) -name "*.h" -exec dirname {} \; | uniq ))
CFILES += $(shell find $(LPCOpen_DIR) -name "*.c" -print)

# FreeRTOS Files
# ==============
FreeRTOS_DIR=thirdparty/FreeRTOS
INCLUDE_PATHS += $(patsubst %,-I%,$(shell find $(FreeRTOS_DIR) -name "*.h" -exec dirname {} \; | uniq ))
CFILES += $(shell find $(FreeRTOS_DIR) -name "*.c" -print)

ifdef INCLUDE_SDMMC 
# FatFS Files
# ===========
FatFS_DIR=thirdparty/fatfs
INCLUDE_PATHS += $(patsubst %,-I%,$(shell find $(FatFS_DIR) -name "*.h" -exec dirname {} \; | uniq ))
CFILES += $(shell find $(FatFS_DIR) -name "*.c" -print)
endif

# LWIP Files
# ==========
LWIP_DIR=thirdparty/lwip
INCLUDE_PATHS += -I$(LWIP_DIR)/inc -I$(LWIP_DIR)/inc/ipv4 -I$(LWIP_DIR)/inc/apps
CFILES += $(shell find $(LWIP_DIR) -name "*.c" -print)

# VL530L0X Files
# ==============
VL_DIR=thirdparty/VL530L0X
INCLUDE_PATHS += -I$(VL_DIR)/inc 
CFILES += $(shell find $(VL_DIR)/src -name "*.c" -print)

##########################################################################
# Project-specific files 
##########################################################################

ifdef INCLUDE_SDMMC
GCC_DEFINE+=-DINCLUDE_SDMMC
endif

ifdef INCLUDE_ETHERNET 
GCC_DEFINE+=-DINCLUDE_ETHERNET
endif

ifdef INCLUDE_CAN
GCC_DEFINE+=-DINCLUDE_CAN
endif

ifdef INCLUDE_USB
GCC_DEFINE+=-DINCLUDE_USB
endif

# Main Files
# ==========
App_DIR=App
App_Inc_DIR=Inc
INCLUDE_PATHS += -IInc
INCLUDE_PATHS += $(patsubst %,-I%,$(shell find $(App_Inc_DIR) -name "*.h" -exec dirname {} \; | uniq ))
INCLUDE_PATHS += $(patsubst %,-I%,$(shell find $(App_DIR) -name "*.h" -exec dirname {} \; | uniq ))
INCLUDE_PATHS += $(patsubst %,-I%,$(shell find $(OLOC) -name "*.h" -exec dirname {} \; | uniq ))
CFILES += $(shell find $(App_DIR) -name "*.c" -print)

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
LPCRC = Tools/lpcrc/lpcrc-linux
GET_GIT_HASH = Tools/git_hash_to_c/git_hash_to_c.sh
MAKE = make
OUTFILE = Garonne

##########################################################################
# Startup files
#########################################################################

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
HOST=--specs=rdimon.specs -lgcc -lc -lrdimon -lm
else
HOST=-lc -lc -lnosys -lm
endif

##########################################################################
# Compiler settings, parameters and flags
##########################################################################
# filename for embedded git revision 
GIT_HASH_FILENAME=git_version_info.h
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
LDFLAGS = $(CFLAGS) $(USE_NANO) $(ARCH_FLAGS) -Wl,--no-wchar-size-warning,--gc-sections $(MAP) 

OCFLAGS = --strip-unneeded

OBJS =  $(patsubst %.c,%.o,$(CFILES)) $(patsubst %.s,%.o,$(SFILES))
POBJS = $(patsubst %,$(OLOC)/%,$(OBJS))
PDEPS =$(POBJS:.o=.d)

all : build 

get_version:
	$(Q)$(GET_GIT_HASH) > $(OLOC)/$(GIT_HASH_FILENAME)

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

build: get_version $(POBJS) $(SYS_OBJS)
	$(Q)$(LD) $(LDFLAGS) -T $(LD_SCRIPT) -o $(OLOC)/$(OUTFILE).elf $(MAP) $(POBJS) $(LDLIBS) $(HOST)
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

$(generated_dir)/git_head_revision.c:
	mkdir -p $(dir $@)
	../Tools/git_hash_to_c.sh > $@    
    
doc:
	doxygen $(DOXCONFIG)

export:	clean
	$(call cmd \tar -cvzf $(EXPORT) Makefile config $(VPATH) ,\
	Exporting )

print-%:
	@echo $* is $($*)

pretty:
#The exclude is needed to prevent prettifying the cyclic link..no detrimental impact
	$(Q)-$(ASTYLE) --options=config/astyle.conf -i --exclude="Inc/enet/arch" "Inc/*.h" "App/*.c"

-include $(PDEPS)
