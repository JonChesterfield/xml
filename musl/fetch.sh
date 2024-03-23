#!/bin/bash

# Post from me in 2018... https://www.openwall.com/lists/musl/2018/08/31/6


set -x
set -e

thisdir="`cd "$(dirname "$0")"; pwd`"

musldir="$thisdir/musl"

libctestdir="$thisdir/libc-test"

installdir="$thisdir"/install

llvmsrcdir="$HOME/llvm-project"
llvmdir="$HOME/llvm-install"

NP=$(nproc)

CC=$llvmdir/bin/clang
CXX=$llvmdir/bin/clang++

musllink="-Wl,--start-group $installdir/lib/libc.a $installdir/lib/libclang_rt.builtins-x86_64.a -Wl,--end-group"

# this might mean clang's stdint is chosen over musl
# the challenge is that building clang requires headers like immintrin.h which are
# available in clang, but not in the sysroot before clang is built
# not ideal to use the ones from the initial clang but not terrible
muslflags="-nostdlibinc -nostdlib -nostartfiles -I$installdir/include"
cxxmuslflags="-nostdlibinc -nostdlib -nostartfiles -I$installdir/include/c++/v1/ -I$installdir/include"


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


if true; then

rm -rf $installdir "$thisdir"/*.log


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
        asm/prctl.h \
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
         

## libunwind start
cd "$thisdir"
libunwind_dir=build_libunwind
rm -rf $libunwind_dir && mkdir $libunwind_dir && cd $libunwind_dir

cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_ASM_COMPILER=$CC                                                  \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_ASM_COMPILER_TARGET=x86_64-unknown-linux-musl                     \
      -D CMAKE_C_COMPILER_TARGET=x86_64-unknown-linux-musl                     \
      -D CMAKE_CXX_COMPILER_TARGET=x86_64-unknown-linux-musl                   \
      -D CMAKE_C_FLAGS="$muslflags --sysroot=$installdir"                                            \
      -D CMAKE_ASM_FLAGS=" --sysroot=$installdir"                                            \
      -D CMAKE_CXX_FLAGS="$muslflags --sysroot=$installdir"                                            \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                    \
      -D LIBUNWIND_USE_COMPILER_RT=ON                                          \
      -D LIBUNWIND_ENABLE_SHARED=OFF \
      -D CMAKE_SYSTEM_NAME=Linux \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/libunwind

ninja -v && ninja install

## libunwind complete

# don't think I can install the headers without building the library
# but I probably can install the headers without installing the library
# todo: can we build from the headers in the source tree directly? no, config_site

# without doing anything with unicode one gets
# error: unknown rune table for this platform -
# do you mean to define _LIBCPP_PROVIDES_DEFAULT_RUNE_TABLE
# using LIBCXX_ENABLE_UNICODE=off to continue trying to bootstrap

# benchmarks are some c++ programs, can't build them yet
# maybe libcxx can be built against the header

## libcxx start
cd $thisdir
libcxx_dir=build_libcxx
rm -rf $libcxx_dir && mkdir $libcxx_dir && cd $libcxx_dir

cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_C_FLAGS="$muslflags --sysroot=$installdir"                      \
      -D CMAKE_C_COMPILER_TARGET=x86_64-unknown-linux-musl                     \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_CXX_FLAGS="-I$llvmsrcdir/libcxxabi/include $muslflags --sysroot=$installdir"                    \
      -D CMAKE_CXX_COMPILER_TARGET=x86_64-unknown-linux-musl                   \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                    \
      -D LIBCXX_USE_COMPILER_RT=ON                                          \
      -D LIBCXX_ENABLE_SHARED=OFF \
      -D LIBCXX_ENABLE_STATIC=ON \
      -D LIBCXX_HAS_MUSL_LIBC=ON \
      -D LIBCXX_INCLUDE_BENCHMARKS=OFF \
      -D LIBCXX_CXX_ABI=libcxxabi \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/libcxx

#      -D LIBCXX_ENABLE_LOCALIZATION=OFF\
#      -D LIBCXX_ENABLE_UNICODE=OFF \

ninja -v && ninja install -v
## libcxx complete

## libcxxabi start, have a libc++ install to build against
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

## libcxxabi complete

# repackage libunwind and libc++abi into libc++.a
cd $thisdir
echo "create $installdir/lib/libcombinedc++.a
addlib $installdir/lib/libunwind.a
addlib $installdir/lib/libc++abi.a
addlib $installdir/lib/libc++.a
save
end" | llvm-ar -M
rm $installdir/lib/libunwind.a $installdir/lib/libc++abi.a $installdir/lib/libc++.a
mv $installdir/lib/libcombinedc++.a $installdir/lib/libc++.a


# try to build a clang linked against musl and targetting musl by default

fi

cd $thisdir
llvm_dir=build_llvm
rm -rf $llvm_dir && mkdir $llvm_dir && cd $llvm_dir
# some of the benchmarks build with Werror, include fcntl.h and then complain about
# flexible array members, even though clang understands what a trailing char [] is
# and that's under _GNU_SOURCE as well. Switch off the benchmarks instead.

# This is a mess.
# Cmake has it's own special ways of specifying things like the names of standard libs
# and where they are, and if you ignore that there's no other way to append to a link line
# However it doesn't pass these to the various compiler identification hacks it does
# Also LLVM does it's own compiler identification hacks which use a different set of
# flags again
# the COMPILER_WORKS hacks are currently present because cmake doesn't
# pass it's own C STANDARD LIBRARIES flag to executables it builds to test if
# the standard libary (e.g. pthread) is available.
#
# the CheckAtomic.cmake construct doesn't pass the CMAKE_CXX_STANDARD_INCLUDE dirs
# value through, so that needs to be specified in cxx_flags as well

#      -D CMAKE_C_FLAGS="$muslflags"                      \
#      -D CMAKE_CXX_FLAGS="$cxxmuslflags -L$installdir/lib"                 \


# Better is to use standard libraries:
#  -D CMAKE_CXX_STANDARD_LIBRARIES="$installdir/lib/libc++.a $musllink"
#  but haven't managed to get through the various ad hoc toolchain tests with that
# because they don't pass SHARED_LINKER_FLAGS etc
# SYSTEM_NAME=Generic might be an escape hatch to turn off some of the compiler tests

# documentation suggests the runtimes can be built under the runtimes subdir
# that's probably a better time than component wise if they work and worse
# if they dont
# -B build
#cmake -D CMAKE_BUILD_TYPE=Release                                              \
#      -D LLVM_ENABLE_PROJECTS="clang" \
#      -D LLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
#      -D LLVM_RUNTIME_TARGETS="x86_64-unknown-linux-musl" \
#      -G Ninja \
#      -S $HOME/llvm-project/llvm
##      exit 0

# following isn't working yet. problem is things like Hello.so are not shared
# libraries as far as cmake is concerned so they don't get the extra arguments
# build is doing something dubious with a subdir called NATIVE and tablegen, to
# which some of the musl related flags are being passed and some aren't

cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_AR=$llvmdir/bin/llvm-ar                                         \
      -D CMAKE_NM=$llvmdir/bin/llvm-nm                                         \
      -D CMAKE_RANLIB=$llvmdir/bin/llvm-ranlib                                 \
      -D CMAKE_OBJCOPY=$llvmdir/bin/llvm-objcopy                               \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_ASM_COMPILER=$CC                                                \
      -D CMAKE_C_COMPILER_TARGET=x86_64-unknown-linux-musl                     \
      -D CMAKE_ASM_COMPILER_TARGET=x86_64-unknown-linux-musl                     \
      -D CMAKE_CXX_COMPILER_TARGET=x86_64-unknown-linux-musl                   \
      -D CMAKE_C_FLAGS="-I$installdir/include"                      \
      -D CMAKE_CXX_FLAGS="-I$installdir/include/c++/v1/ -I$installdir/include"                 \
      -D CMAKE_SYSTEM_NAME=Linux \
      -D CMAKE_SYSROOT=$installdir \
      -D CMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
      -D CMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
      -D CMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
      -D CMAKE_C_STANDARD_LIBRARIES="$musllink" \
      -D CMAKE_C_STANDARD_INCLUDE_DIRECTORIES="$installdir/include" \
      -D CMAKE_CXX_STANDARD_LIBRARIES="" \
      -D CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES="$installdir/include/c++/v1/;$installdir/include" \
      -D CMAKE_EXE_LINKER_FLAGS="-nostdlib -nostartfiles $installdir/lib/Scrt1.o --rtlib=compiler-rt -fuse-ld=lld -Wl,--whole-archive $installdir/lib/libc++.a $musllink -Wl,--no-whole-archive" \
      -D CMAKE_STATIC_LINKER_FLAGS="" \
      -D CMAKE_SHARED_LINKER_FLAGS="-nostdlib -nostartfiles --rtlib=compiler-rt -fuse-ld=lld -Wl,--whole-archive $installdir/lib/libc++.a $musllink -Wl,--no-whole-archive" \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                    \
      -D LLVM_DEFAULT_TARGET_TRIPLE=x86_64-unknown-linux-musl \
      -D LLVM_ENABLE_PROJECTS="clang;lld" \
      -D LLVM_USE_LINKER=lld \
      -D LLVM_ENABLE_LIBCXX=ON \
      -D LLVM_ENABLE_ZLIB=OFF \
      -D LLVM_ENABLE_ZSTD=OFF \
      -D LLVM_ENABLE_TERMINFO=OFF \
      -D LLVM_INCLUDE_EXAMPLES=OFF \
      -D LLVM_INCLUDE_TESTS=FALSE \
      -D LLVM_INCLUDE_BENCHMARKS=FALSE \
      -D DEFAULT_SYSROOT="$installdir" \
      -D CLANG_DEFAULT_LINKER=lld \
      -D CLANG_DEFAULT_CXX_STDLIB=libc++ \
      -D CLANG_DEFAULT_RTLIB=compiler-rt \
      -D CLANG_DEFAULT_UNWINDLIB=none \
      -D CMAKE_C_COMPILER_FORCED=TRUE \
      -D CMAKE_CXX_COMPILER_FORCED=TRUE \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/llvm

# failed to persuade cmake that we don't need libatomic
#      -D HAVE_CXX_ATOMICS_WITHOUT_LIB=TRUE \
#      -D CMAKE_C_COMPILER_WORKS=1 \
#      -D CMAKE_CXX_COMPILER_WORKS=1 \

ninja -v
