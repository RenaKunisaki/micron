# Make commands/variables common to all target CPUs.
# Get the directory of this makefile. (thanks to 0xff and Flimm)
MAKEFILE_PATH := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
LIBDIR := $(MAKEFILE_PATH)/..

include $(MAKEFILE_PATH)/config.mk
include $(MAKEFILE_PATH)/usb.mk
include $(MAKEFILE_PATH)/paths.mk
include $(MAKEFILE_PATH)/commands.mk

# Target files
ELF=$(BINDIR)/$(PROJECT).elf
HEX=$(BINDIR)/$(PROJECT).hex
BIN=$(BINDIR)/$(PROJECT).bin
DIRS=$(BINDIR) $(BUILDDIR)

.PHONY: all dirs clean stats program disasm dump-make-vars

all: $(HEX) $(BIN) stats
	$(call INFO,Done.)

$(BIN): dirs $(ELF)
	$(call INFO," [*] Building $@...")
	$(OBJCOPY) -O binary -j .text -j .data $(ELF) $@

$(HEX): dirs $(ELF)
	$(call INFO," [*] Building $@...")
	$(OBJCOPY) -R .stack -O ihex $(ELF) $@

$(ELF): dirs $(OBJS)
	$(call INFO," [*] Linking $@...")
	$(call LINK, $(OBJS), $(ELF))

dirs:
	$(MKDIR) $(DIRS) $(dir $(OBJS))

clean:
	$(DELETE) $(DIRS)

# Display stats of the compiled binary
stats: $(ELF)
	$(SIZE) $(ELF)

# Upload to the chip
program: $(HEX)
	$(call PROGRAM,$(HEX))

# Disassemble binary
disasm: $(ELF)
	$(OBJDUMP) -trdS -Mreg-names-std $(ELF) > $(BINDIR)/$(PROJECT).asm

# for makefile debugging
dump-make-vars:
	@echo 'SRCDIR = $(SRCDIR)'
	@echo 'BUILDDIR = $(BUILDDIR)'
	@echo 'BINDIR = $(BINDIR)'
	@echo 'CFILES = $(CFILES)'
	@echo 'CPPFILES = $(CPPFILES)'
	@echo 'CM4FILES = $(CM4FILES)'
	@echo 'CPPM4FILES = $(CPPM4FILES)'
	@echo 'ASFILES = $(ASFILES)'
	@echo 'BINFILES = $(BINFILES)'
	@echo 'OBJS = $(OBJS)'
	@echo
	@echo 'TOOLPATH = $(TOOLPATH)'
	@echo 'TOOLPREFIX = $(TOOLPREFIX)'
	@echo 'TOOL = $(TOOL)'
	@echo 'CC = $(CC)'
	@echo 'AS = $(AS)'
	@echo 'AR = $(AR)'
	@echo 'LD = $(LD)'
	@echo 'OBJCOPY = $(OBJCOPY)'
	@echo 'OBJDUMP = $(OBJDUMP)'
	@echo 'SIZE = $(SIZE)'
	@echo 'DELETE = $(DELETE)'
	@echo 'MKDIR = $(MKDIR)'
	@echo 'COMPILE = $(COMPILE)'
	@echo 'PREPROCESS = $(PREPROCESS)'
	@echo 'ASSEMBLE = $(ASSEMBLE)'
	@echo 'LINK = $(LINK)'
	@echo 'BIN2O = $(BIN2O)'
	@echo 'PROGRAM = $(PROGRAM)'


# Compiler generated dependency info
-include $(OBJS:.o=.d)

$(BUILDDIR)/micron/%.o: $(LIBDIR)/src/%.c
	$(call INFO," [*] Compiling $(subst $(LIBDIR)/src,micron,$<)...")
	$(call COMPILE, $<, $@)


$(BUILDDIR)/%.c: $(SRCDIR)/%.c.m4
	$(call INFO," [*] Preprocessing $<...")
	$(call PREPROCESS, $<, $@)

$(BUILDDIR)/%.cpp: $(SRCDIR)/%.cpp.m4
	$(call INFO," [*] Preprocessing $<...")
	$(call PREPROCESS, $<, $@)

$(BUILDDIR)/%.h: $(SRCDIR)/%.h.m4
	$(call INFO," [*] Preprocessing $<...")
	$(call PREPROCESS, $<, $@)


$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(call INFO," [*] Compiling $<...")
	$(call COMPILE, $<, $@)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(call INFO," [*] Compiling $<...")
	$(call COMPILE, $<, $@)

$(BUILDDIR)/%.o: $(BUILDDIR)/%.c
	$(call INFO," [*] Compiling $<...")
	$(call COMPILE, $<, $@)

$(BUILDDIR)/%.o: $(BUILDDIR)/%.cpp
	$(call INFO," [*] Compiling $<...")
	$(call COMPILE, $<, $@)

$(BUILDDIR)/%.o: $(SRCDIR)/%.s
	$(call INFO," [*] Assembling $<...")
	$(call COMPILE, $<, $@)

$(BUILDDIR)/%.o: $(SRCDIR)/%.bin
	$(call INFO," [*] Converting $<...")
	$(call BIN2O, $<, $@, $(basename $(notdir $<)))
