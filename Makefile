# This makefile is used to install Micron.
# For building projects, look at the included examples.

PROJECT=micron
PREFIX=/usr/local/include
INSTALLPATH=$(PREFIX)/$(PROJECT)
THISPATH=$(shell readlink -f .)

MKDIR=mkdir -p
DELETE=rm -rf

.PHONY: all install uninstall install-dev

# There's nothing to compile here, because the library is a set of source files
# which, due to the variety of configurations available, have to be compiled
# into each project.
all:
	@echo Nothing to compile, just run 'make install'.

install:
	$(MKDIR) $(INSTALLPATH)
	cp -r ./* $(INSTALLPATH)

# Install as symlinks to the copies in this directory.
# Useful when developing micron itself.
install-dev:
	$(MKDIR) $(INSTALLPATH)
	find . -mindepth 1 -type d -exec $(MKDIR) $(INSTALLPATH)/{} \;
	find . -type f -exec ln -fs $(THISPATH)/{} $(INSTALLPATH)/{} \;

uninstall:
	$(DELETE) $(INSTALLPATH)
