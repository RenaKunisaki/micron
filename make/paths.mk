# Path to your compiler, linker, etc.
#TOOLPATH ?= /usr/local
TOOLPATH ?= /usr

# Name prefix for the compiler, linker, etc.
TOOLPREFIX ?= arm-none-eabi-

# Project dirs
SRCDIR ?= src
BUILDDIR ?= build
BINDIR ?= bin
LINKSCRIPT ?= $(LIBDIR)/make/$(MCU).ld

# All file extensions we need to make object files from.
types=c cpp c.m4 cpp.m4 s bin


# WARNING: Madness follows.


# Get absolute paths of our source dirs.
libsrcabs = $(abspath $(LIBDIR)/src)
srcabs = $(abspath $(SRCDIR)/)

# a couple functions that tidy up source paths.
# This avoids needless nesting in the build dir. eg:
# build/usr/local/include/micron/src/foo/blah.c => build/micron/foo/blah.c
# build/src/foo/blah.c => build/foo/blah.c
stripsrc=$(subst $(srcabs)/,,$1)
striplib=$(subst $(libsrcabs),micron,$1)
stripboth=$(call stripsrc,$(call striplib,$1))
findstrip=$(call stripboth,$(call FINDABS,$1,$2))
# FINDABS is in commands.mk

# function findobj(path, extension)
# Wrapper for findstrip() that replaces the given extension with '.o'.
findobj=$(patsubst %.$2,%.o,$(call findstrip,$1,'*.$2'))

# function listobjs(path, extension)
# produces a nice, tidy list of object files in build directory for all files
# of type `extension` in source directory `path`.
listobjs=$(addprefix $(BUILDDIR)/,$(call findobj,$1,$2))

# Build the grand final object list.
OBJS ?= $(foreach type,$(types),$(call listobjs,$(srcabs),$(type)))
OBJS += $(foreach type,$(types),$(call listobjs,$(libsrcabs),$(type)))
OBJS += $(EXTRA_OBJS)
