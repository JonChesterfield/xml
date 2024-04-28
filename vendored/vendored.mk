$(if $(TOOLS_DIR),,$(error vendored.mk requires tools dir))

# should probably handle tools/cmark here as well
# maybe produce a tarball for evilunit instead of pulling from github

vendored::	vendored_libxml2 vendored_libxslt vendored_evilunit
#	Clean up some additional unused files
	@find $(TOOLS_DIR)/libx??? -type f '(' -iname Makefile -o -iname NEWS -o -iname '*.Po' -o -iname '*.Plo' -o -iname '*.in' -o -iname '*.am' -o -iname '*.pl' -o -iname '*.syms' -o -name configure -o -name libtool -o -name ltmain.sh -o -name CMakeLists.txt -o -name INSTALL -o -name missing ')' -delete
	@find $(TOOLS_DIR)/libx??? -type d -iname .deps -delete
	@rm -rf $(TOOLS_DIR)/libx???/m4

deepclean::
	rm -rf $(TOOLS_DIR)/libxml2
	rm -rf $(TOOLS_DIR)/libxslt
	rm -f $(TOOLS_DIR)/xmllint.c $(TOOLS_DIR)/xsltproc.c
	rm -rf $(TOOLS_DIR)/EvilUnit

# considering this caching approach instead of fetch.sh, but want the tar checked in
#vendored/libxml2-2.12.0.tar.xz:
#	wget https://download.gnome.org/sources/libxml2/2.12/libxml2-2.12.0.tar.xz -O $@

.PHONY: vendored_libxml2
vendored_libxml2:	deepclean
#	extract the tar
	mkdir $(TOOLS_DIR)/libxml2
	tar xf vendored/libxml2-2.12.0.tar.xz -C $(TOOLS_DIR)/libxml2 --strip-components=1
	cd $(TOOLS_DIR)/libxml2 && ./configure --without-iconv --without-icu --without-lzma --without-iso8859x --without-http --without-zlib --without-python
#	delete docs, tests, python
	cd $(TOOLS_DIR)/libxml2 && rm -r doc test result python fuzz example *.py
#	delete some dead architectures
	cd $(TOOLS_DIR)/libxml2 && rm -r os400 win32 vms
#	delete various tests and standalone tools
	cd $(TOOLS_DIR)/libxml2 && rm runtest.c runsuite.c runxmlconf.c xmlcatalog.c test*.c

#	Adjust includes for xmllint.c and move under tools
	mv $(TOOLS_DIR)/libxml2/xmllint.c $(TOOLS_DIR)/xmllint.c
	sed -i 's_#include "libxml.h"_#include "libxml2/libxml.h"_g' $(TOOLS_DIR)/xmllint.c
	sed -i 's_<libxml/\(.*\)>_"libxml2/include/libxml/\1"_g' $(TOOLS_DIR)/xmllint.c

#	trio.c is missing an include, patch that
	sed -i 's_#include <float.h>_#include <float.h>\n#include <string.h>_g' $(TOOLS_DIR)/libxml2/trio.c
#	the &va_list construct in trio.c provokes spurious warnings from clang
	sed -i 's_(&(l))_((void*)\&(l))_' $(TOOLS_DIR)/libxml2/trio.c

#	trio.c subtracts a null pointer with a comment about avoiding compiler warnings, but
#	as subtracting the null pointer has undefined behavior according to clang, don't do that
	sed -i 's_((char \*)pointer - (char \*)0)_((char *)pointer /*- (char *)0*/)_' $(TOOLS_DIR)/libxml2/trio.c


#	triostr.c uses strcasecmp which is in strings.h on linux, but doesn't include that
	sed -i 's_#include <string.h>_#include <string.h>\n#include <strings.h>_g' $(TOOLS_DIR)/libxml2/triostr.c

#	probably don't want to trust the config script
	rm $(TOOLS_DIR)/libxml2/xml2-config*

#	Rewrite the includes to be relative to the file location, keeps the build-C top level simple
	sed -i 's_#include <libxml/xmlversion.h>_#include "include/libxml/xmlversion.h"_g' $(TOOLS_DIR)/libxml2/libxml.h
	sed -i 's_<libxml/\(.*\)>_"\1"_g' $(TOOLS_DIR)/libxml2/include/libxml/*.h
	sed -i 's_<libxml/\(.*\)>_"../libxml/\1"_g' $(TOOLS_DIR)/libxml2/include/private/*.h
	sed -i 's_<libxml/\(.*\)>_"include/libxml/\1"_g' $(TOOLS_DIR)/libxml2/*.c
	sed -i 's_"private/\(.*\)"_"include/private/\1"_g' $(TOOLS_DIR)/libxml2/*.c


.PHONY: vendored_libxslt
vendored_libxslt:	deepclean
#	extract the tar
	mkdir $(TOOLS_DIR)/libxslt
	tar xf vendored/libxslt-1.1.39.tar.xz -C $(TOOLS_DIR)/libxslt --strip-components=1
	cd $(TOOLS_DIR)/libxslt && ./configure --without-python --without-plugins LIBXML_LIBS="skip-test"
	cd $(TOOLS_DIR)/libxslt && rm -r doc python tests examples
	cd $(TOOLS_DIR)/libxslt && rm -r win32 vms
	rm $(TOOLS_DIR)/libxslt/xsltproc/testThreads.c

	cd $(TOOLS_DIR)/libxslt && sed -i 's_<libxml/\(.*\)>_"../../libxml2/include/libxml/\1"_g' libexslt/*.c libexslt/*.h libxslt/*.c libxslt/*.h
	sed -i 's_<libxslt/\(.*\)>_"../libxslt/\1"_g' $(TOOLS_DIR)/libxslt/libexslt/*.c $(TOOLS_DIR)/libxslt/libexslt/*.h
	sed -i 's_#include "libexslt/libexslt.h"_#include "libexslt.h"_g' $(TOOLS_DIR)/libxslt/libexslt/*.c
	sed -i 's_#include "config.h"_#include "../config.h"_g' $(TOOLS_DIR)/libxslt/libxslt/libxslt.h $(TOOLS_DIR)/libxslt/libexslt/libexslt.h

	sed -i 's_<libxslt/\(.*\)>_"\1"_g' $(TOOLS_DIR)/libxslt/libxslt/*.h
	sed -i 's_<libexslt/\(.*\)>_"\1"_g' $(TOOLS_DIR)/libxslt/libexslt/*.c $(TOOLS_DIR)/libxslt/libexslt/*.h
#	Move xsltproc to tools dir and fix up the includes
	mv $(TOOLS_DIR)/libxslt/xsltproc/xsltproc.c $(TOOLS_DIR)/xsltproc.c
	sed -i 's_"libxslt/\(.*\)"_"libxslt/libxslt/\1"_g' $(TOOLS_DIR)/xsltproc.c
	sed -i 's_"libexslt/\(.*\)"_"libxslt/libexslt/\1"_g' $(TOOLS_DIR)/xsltproc.c
	sed -i 's_<libxml/\(.*\)>_"libxml2/include/libxml/\1"_g' $(TOOLS_DIR)/xsltproc.c
	sed -i 's_<libxslt/\(.*\)>_"libxslt/libxslt/\1"_g' $(TOOLS_DIR)/xsltproc.c
	sed -i 's_<libexslt/\(.*\)>_"libxslt/libexslt/\1"_g' $(TOOLS_DIR)/xsltproc.c

.PHONY: vendored_evilunit
vendored_evilunit:	deepclean
	git clone https://github.com/JonChesterfield/EvilUnit.git $(TOOLS_DIR)/EvilUnit
