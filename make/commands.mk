# Tool/command names and invocation functions
TOOL=$(TOOLPATH)/bin/$(TOOLPREFIX)
CC=$(Q)$(TOOL)g++
AS=$(Q)$(TOOL)as
AR=$(Q)$(TOOL)ar
LD=$(Q)$(TOOL)g++
OBJCOPY=$(Q)$(TOOL)objcopy
OBJDUMP=$(Q)$(TOOL)objdump
SIZE=$(Q)$(TOOL)size
DELETE=$(Q)rm -rf
MKDIR=$(Q)mkdir -p

# function COMPILE(infile, outfile)
COMPILE=$(CC) $(CFLAGS) -c $1 -o $2

# function PREPROCESS(infile, outfile)
# uses the GNU M4 preprocessor, and a bit of messing about with subshells
# and path manipulation to make relative include paths work in macros.
PREPROCESS=$(Q)( cd $(dir $1) && m4 -I$(LIBDIR)/src -I$(SRCDIR) -s $(notdir $1) > $(abspath $2) )

# function ASSEMBLE(infile, outfile)
ASSEMBLE=$(AS) $(ASFLAGS) $1 -o $2

# function LINK(infile, outfile)
LINK=$(CC) $1 $(LDFLAGS) -o $2

# function BIN2O(infile, outfile, label)
BIN2O=$(Q)bin2o $1 $2 $3

# function PROGRAM(file) - uploads file to MCU
PROGRAM=$(Q)teensy_loader_cli -w -s -v -mmcu=$(MCU) --device=$(USB_VID):$(USB_PID) $1

# function FIND(path, pattern)
# find specified pattern in specified path and subdirs.
FIND=$(shell find -L $1 -type f -name $2)

# function FINDABS(path, pattern)
# same as FIND, but returns absolute paths.
FINDABS=$(abspath $(call FIND,$1,$2))
