#!/bin/bash

# Libxml2 directory is derived from the contents of the libxml2-2.12.0.tar.xz release.
# Initially by deleting tests, docs, python.

rm -rf libxml2 xmllint.c
wget https://download.gnome.org/sources/libxml2/2.12/libxml2-2.12.0.tar.xz
tar xf libxml2-2.12.0.tar.xz && rm libxml2-2.12.0.tar.xz && mv libxml2-2.12.0 libxml2 && cd libxml2

./configure --without-iconv --without-icu --without-lzma --without-iso8859x --without-http --without-zlib

rm -r doc test result python os400 win32 fuzz example *.py
rm runtest.c test*.c

# delete some more standalone files
rm runsuite.c runxmlconf.c xmlcatalog.c

# want this one, but will move it to tools
sed -i 's_#include "libxml.h"_#include "libxml2/libxml.h"_g' xmllint.c
mv xmllint.c ..

cd ../..
make libraries
cd .libraries.O/
gcc xmllint.o libxml2/*.o -o xmllint -lm


