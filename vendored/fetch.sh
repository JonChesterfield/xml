#/bin/bash

wget https://download.gnome.org/sources/libxml2/2.12/libxml2-2.12.0.tar.xz
wget https://download.gnome.org/sources/libxslt/1.1/libxslt-1.1.39.tar.xz

# Most recent release
wget https://github.com/wasm3/wasm3/archive/refs/tags/v0.5.0.zip

# Roughly main, will try the out of date release first
# wget https://github.com/wasm3/wasm3/archive/d28e14f3521f58ab836ce8a4609427ef5384cdf4.zip


# Specific hash of uvwasi, could lift the sha from unzipped wasm3
# ./wasm3-0.5.0/CMakeLists.txt:195:    GIT_TAG b063d686848c32a26119513056874f051c74258a
wget https://github.com/vshymanskyy/uvwasi/archive/b063d686848c32a26119513056874f051c74258a.zip -O uvwasi.zip

# v1.40.0 is the revision the above wasm3 defaults to
wget https://github.com/libuv/libuv/archive/refs/tags/v1.40.0.zip -O libuv.zip

