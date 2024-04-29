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


TRIPLE="wasm32-unknown-wasi"
rm -rf "$installdir"

boot_dir="$thisdir"/build_bootstrap

if true
then
rm -rf "$bootstrapdir" && mkdir "$bootstrapdir"

# Build a clang that targets musl by default. This hacks around it being difficult to
# persuade cmake to do the right thing in terms of consistently passing flags around
# It also means we've got a tablegen etc to use later

cd "$thisdir"
rm -rf $boot_dir && mkdir $boot_dir && cd $boot_dir

# clang doesn't look in sysroot for compiler-rt, try building it a local copy
# might want LLVM_HOST_TRIPLE instead of default target triple, might need both
cmake -D CMAKE_BUILD_TYPE=Release                                                \
      -D CMAKE_SYSTEM_NAME=Linux                                               \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_ASM_COMPILER=$CC                                                \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$bootstrapdir"                                  \
      -D LLVM_ENABLE_ASSERTIONS=On                                             \
      -D LLVM_ENABLE_PROJECTS="clang;lld"                                      \
      -D LLVM_TARGET_TRIPLE=$TRIPLE                          \
      -D LLVM_DEFAULT_TARGET_TRIPLE=$TRIPLE                  \
      -D LLVM_USE_LINKER=lld                                                   \
      -D LLVM_ENABLE_ZLIB=OFF                                                  \
      -D LLVM_ENABLE_ZSTD=OFF                                                  \
      -D LLVM_ENABLE_TERMINFO=OFF                                              \
      -D DEFAULT_SYSROOT="$installdir"                                         \
      -D CLANG_DEFAULT_LINKER=lld                                              \
      -D CLANG_DEFAULT_CXX_STDLIB=libc++                                       \
      -D CLANG_DEFAULT_RTLIB=compiler-rt                                       \
      -D CLANG_DEFAULT_UNWINDLIB=none                                          \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/llvm

ninja -v && ninja -v install
else
cd $boot_dir
ninja -v && ninja -v install
fi

# This is needed by the build but isn't installed
cp "$boot_dir"/bin/clang-ast-dump "$bootstrapdir"/bin

GLOBALFLAGS=""

CC="$bootstrapdir"/bin/clang
CXX="$bootstrapdir"/bin/clang++
AR="$bootstrapdir"/bin/llvm-ar
NM="$bootstrapdir"/bin/llvm-nm


# Can build wasi without compiler-rt
make -C "$thisdir"/wasi-sdk/src/wasi-libc \
     CC=$CC \
     AR=$AR \
     NM=$NM \
     INSTALL_DIR="$installdir" \
     TARGET_TRIPLE="wasm32-wasi" \
     THREAD_MODEL="single" \
     BUILTINS_LIB="$installdir/lib/clang/19/lib/linux/libclang_rt.builtins-something.a" \
     -j"$NP" install

# BUILTINS_LIB defaults to something like libgcc, but it's only used by the so

for i in include lib share; do
    mv "$installdir"/$i/wasm32-wasi/* "$installdir"/$i/
    rmdir "$installdir"/$i/wasm32-wasi
done

cd "$thisdir"
runtimes_dir="$thisdir"/build_runtimes
rm -rf $runtimes_dir && mkdir $runtimes_dir && cd $runtimes_dir

# builds static libraries only - they don't need to be linked against a libc or a compiler-rt
# no libunwind
cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_SYSTEM_NAME=Linux                                               \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_C_FLAGS="$GLOBALFLAGS"                                          \
      -D CMAKE_CXX_FLAGS="$GLOBALFLAGS"                                        \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                    \
      -D LLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi"                   \
      -D LLVM_ENABLE_ASSERTIONS=On                                             \
      -D COMPILER_RT_BUILD_LIBFUZZER=NO                                        \
      -D COMPILER_RT_BUILD_PROFILE=NO                                          \
      -D COMPILER_RT_BUILD_MEMPROF=NO                                          \
      -D COMPILER_RT_BUILD_ORC=NO                                              \
      -D COMPILER_RT_BUILD_GWP_ASAN=NO                                         \
      -D COMPILER_RT_BUILD_SANITIZERS=NO                                       \
      -D COMPILER_RT_BUILD_XRAY=NO                                             \
      -D COMPILER_RT_DEFAULT_TARGET_TRIPLE=$TRIPLE                             \
      -D LIBUNWIND_USE_COMPILER_RT=ON                                          \
      -D LIBCXXABI_USE_COMPILER_RT=ON                                          \
      -D LIBCXX_USE_COMPILER_RT=ON                                             \
      -D LIBUNWIND_ENABLE_SHARED=OFF                                           \
      -D LIBCXXABI_ENABLE_SHARED=OFF                                           \
      -D LIBCXX_ENABLE_SHARED=OFF                                              \
      -D LIBCXX_HAS_MUSL_LIBC=ON                                               \
      -D LIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON                                   \
      -D LIBCXX_INCLUDE_BENCHMARKS=OFF                                         \
      -D LIBCXX_CXX_ABI=libcxxabi                                              \
      -D CMAKE_EXE_LINKER_FLAGS="-static -nostdlib -nostartfiles -Wl,--no-entry"              \
    -DLIBCXX_HAS_EXTERNAL_THREAD_API:BOOL=OFF \
    -DLIBCXX_BUILD_EXTERNAL_THREAD_LIBRARY:BOOL=OFF \
    -DLIBCXX_HAS_WIN32_THREAD_API:BOOL=OFF \
    -DLIBCXX_ENABLE_EXCEPTIONS:BOOL=OFF \
    -DLIBCXX_ENABLE_FILESYSTEM:BOOL=ON \
    -DLIBCXX_ENABLE_ABI_LINKER_SCRIPT:BOOL=OFF \
    -DLIBCXXABI_ENABLE_EXCEPTIONS:BOOL=OFF \
    -DLIBCXXABI_HAS_PTHREAD_API:BOOL=OFF \
    -DLIBCXXABI_HAS_EXTERNAL_THREAD_API:BOOL=OFF \
    -DLIBCXXABI_BUILD_EXTERNAL_THREAD_LIBRARY:BOOL=OFF \
    -DLIBCXXABI_HAS_WIN32_THREAD_API:BOOL=OFF \
    -DLIBCXXABI_USE_LLVM_UNWINDER:BOOL=OFF \
    -G Ninja                                                                 \
      -S $HOME/llvm-project/runtimes
ninja -v && ninja -v install

