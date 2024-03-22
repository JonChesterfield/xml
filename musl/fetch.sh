#!/bin/bash

# Post from me in 2018... https://www.openwall.com/lists/musl/2018/08/31/6


set -x
set -e

thisdir="`cd "$(dirname "$0")"; pwd`"

musldir="$thisdir/musl"

libctestdir="$thisdir/libc-test"

installdir="$thisdir"/install

llvmdir="$HOME/llvm-build/llvm"

rm -rf $installdir "$thisdir"/*.log

NP=$(nproc)

CC=clang

if [[ -d "$musldir" ]]
then
    echo "musl already present"
else
    git clone git://git.musl-libc.org/musl "$musldir"
fi


if [[ -d "$libctestdir" ]]
then
    echo "lib-test already present"
else 
    git clone git://repo.or.cz/libc-test "$libctestdir"
fi

cd "$musldir"
make clean

# todo, fix the name of builtins
CC=$CC LIBCC="$installdir/lib/libclang_rt.builtins-x86_64.a" ./configure --prefix="$installdir" --syslibdir="$installdir"/lib

# install the headers without building musl
make -j $NP install-headers


# detour to build compiler_rt against those headers
cd "$thisdir"
compiler_rt_dir=build_compiler_rt

rm -rf $compiler_rt_dir && mkdir $compiler_rt_dir && cd $compiler_rt_dir

muslflags="-nostdinc -nostdlib -I$installdir/include"

cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_C_COMPILER=clang                                                \
      -D CMAKE_C_FLAGS="$muslflags"             \
      -D CMAKE_C_COMPILER_TARGET=x86_64-unknown-linux-musl                     \
      -D CMAKE_CXX_COMPILER=clang++                                            \
      -D CMAKE_CXX_COMPILER_TARGET=x86_64-unknown-linux-musl                   \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                    \
      -D COMPILER_RT_BUILD_LIBFUZZER=NO                                        \
      -D COMPILER_RT_BUILD_PROFILE=NO                                          \
      -D COMPILER_RT_BUILD_MEMPROF=NO                                          \
      -D COMPILER_RT_BUILD_ORC=NO                                              \
      -D COMPILER_RT_BUILD_GWP_ASAN=NO                                         \
      -D COMPILER_RT_BUILD_SANITIZERS=NO                                       \
      -D COMPILER_RT_BUILD_XRAY=NO                                             \
      -D COMPILER_RT_DEFAULT_TARGET_TRIPLE=x86_64-unknown-linux-musl           \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/compiler-rt

ninja -v && ninja install
cd "$thisdir"

mv install/lib/linux/*clang* install/lib/ && rmdir install/lib/linux


# now go and build musl

cd "$musldir"
make -j$NP &> "$thisdir"/musl.log

make -j$NP install &> "$thisdir"/musl.install.log

cd "$thisdir"

# try to build a C program
cprog="$thisdir"/../tools/file_to_cdata.c

musllink="-Wl,--start-group $installdir/lib/libc.a $installdir/lib/libclang_rt.builtins-x86_64.a -Wl,--end-group"

$CC --target=x86_64-unknown-linux-musl -no-pie $installdir/lib/crt1.o  $muslflags $cprog $musllink -o a.out


# try to build libc-test
cd "$libctestdir"
cp config.mak.def config.mak

# sysroot is the only option to control the library search paths clang pastes into the link line. nostdlib doesn't do it.
# e.g. all the following cruft turns up despite nostdib
#  "/usr/bin/ld" "--eh-frame-hdr" a"-m" "elf_x86_64" "-static" "-o" "src/functional/tgmath-static.exe" "-L/usr/lib/gcc/x86_64-linux-gnu/12" "-L/usr/lib/gcc/x86_64-linux-gnu/12/../../../x86_64-linux-gnu" "-L/usr/lib/gcc/x86_64-linux-gnu/12/../../../../lib64" "-L/lib/x86_64-linux-gnu" "-L/lib/../lib64" "-L/usr/lib/x86_64-linux-gnu" "-L/usr/lib/../lib64" "-L/usr/lib/x86_64-linux-gnu/../../lib64" "-L/usr/lib/gcc/x86_64-linux-gnu/12/../../.."  "-L/lib" "-L/usr/lib" ...
# with a sysroot, this is reduced to a single "-L$installdir/lib"

cat << EOF >> config.mak
CC = $CC
CFLAGS += --target=x86_64-unknown-linux-musl $muslflags

LDLIBS += $musllink # this needs to end up at the end somehow
LDFLAGS += -nostdlib --sysroot=$installdir
CFLAGS += -Wno-switch-bool
EOF

# This is set in the cflags for api.OBJs which comprehensively breaks the build under clang,
# things like error: switch condition has boolean value [-Werror,-Wswitch-bool]
sed -i 's_ -Werror__g' Makefile

# Unistd has a couple of failures on missing definitions. wasi, bionic just disable both of them (and a few others)
sed -i 's$C(_SC_XOPEN_UUCP)$/* _SC_XOPEN_UUCP */ $' src/api/unistd.c
sed -i 's$C(_PC_TIMESTAMP_RESOLUTION)$/* _PC_TIMESTAMP_RESOLUTION */$' src/api/unistd.c

# Need to use a different crt for static and for shared but don't have a user control to do so
# Also need to use a different libc, the .a one is compiled without fPIC and falls over if linked into a shared library
# at least some test failures are due to this
sed -i 's_$(CC) -static_$(CC) '$installdir'/lib/crt1.o -static_' Makefile
sed -i 's_$(CC) -shared_$(CC) '$installdir'/lib/Scrt1.o -shared_' Makefile

sed -i 's_$(CC) $(LDFLAGS)_$(CC) '$installdir'/lib/Scrt1.o $(LDFLAGS)_' Makefile

make
