.PHONY: vendored
vendored:	vendored_libxml2

deepclean::
	rm -rf tools/libxml2
	rm -f tools/xmllint.c

.PHONY: vendored_libxml2
vendored_libxml2:	deepclean
	mkdir tools/libxml2
	tar xf vendored/libxml2-2.12.0.tar.xz -C tools/libxml2 --strip-components=1
	cd tools/libxml2 && ./configure --without-iconv --without-icu --without-lzma --without-iso8859x --without-http --without-zlib
	cd tools/libxml2 && rm -r doc test result python fuzz example *.py
#	delete some dead architectures
	cd tools/libxml2 && rm -r os400 win32 vms
#	delete various tests and standalone tools
	cd tools/libxml2 && rm runtest.c runsuite.c runxmlconf.c xmlcatalog.c test*.c

#	Adjust includes for xmllint.c and move under tools
	sed -i 's_#include "libxml.h"_#include "libxml2/libxml.h"_g' tools/libxml2/xmllint.c
	sed -i 's_<libxml/\(.*\)>_"libxml2/include/libxml/\1"_g' tools/libxml2/xmllint.c
	mv tools/libxml2/xmllint.c tools/

#	trio.c is missing an include, patch that
	sed -i 's_#include <float.h>_#include <float.h>\n#include <string.h>_g' tools/libxml2/trio.c
#	probably don't want to trust the config script
	rm tools/libxml2/xml2-config*
#	drop the .in pre-configure files too
	find tools/libxml2/ -type f -iname '*.in*' -delete 
#	the .deps directory isn't useful either
	rm -r tools/libxml2/.deps
	sed -i 's_#include <libxml/xmlversion.h>_#include "include/libxml/xmlversion.h"_g' tools/libxml2/libxml.h

#	Currently thinking of rewriting include paths instead of passing -I flags to the compiler
	sed -i 's_<libxml/\(.*\)>_"\1"_g' tools/libxml2/include/libxml/*.h
	sed -i 's_<libxml/\(.*\)>_"../libxml/\1"_g' tools/libxml2/include/private/*.h

	sed -i 's_<libxml/\(.*\)>_"include/libxml/\1"_g' tools/libxml2/*.c
	sed -i 's_"private/\(.*\)"_"include/private/\1"_g' tools/libxml2/*.c

