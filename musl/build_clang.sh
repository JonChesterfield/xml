#!/bin/bash

set -x
set -e

thisdir="`cd "$(dirname "$0")"; pwd`"
NP=$(nproc)

installdir="$thisdir"/install
bootstrapdir="$thisdir"/bootstrap

llvmsrcdir="$HOME/llvm-project"

CC=$HOME/llvm-install/bin/clang
CXX=$HOME/llvm-install/bin/clang++

musldir="$thisdir/musl"

linuxdir="$thisdir/linuxsrc"


rm -rf "$installdir"

if [[ -d "$linuxdir" ]]
then
    echo "Linux dir already present"
else
    mkdir -p "$linuxdir" && cd "$linuxdir"
    apt-get source linux # debian specific, not sure how much an exact version match matters
fi


cd "$linuxdir"
cd $(find . -mindepth 1 -maxdepth 1 -type d)
make headers_install ARCH=x86_64 INSTALL_HDR_PATH="$installdir"

if true
then
rm -rf "$bootstrapdir"

# Build a clang that targets musl by default. This hacks around it being difficult to
# persuade cmake to do the right thing in terms of consistently passing flags around

cd "$thisdir"
clang_dir=build_clang
rm -rf $clang_dir && mkdir $clang_dir && cd $clang_dir

# clang doesn't look in sysroot for compiler-rt, try building it a local copy
# might want LLVM_HOST_TRIPLE instead of default target triple, might need both
cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_ASM_COMPILER=$CC                                                \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$bootstrapdir"                                  \
      -D LLVM_ENABLE_PROJECTS="clang;lld"                                      \
      -D LLVM_TARGET_TRIPLE=x86_64-unknown-linux-musl                          \
      -D LLVM_DEFAULT_TARGET_TRIPLE=x86_64-unknown-linux-musl                  \
      -D LLVM_USE_LINKER=lld                                                   \
      -D LLVM_ENABLE_ZLIB=OFF                                                  \
      -D LLVM_ENABLE_ZSTD=OFF                                                  \
      -D LLVM_ENABLE_TERMINFO=OFF                                              \
      -D DEFAULT_SYSROOT="$installdir"                                         \
      -D GCC_INSTALL_PREFIX="$installdir" \
      -D CLANG_DEFAULT_LINKER=lld                                              \
      -D CLANG_DEFAULT_CXX_STDLIB=libc++                                       \
      -D CLANG_DEFAULT_RTLIB=compiler-rt                                       \
      -D CLANG_DEFAULT_UNWINDLIB=none                                          \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/llvm

ninja -v && ninja -v install

fi

GLOBALFLAGS="-fPIC"

CC=$bootstrapdir/bin/clang
CXX=$bootstrapdir/bin/clang++

cd "$musldir"
make clean

CC=$CC LIBCC="$installdir/lib/clang/19/lib/linux/libclang_rt.builtins-x86_64.a" CFLAGS="$GLOBALFLAGS" ./configure --prefix="$installdir" --syslibdir="$installdir"/lib

make -j $NP install-headers

# Clang depends on Linux headers. e.g. apt-get source linux


if false
then
mkdir -p "$installdir"/include/linux "$installdir"/include/asm "$installdir"/include/asm-generic "$installdir"/include/x86_64-linux-gnu/asm
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
        x86_64-linux-gnu/asm/unistd_64.h \
    ; do
    cp /usr/include/$i  "$installdir"/include/$i
done
fi

cd "$thisdir"
runtimes_dir=build_runtimes
rm -rf $runtimes_dir && mkdir $runtimes_dir && cd $runtimes_dir

# builds static libraries only - they don't need to be linked against a libc or a compiler-rt
cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_C_FLAGS="$GLOBALFLAGS" \
      -D CMAKE_CXX_FLAGS="$GLOBALFLAGS" \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                    \
      -D LLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi;libunwind"         \
      -D COMPILER_RT_BUILD_LIBFUZZER=NO                                        \
      -D COMPILER_RT_BUILD_PROFILE=NO                                          \
      -D COMPILER_RT_BUILD_MEMPROF=NO                                          \
      -D COMPILER_RT_BUILD_ORC=NO                                              \
      -D COMPILER_RT_BUILD_GWP_ASAN=NO                                         \
      -D COMPILER_RT_BUILD_SANITIZERS=NO                                       \
      -D COMPILER_RT_BUILD_XRAY=NO                                             \
      -D COMPILER_RT_DEFAULT_TARGET_TRIPLE=x86_64-unknown-linux-musl           \
      -D LIBUNWIND_USE_COMPILER_RT=ON                                          \
      -D LIBCXXABI_USE_COMPILER_RT=ON                                          \
      -D LIBCXX_USE_COMPILER_RT=ON                                             \
      -D LIBUNWIND_ENABLE_SHARED=OFF                                           \
      -D LIBCXXABI_ENABLE_SHARED=OFF                                           \
      -D LIBCXX_ENABLE_SHARED=OFF                                              \
      -D LIBCXXABI_USE_LLVM_UNWINDER=ON                                        \
      -D LIBCXX_HAS_MUSL_LIBC=ON                                               \
      -D LIBCXXABI_ENABLE_STATIC_UNWINDER=ON                                   \
      -D LIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON                                   \
      -D LIBCXX_INCLUDE_BENCHMARKS=OFF                                         \
      -D LIBCXX_CXX_ABI=libcxxabi                                              \
      -D CMAKE_EXE_LINKER_FLAGS="-static -nostdlib -nostartfiles"              \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/runtimes
ninja -v && ninja -v install

# This has put compiler-rt in $installdir/lib/linux but clang looks for it
# under $installdir/lib/clang/19/lib/linux/. Whatever, move it, and give bootstrap a copy.
 
mkdir -p $bootstrapdir/lib/clang/19/lib/linux/
mkdir -p $installdir/lib/clang/19/lib/linux/
cp $installdir/lib/linux/* $bootstrapdir/lib/clang/19/lib/linux/
mv $installdir/lib/linux/* $installdir/lib/clang/19/lib/linux/
rmdir $installdir/lib/linux


# Build libc now we have a compiler-rt to link against the loader
cd "$musldir"
make -j $NP && make -j $NP install


# Now try to build clang and the llvm tools
cd "$thisdir"
clang_dir=build_clang
rm -rf $clang_dir && mkdir $clang_dir && cd $clang_dir



# clang looks next to itself for these, not in the sysroot

# can't find atomic because it hasn't put c++/v1 on the include path
# failed to work out how that's meant to be working, looked like it involved gcc_install_prefix
# but that didn't help
# various shared libaries are built as part of tests, tools (bugpoint, opt) etc
# these currently fail because things weren't compiled with fpic and that failure is fatal
# inclined to compile everything with fpic despite it being unhelpful for the static binaries
# hello.so succeeded without the relocation warning, guess it doesn't use that data type
# examples did not (R_X86_64_PC32 cannot be used...)

# might be using the wrong include paths despite sysroot, though that might be the first build
# In file included from /home/jon/llvm-project/clang/include/clang/AST/ASTVector.h:22:
# In file included from /usr/lib/gcc/x86_64-linux-gnu/12/../../../../include/c++/12/algorithm:61:
#       -D CMAKE_CXX_FLAGS="-fPIC -I$installdir/include/c++/v1" \
#      -D CMAKE_C_STANDARD_INCLUDE_DIRECTORIES="$installdir/include" \
#      -D CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES="$installdir/include/c++/v1/;$installdir/include" \
# maybe -DBUILD_SHARED_LIBS=OFF

# skip rpath because cmake tries to mangle rpath on static binaries and falls over
# without -static, tries to use dynamic linker under / which doesn't exist
# with -static, libclang erroneously applies EXE_LINKER_FLAGS to itself and fails to build
#      -D CMAKE_EXE_LINKER_FLAGS="-static"              \
# llvm_include_tools false disables clang, trying clang_build_tools=off but that still
# tries (and fails) to build libclang. commenting out the libclang cmake line.
# arcmt links against libclang, disable that too
# added LLVM_BUILD_STATIC, which along with some find_library hacks adds
# -static to EXE_LINKER_FLAGS

# Should be able to use  -D CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES="$installdir/include/c++/v1/;$installdir/include"
# and in a brighter world clang would find the libc++ headers anyway, but right now that means CheckAtomic.cmake fails
# to find <atomic> and falls over


cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_ASM_COMPILER=$CC                                                \
      -D CMAKE_C_FLAGS="$GLOBALFLAGS" \
      -D CMAKE_CXX_FLAGS="$GLOBALFLAGS -I$installdir/include/c++/v1"                  \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                  \
      -D LLVM_ENABLE_PROJECTS="clang;lld" \
      -D LLVM_DEFAULT_TARGET_TRIPLE=x86_64-unknown-linux-musl \
      -D LLVM_BUILD_STATIC=ON \
      -D LLVM_USE_LINKER=lld \
      -D LLVM_ENABLE_LIBCXX=ON \
      -D LLVM_ENABLE_ZLIB=OFF \
      -D LLVM_ENABLE_ZSTD=OFF \
      -D LLVM_ENABLE_LIBEDIT=OFF\
      -D LLVM_ENABLE_LIBXML2=OFF \
      -D LLVM_ENABLE_LIBPFM=OFF \
      -D LLVM_ENABLE_TERMINFO=OFF \
      -D DEFAULT_SYSROOT="$installdir" \
      -D GCC_INSTALL_PREFIX="$installdir" \
      -D CLANG_DEFAULT_LINKER=lld \
      -D CLANG_DEFAULT_CXX_STDLIB=libc++ \
      -D CLANG_DEFAULT_RTLIB=compiler-rt \
      -D CLANG_DEFAULT_UNWINDLIB=none \
      -D LLVM_INCLUDE_BENCHMARKS=FALSE \
      -D LLVM_INCLUDE_TESTS=FALSE \
      -D LLVM_INCLUDE_EXAMPLES=FALSE \
      -D LIBCLANG_BUILD_STATIC=ON \
      -D CMAKE_SKIP_RPATH=TRUE \
      -D CLANG_BUILD_TOOLS=ON \
      -D CLANG_ENABLE_ARCMT=ON \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/llvm
ninja -v && ninja -v install


# If that worked out, we now have a complete statically linked toolchain under install. No dynamic libaries but otherwise good.

