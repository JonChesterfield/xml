$(if $(TOOLS_DIR),,$(error vendored.mk requires tools dir))

.PHONY: vendored
vendored:	vendored_libxml2 vendored_libxslt

deepclean::
	rm -rf $(TOOLS_DIR)/libxml2
	rm -rf $(TOOLS_DIR)/libxslt
	rm -f $(TOOLS_DIR)/xmllint.c $(TOOLS_DIR)/xsltproc.c


# considering this caching approach instead of fetch.sh, but want the tar checked in
#vendored/libxml2-2.12.0.tar.xz:
#	wget https://download.gnome.org/sources/libxml2/2.12/libxml2-2.12.0.tar.xz -O $@

.PHONY: vendored_libxml2
vendored_libxml2:	deepclean
	mkdir $(TOOLS_DIR)/libxml2
	tar xf vendored/libxml2-2.12.0.tar.xz -C $(TOOLS_DIR)/libxml2 --strip-components=1
	cd $(TOOLS_DIR)/libxml2 && ./configure --without-iconv --without-icu --without-lzma --without-iso8859x --without-http --without-zlib --without-python
	cd $(TOOLS_DIR)/libxml2 && rm -r doc test result python fuzz example *.py
#	delete some dead architectures
	cd $(TOOLS_DIR)/libxml2 && rm -r os400 win32 vms
#	delete various tests and standalone tools
	cd $(TOOLS_DIR)/libxml2 && rm runtest.c runsuite.c runxmlconf.c xmlcatalog.c test*.c

#	Adjust includes for xmllint.c and move under tools
	sed -i 's_#include "libxml.h"_#include "libxml2/libxml.h"_g' $(TOOLS_DIR)/libxml2/xmllint.c
	sed -i 's_<libxml/\(.*\)>_"libxml2/include/libxml/\1"_g' $(TOOLS_DIR)/libxml2/xmllint.c
	mv $(TOOLS_DIR)/libxml2/xmllint.c $(TOOLS_DIR)/

#	trio.c is missing an include, patch that
	sed -i 's_#include <float.h>_#include <float.h>\n#include <string.h>_g' $(TOOLS_DIR)/libxml2/trio.c
#	probably don't want to trust the config script
	rm $(TOOLS_DIR)/libxml2/xml2-config*
#	drop the .in pre-configure files too
	find $(TOOLS_DIR)/libxml2/ -type f -iname '*.in*' -delete 
#	the .deps directory isn't useful either
	rm -r $(TOOLS_DIR)/libxml2/.deps
	sed -i 's_#include <libxml/xmlversion.h>_#include "include/libxml/xmlversion.h"_g' $(TOOLS_DIR)/libxml2/libxml.h

#	Currently thinking of rewriting include paths instead of passing -I flags to the compiler
	sed -i 's_<libxml/\(.*\)>_"\1"_g' $(TOOLS_DIR)/libxml2/include/libxml/*.h
	sed -i 's_<libxml/\(.*\)>_"../libxml/\1"_g' $(TOOLS_DIR)/libxml2/include/private/*.h

	sed -i 's_<libxml/\(.*\)>_"include/libxml/\1"_g' $(TOOLS_DIR)/libxml2/*.c
	sed -i 's_"private/\(.*\)"_"include/private/\1"_g' $(TOOLS_DIR)/libxml2/*.c


.PHONY: vendored_libxslt
vendored_libxslt:	deepclean
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

	mv $(TOOLS_DIR)/libxslt/xsltproc/xsltproc.c $(TOOLS_DIR)/xsltproc.c
	sed -i 's_"libxslt/\(.*\)"_"libxslt/libxslt/\1"_g' $(TOOLS_DIR)/xsltproc.c
	sed -i 's_"libexslt/\(.*\)"_"libxslt/libexslt/\1"_g' $(TOOLS_DIR)/xsltproc.c
	sed -i 's_<libxml/\(.*\)>_"libxml2/include/libxml/\1"_g' $(TOOLS_DIR)/xsltproc.c
	sed -i 's_<libxslt/\(.*\)>_"libxslt/libxslt/\1"_g' $(TOOLS_DIR)/xsltproc.c
	sed -i 's_<libexslt/\(.*\)>_"libxslt/libexslt/\1"_g' $(TOOLS_DIR)/xsltproc.c
