#
# Copyright (c) 2020
#      Christian CAMIER <christian.c@promethee.services>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

TOP  = $(shell cd ../../..; pwd)

FDEF = mod_sys_linux.xml
SRCS = $(wildcard *.c)
DOCS = $(wildcard *.doc)
HDRS = $(wildcard *.h)
HDCS = $(DOCS:.doc=.d)

.SUFFIXES: .c .doc .d .h .i

.c.i:
	@ x86_64-linux-gnu-gcc -pthread -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -g -fdebug-prefix-map=/build/python3.5-MLq5fN/python3.5-3.5.3=. \
                               -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 -fPIC -DMODULENAME=sys_linux \
			       -DUSE_CONSTRUCTOR -DDEBUG -I/usr/include/python3.5m -E $< -o $@

all: $(FDEF) $(SRCS) $(DOCS) $(HDRS)
	@ echo "Building $$(dirname $$(pwd))"
	@ python3 $(TOP)/bin/python_compile_module $(FDEF) build
	@ ls ../..
	@ pwd
	@ cp build/lib.linux*/sys_linux*.so $(TOP)/lib/modules

clean:
	@ echo "Cleaning $$(pwd)"
	@ rm -fr $(HDCS) build *.i *~

