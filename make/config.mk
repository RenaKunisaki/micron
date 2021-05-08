# Compiler optimization setting.
# Should be left at "s"; otherwise may require editing the link script.
OPTIMIZE ?= s

# Whether to include debug symbols.
DEBUG ?= -g

# Additional defines to pass on command line.
DEFS ?=

WITH_STDLIBS ?= 0
WITH_EXCEPTIONS ?= 0
WITH_RTTI ?= 0

# CPU clock in Hz.
ifndef F_CPU
	# this is ugly, but it's less ugly than attempting to
	# perform actual arithmetic in a makefile.
	ifdef CPU_MHZ
		F_CPU = $(CPU_MHZ)000000
	else
		F_CPU = 72000000
	endif
endif

# Compiler settings
# We include $(SRCDIR) as an include path so that when compiling a preprocessed
# source file from $(BUILDDIR), relative include paths still work.
CFLAGS ?= -mcpu=$(CPU) -mthumb -MMD -std=c++14 -I$(LIBDIR)/src -I$(SRCDIR) \
	-Wall -O$(OPTIMIZE) $(DEBUG) \
	-fno-common -felide-constructors -ffunction-sections -fdata-sections \
	-DF_CPU=$(F_CPU) -DCPU_BITS=$(CPU_BITS) -D$(MCUDEF) -DBOARD_TYPE_$(BOARD_TYPE) $(DEFS)


# Assembler settings
ASFLAGS ?= -mcpu=$(CPU)

# Linker settings
# Since we are using gcc as the linker rather than ld, we need to pass linker
# specific options with the -Wl,<option>,<value> format.
LDFLAGS ?= -mcpu=$(CPU) -mthumb -T$(LINKSCRIPT) \
	-O$(OPTIMIZE) -Wl,--gc-sections
	#-Wl,--print-gc-sections


ifneq ($(WITH_STDLIBS),1)
	CFLAGS += -nostdlib -nostartfiles -nodefaultlibs
endif

ifneq ($(WITH_EXCEPTIONS),1)
	CFLAGS += -fno-exceptions
endif

ifneq ($(WITH_RTTI),1)
	CFLAGS += -fno-rtti
endif


ifeq ($(BUILD_MAP),1)
	LDFLAGS += -Wl,-Map,$(BINDIR)/$(PROJECT).map
endif


# Log settings
# enable LOG_INFO by default.
ifndef LOG_INFO
	LOG_INFO=1
endif

ifeq ($(LOG_COMMANDS),1)
   Q=
else
   Q=@
endif

# this doesn't affect "echo foo", only "$(call INFO,foo)"
ifeq ($(LOG_INFO),1)
	# hack, there doesn't seem to be anything that expands to "all params".
	# in bash we'd use $@ but that has different meaning here
	INFO=@echo $1 $2 $3 $4 $5 $6 $7 $8 $9
else
	INFO=
endif
