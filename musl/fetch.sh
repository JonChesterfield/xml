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

CC=$llvmdir/bin/clang
CXX=$llvmdir/bin/clang++

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

# /home/jon/xml/musl/install/include/c++/v1/cstring:65:5:
# error: <cstring> tried including <string.h> but didn't find libc++'s <string.h> header.
# This usually means that your header search paths are not configured properly.
# The header search paths should contain the C++ Standard Library headers before
# any C Standard Library, and you are probably using compiler flags that make that
# not be the case.

muslflags="-nostdinc -nostdlib -I$installdir/include"
cxxmuslflags="-nostdinc -nostdlib -I$installdir/include/c++/v1/ -I$installdir/include"


cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_C_FLAGS="$muslflags"                                            \
      -D CMAKE_C_COMPILER_TARGET=x86_64-unknown-linux-musl                     \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
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
if false
then
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

fi


# libcxx depends on linux
# the files in question are under /usr/include and accesed via
# #include <linux/futex.h>, and optionally linux/random.h
# futex crawls various minor files from asm as well
# not sure how stable this list is with regard to different kernel versions
# make headers_install ARCH=i386 INSTALL_HDR_PATH=/usr is probably a better plan
mkdir -p "$installdir"/include/linux "$installdir"/include/asm "$installdir"/include/asm-generic
for i in \
    linux/futex.h \
        linux/types.h \
        linux/stddef.h \
        linux/posix_types.h \
        asm/types.h \
        asm/posix_types.h \
        asm/posix_types_64.h \
        asm-generic/types.h \
        asm-generic/posix_types.h \
        asm-generic/int-ll64.h \
        asm/bitsperlong.h \
        asm-generic/bitsperlong.h \
    ; do
    cp /usr/include/$i  "$installdir"/include/$i
done
         


cd "$thisdir"
libunwind_dir=build_libunwind
rm -rf $libunwind_dir && mkdir $libunwind_dir && cd $libunwind_dir

cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_C_FLAGS="$muslflags --sysroot=$installdir"                                            \
      -D CMAKE_C_COMPILER_TARGET=x86_64-unknown-linux-musl                     \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_CXX_FLAGS="$muslflags --sysroot=$installdir"                                            \
      -D CMAKE_CXX_COMPILER_TARGET=x86_64-unknown-linux-musl                   \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                    \
      -D LIBUNWIND_USE_COMPILER_RT=ON                                          \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/libunwind

ninja -v && ninja install


# don't think I can install the headers without building the library
# but I probably can install the headers without installing the library
# todo: can we build from the headers in the source tree directly
# decent chance since I think that's probably what the all in one build does

# without doing anything with unicode one gets
# error: unknown rune table for this platform -
# do you mean to define _LIBCPP_PROVIDES_DEFAULT_RUNE_TABLE
# using LIBCXX_ENABLE_UNICODE=off to continue trying to bootstrap

# benchmarks are some c++ programs, can't build them yet

cd $thisdir
libcxx_dir=build_libcxx_nounwind
rm -rf $libcxx_dir && mkdir $libcxx_dir && cd $libcxx_dir

cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_C_FLAGS="$muslflags --sysroot=$installdir"                      \
      -D CMAKE_C_COMPILER_TARGET=x86_64-unknown-linux-musl                     \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_CXX_FLAGS="$muslflags --sysroot=$installdir"                    \
      -D CMAKE_CXX_COMPILER_TARGET=x86_64-unknown-linux-musl                   \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                    \
      -D LIBCXX_USE_COMPILER_RT=ON                                          \
      -D LIBCXX_ENABLE_SHARED=OFF \
      -D LIBCXX_ENABLE_STATIC=ON \
      -D LIBCXX_INCLUDE_BENCHMARKS=OFF \
      -D LIBCXX_ENABLE_LOCALIZATION=OFF\
      -D LIBCXX_ENABLE_UNICODE=OFF \
      -D LIBCXX_CXX_ABI=none \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/libcxx

ninja -v && ninja install -v

cd $thisdir
libcxxabi_dir=build_libcxxabi
rm -rf $libcxxabi_dir && mkdir $libcxxabi_dir && cd $libcxxabi_dir

# LIBCXXABI_USE_LLVM_UNWINDER=ON sounds great and errors that we're not using
# LLVM_ENABLE_RUNTIMES


# libcxxabi depends on libc++ - at least on a lot of headers from it - which
# is difficult since we don't have a libc++ yet, as it depends on libcxxabi
# shared lib tries to link against -lcxx-headers despite nostdlib
cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_C_FLAGS="$muslflags --sysroot=$installdir"                      \
      -D CMAKE_C_COMPILER_TARGET=x86_64-unknown-linux-musl                     \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_CXX_FLAGS="$cxxmuslflags --sysroot=$installdir"                    \
      -D CMAKE_CXX_COMPILER_TARGET=x86_64-unknown-linux-musl                   \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                    \
      -D LIBCXXABI_USE_COMPILER_RT=ON                                          \
      -D LIBCXXABI_USE_LLVM_UNWINDER=OFF                                       \
      -D LIBCXXABI_ENABLE_SHARED=OFF \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/libcxxabi

ninja -v && ninja install

# Now we've got a libunwind, build a new libcxx

cd $thisdir
libcxx_dir=build_libcxx
rm -rf $libcxx_dir && mkdir $libcxx_dir && cd $libcxx_dir

# other interesting flags LIBCXX_ENABLE_STATIC_ABI_LIBRARY
# LIBCXX_STATICALLY_LINK_ABI_IN_STATIC_LIBRARY

# I think libcxx is failing to build using its own headers
# first error is unknown type name ldiv_t, followed but a lot of missing size_t

cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_C_FLAGS="$muslflags --sysroot=$installdir"                      \
      -D CMAKE_C_COMPILER_TARGET=x86_64-unknown-linux-musl                     \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_CXX_FLAGS="$cxxmuslflags --sysroot=$installdir"                    \
      -D CMAKE_CXX_COMPILER_TARGET=x86_64-unknown-linux-musl                   \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                    \
      -D LIBCXX_USE_COMPILER_RT=ON                                          \
      -D LIBCXX_ENABLE_SHARED=OFF \
      -D LIBCXX_ENABLE_STATIC=ON \
      -D LIBCXX_INCLUDE_BENCHMARKS=OFF \
      -D LIBCXX_ENABLE_LOCALIZATION=OFF\
      -D LIBCXX_ENABLE_UNICODE=OFF \
      -D LIBCXX_CXX_ABI=libcxxabi \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/libcxx

ninja -v && ninja install -v


