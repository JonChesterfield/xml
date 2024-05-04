$(if $(TOOLS_DIR),,$(error vendored.mk requires tools dir))

# should probably handle tools/cmark here as well
# maybe produce a tarball for evilunit instead of pulling from github

vendored::	vendored_evilunit
vendored::	vendored_libxml2 vendored_libxslt
vendored::	vendored_wasm3 vendored_libuv vendored_uvwasi

vendored::
#	Clean up some additional unused files
	@find $(TOOLS_DIR)/libx??? -type f '(' -iname Makefile -o -iname NEWS -o -iname '*.Po' -o -iname '*.Plo' -o -iname '*.in' -o -iname '*.am' -o -iname '*.pl' -o -iname '*.syms' -o -name configure -o -name libtool -o -name ltmain.sh -o -name CMakeLists.txt -o -name INSTALL -o -name missing ')' -delete
	@find $(TOOLS_DIR)/libx??? -type d -iname .deps -delete
	@rm -rf $(TOOLS_DIR)/libx???/m4

deepclean::
	rm -rf $(TOOLS_DIR)/libxml2
	rm -rf $(TOOLS_DIR)/libxslt
	rm -f $(TOOLS_DIR)/xmllint.c $(TOOLS_DIR)/xsltproc.c
	rm -rf $(TOOLS_DIR)/wasm3 $(TOOLS_DIR)/wasm3.c
	rm -rf $(TOOLS_DIR)/libuv
	rm -rf $(TOOLS_DIR)/uvwasi
	rm -rf $(TOOLS_DIR)/EvilUnit


.PHONY: vendored_evilunit
vendored_evilunit:	deepclean
	git clone https://github.com/JonChesterfield/EvilUnit.git $(TOOLS_DIR)/EvilUnit


# considering this caching approach instead of fetch.sh, but want the tar checked in
#vendored/libxml2-2.12.0.tar.xz:
#	wget https://download.gnome.org/sources/libxml2/2.12/libxml2-2.12.0.tar.xz -O $@

.PHONY: vendored_libxml2
vendored_libxml2:	deepclean
#	extract the tar
	mkdir $(TOOLS_DIR)/libxml2
	tar xf vendored/libxml2-2.12.0.tar.xz -C $(TOOLS_DIR)/libxml2 --strip-components=1
	cd $(TOOLS_DIR)/libxml2 && ./configure --without-iconv --without-icu --without-lzma --without-iso8859x --without-http --without-zlib --without-python --without-threads
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

#	config.h probably decided mmap was available. Disable it for wasm
	@echo "#ifdef __wasi__" >> $(TOOLS_DIR)/libxml2/config.h
	@echo "#undef HAVE_MMAP" >> $(TOOLS_DIR)/libxml2/config.h
	@echo "#undef HAVE_MUNMAP" >> $(TOOLS_DIR)/libxml2/config.h
	@echo "#undef HAVE_SYS_MMAN_H" >> $(TOOLS_DIR)/libxml2/config.h
	@echo "#endif" >> $(TOOLS_DIR)/libxml2/config.h

#	wasi behaves a fair amount like a unix, cygwin is a reasonable place to insert the check
	sed -i 's$$#elif defined(__CYGWIN__)$$#elif defined(__wasi__)\n# define TRIO_PLATFORM_UNIX\n&$$' $(TOOLS_DIR)/libxml2/triodef.h

#	dubious hack, compromising the claim to be unix stops this file trying to use signals
	sed -i 's$$#include "triodef.h"$$&\n#ifdef __wasi__\n#undef TRIO_PLATFORM_UNIX\n#endif\n$$' $(TOOLS_DIR)/libxml2/trionan.c

	sed -i 's$$#[ ]*include <signal.h>$$// &$$' $(TOOLS_DIR)/libxml2/trio.c
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


.PHONY: vendored_wasm3
vendored_wasm3:	deepclean
	rm -rf -- vendored/wasm3tmp && mkdir -p vendored/wasm3tmp
	unzip vendored/v0.5.0.zip -d vendored/wasm3tmp
	mv vendored/wasm3tmp/wasm3-0.5.0 $(TOOLS_DIR)/wasm3
	rmdir vendored/wasm3tmp
# Move the main executable to the top level for consistency with other tools
	mv $(TOOLS_DIR)/wasm3/platforms/app/main.c $(TOOLS_DIR)/wasm3.c

#	Hack source instead of requiring -Isource
	sed -i -E 's$$#include "(.+[.]h)"$$#include "wasm3/source/\1"$$' $(TOOLS_DIR)/wasm3.c
	mv $(TOOLS_DIR)/wasm3/source/extensions/* $(TOOLS_DIR)/wasm3/source/ && rmdir $(TOOLS_DIR)/wasm3/source/extensions
#	Hack source instead of requiring -Dd_m3HasWASI, only edits the files that use the macro for now
	for F in $(TOOLS_DIR)/wasm3.c $(TOOLS_DIR)/wasm3/source/m3_api_wasi.c ; do mv $$F tmp ; echo "#define d_m3HasWASI 1" >> $$F; cat tmp >> $$F; rm tmp ; done

#	Clean up
	cd $(TOOLS_DIR)/wasm3 && rm -r test docs extra platforms .github .gitignore .codespellrc CMakeLists.txt source/CMakeLists.txt

.PHONY: vendored_libuv
vendored_libuv:	deepclean
	rm -rf -- vendored/libuv && mkdir -p vendored/libuv
	unzip vendored/libuv.zip -d vendored/libuv
	mv vendored/libuv/libuv-1.40.0 $(TOOLS_DIR)/libuv

.PHONY: vendored_uvwasi
vendored_uvwasi:	deepclean
	rm -rf -- vendored/uvwasi && mkdir -p vendored/uvwasi
	unzip vendored/uvwasi.zip -d vendored/uvwasi
	mv vendored/uvwasi/uvwasi-b063d686848c32a26119513056874f051c74258a $(TOOLS_DIR)/uvwasi
	rmdir vendored/uvwasi
	for hdr in wasi_types wasi_serdes uvwasi ; do \
	sed -i -E \
	-e 's$$#include "('$$hdr')[.]h"$$#include "../include/\1.h"$$' \
	$(TOOLS_DIR)/uvwasi/src/*.[hc] ; done
	cd $(TOOLS_DIR)/uvwasi && rm -r test
