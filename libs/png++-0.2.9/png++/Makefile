#
# Copyright (C) 2007,2008   Alex Shulgin
#
# This file is part of png++ the C++ wrapper for libpng.  PNG++ is free
# software; the exact copying conditions are as follows:
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# 3. The name of the author may not be used to endorse or promote products
# derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
# NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

PNGPP := .

# don't forget to update version before releasing!
version := 0.2.9

build_files := common.mk Makefile Doxyfile
doc_files := AUTHORS BUGS ChangeLog COPYING INSTALL NEWS README TODO
headers := $(wildcard *.hpp)
sources :=

include common.mk

dist_dir := png++-$(version)
dist_package := png++-$(version).tar.gz
dist_files := $(build_files) $(doc_files) \
  $(headers) $(sources)
dist_subdirs := example test

all: examples

install: install-headers install-docs

uninstall: uninstall-headers uninstall-docs

install-headers:
	mkdir -p $(PREFIX)/include/png++
	cp $(headers) $(PREFIX)/include/png++

uninstall-headers:
	rm -rf $(PREFIX)/include/png++

dist: dist-mkdir dist-copy-files dist-package

dist-mkdir:
	rm -rf $(dist_dir)
	mkdir $(dist_dir)

dist-copy-files:
	cp $(dist_files) $(dist_dir)
	for i in $(dist_subdirs); do \
		$(MAKE) dist-copy-files -C $$i $(MAKEFLAGS) \
		  dist_dir=`pwd`/$(dist_dir); \
	done

dist-package:
	rm -f $(dist_package)
	tar -zcf $(dist_package) $(dist_dir) --exclude=.svn --exclude='*~'
	rm -rf $(dist_dir)

clean: test-clean examples-clean

thorough-clean: clean docs-clean

check: test

test:
	$(MAKE) test -C test $(MAKEFLAGS)

test-clean:
	$(MAKE) clean -C test $(MAKEFLAGS)

test-compile-headers: $(headers:%.hpp=%.hpp.o)

%.hpp.o:
	$(CXX) -c $(@:%.hpp.o=%.hpp) -o /dev/null $(make_cflags)

docs:
	VERSION=$(version) doxygen

docs-clean:
	rm -rf doc

install-docs:
	if [ -d doc ]; then \
		dir=$(PREFIX)/share/doc/$(dist_dir); \
		rm -rf $$dir; \
		mkdir -p $$dir \
		&& cp -r $(doc_files) doc/html $$dir; \
		cd $(PREFIX)/share/doc; \
		[ -L png++ ] && rm png++; \
		[ -d png++ ] || ln -s $(dist_dir) png++; \
	fi

uninstall-docs:
	rm -rf $(PREFIX)/share/doc/$(dist_dir) $(PREFIX)/share/doc/png++

examples:
	$(MAKE) -C example $(MAKEFLAGS)

examples-clean:
	$(MAKE) clean -C example $(MAKEFLAGS)

.PHONY: install \
  dist dist-mkdir dist-copy-files dist-package \
  thorough-clean \
  check test test-clean test-compile-headers \
  docs docs-clean \
  examples examples-clean
