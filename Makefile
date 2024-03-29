# ORSON/MAKEFILE. Compile, install, uninstall Orson.
#
# Copyright (C) 2015, 2019 James B. Moen and Jade Michael Thornton
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <http://www.gnu.org/licenses/>.
#

# Each action has a comment that describes exactly what it does to your system
# and whether you must be root to do it. Please read these comments carefully!
# This Makefile assumes that GCC is installed.
#
# These directories are where Orson will be installed.
#
#  PREFIX/BINDIR	The Orson translator will be installed here.
#  PREFIX/LIBDIR	The Orson library will be installed here.
#  PREFIX/MANDIR	The Orson "man" page will be installed here.
#
# If these directories do not exist, then they will be created.

prefix = /usr/local
bindir = $(prefix)/bin
libdir = $(prefix)/lib/orson
mandir = $(prefix)/man/man1

localsrcdir = ./src
localbindir = ./bin
localdocdir = ./doc
locallibdir = ./lib

target ?= $(localbindir)/orson

srcs := $(shell find $(localsrcdir) -name *.c)
objs := $(srcs:%.c=%.o)
hdrs := $(wildcard *.h)

# GCC options for compiling Orson. It will also compile with -O1, with -O2, or
# with both -O3 and -fkeep-inline-functions. Other optimizations may work too.

flags ?= -g -Wall -MMD
CC ?= gcc

# ALL. Compile Orson from C source files, leaving it in the current
# directory. You need not be root to do this.

.PHONY: all
all: $(target)

$(localsrcdir)%.o: $(localsrcdir)%.c $(hdrs)
	$(CC) $(flags) -c $< -o $@

$(target): $(objs)
	$(CC) $(objs) -Wall -o $@

clean:
	-rm -f *.o
	-rm -rf $(localbindir)

# INSTALL. Install Orson, by doing these things.
#
#	 01. Make BIN DIRECTORY if it doesn't exist.
#	 02. Make MAN DIRECTORY if it doesn't exist.
#	 03. Make LIB DIRECTORY if it doesn't exist.
#	 04. Compile the Orson compiler.
#	 05. Move the Orson compiler to BIN DIRECTORY.
#	 06. Let root be the owner of the Orson compiler.
#	 07. Let nonroots read and run the Orson compiler, not write it.
#	 08. Copy the Orson man page to MAN DIRECTORY.
#	 09. Let root be the owner of the Orson man page.
#	 10. Let nonroots read the Orson man page, not run or write it.
#	 11. Copy the Orson library to LIB DIRECTORY.
#	 12. Let nonroots read files in the Orson library, not run or write them.
#	 13. Let nonroots enter the C directory in the Orson library.
#	 14. Let nonroots read files in the C directory, not run or write them.
#
# You must be root to do this.

install: $(target)
	mkdir -p $(bindir)
	mkdir -p $(mandir)
	mkdir -p $(libdir)
	cp bin/orson $(bindir)
	chown root $(bindir)/orson
	chmod go-w+rx $(bindir)/orson
	cp doc/orson.1 $(mandir)
	chown root $(mandir)/orson.1
	chmod go-wx+r $(mandir)/orson.1
	cp -r $(locallibdir)/* $(libdir)
	chmod go-wx+r $(libdir)/*
	chmod go+x $(libdir)/C
	chmod go-wx+r $(libdir)/C/*

# UNINSTALL. Undo the effects of MAKE INSTALL, by doing these things.
#
# 	1. Undo the effects of MAKE ALL.
# 	2. Remove the Orson compiler from BIN DIRECTORY.
# 	3. Remove the Orson man page from MAN DIRECTORY.
# 	4. Remove LIB DIRECTORY and its files.
#
# Note that BIN DIRECTORY and MAN DIRECTORY will still exist. You must be root
# to do this.

uninstall: clean
	rm -f $(bindir)/orson
	rm -f $(mandir)/orson.1
	rm -f -r $(libdir)

# Local Variables:
# indent-tabs-mode: t
