#!/bin/bash

set -x
set -e
set -o pipefail

thisdir="`cd "$(dirname "$0")"; pwd`"

llvmsrcdir="$HOME/llvm-project"

installdir="$thisdir"/install


musldir="$thisdir/../musl/musl"
linuxdir="$thisdir/linuxsrc"
wabtdir="$thisdir/wabt"

NP=$(nproc)

CC=$HOME/llvm-install/bin/clang
CXX=$HOME/llvm-install/bin/clang++

if [[ -d "$linuxdir" ]]
then
    echo "Linux dir already present"
else
    mkdir -p "$linuxdir" && cd "$linuxdir"
    apt-get source linux # debian specific, not sure how much an exact version match matters
fi

if [[ -d "$wabtdir" ]]
then
    echo "Wabt dir already present"
else
    git clone --recursive https://github.com/WebAssembly/wabt "$wabtdir"
    cd "$wabtdir"
    git submodule update --init
fi


if true
then
    
rm -rf "$installdir"

cd "$thisdir"
builddir="$thisdir"/build-clang
rm -rf $builddir && mkdir -p $builddir && cd $builddir

## Build clang, tablegen etc running on the native system.

cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_SYSTEM_NAME=Linux                                               \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_ASM_COMPILER=$CC                                                \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                    \
      -D LLVM_ENABLE_ASSERTIONS=On                                             \
      -D LLVM_ENABLE_PROJECTS="clang;lld"                                      \
      -D LLVM_USE_LINKER=lld                                                   \
      -D LLVM_ENABLE_ZLIB=OFF                                                  \
      -D LLVM_ENABLE_ZSTD=OFF                                                  \
      -D LLVM_ENABLE_TERMINFO=OFF                                              \
      -D CLANG_DEFAULT_LINKER=lld                                              \
      -D CLANG_DEFAULT_CXX_STDLIB=libc++                                       \
      -D CLANG_DEFAULT_RTLIB=compiler-rt                                       \
      -D CLANG_DEFAULT_UNWINDLIB=none                                          \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/llvm

ninja -v && ninja -v install

# This is needed by the cross build but isn't installed
cp "$builddir"/bin/clang-ast-dump "$installdir"/bin

cd "$wabtdir"
builddir="$thisdir"/build-wabt
rm -rf $builddir && mkdir -p $builddir && cd $builddir

cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir"                                    \
      -G Ninja                                                                 \
      -S "$wabtdir"
ninja -v && ninja -v install

fi

CC="$installdir"/bin/clang
CXX="$installdir"/bin/clang++
AR="$installdir"/bin/llvm-ar
NM="$installdir"/bin/llvm-nm
RANLIB="$installdir"/bin/llvm-ranlib



# Linux headers for musl
cd "$linuxdir"
cd $(find . -mindepth 1 -maxdepth 1 -type d)
make headers_install ARCH=x86_64 INSTALL_HDR_PATH="$installdir/x86_64-unknown-linux-musl"
make headers_install ARCH=arm64 INSTALL_HDR_PATH="$installdir/aarch64-unknown-linux-musl"



# Build static musl libc
for TRIPLE in x86_64-unknown-linux-musl aarch64-unknown-linux-musl ; do
    cd "$musldir"
    make clean
    CC=$CC \
    AR=$AR \
    RANLIB=$RANLIB \
        LIBCC="/dev/zero" \
        CFLAGS="$GLOBALFLAGS --target=$TRIPLE" \
           ./configure --disable-shared \
           --target="$TRIPLE" \
           --prefix="$installdir/$TRIPLE" \
           --syslibdir="$installdir/$TRIPLE/lib"

    make -j"$NP" && make -j"$NP" install
done

# Build static wasi libc. Folder tree layout is different to other sysroot
# but clang seems to expect this, so leave it as-is
for TRIPLE in wasm32-unknown-wasi ; do
make -C "$thisdir"/wasi-sdk/src/wasi-libc clean
make -C "$thisdir"/wasi-sdk/src/wasi-libc \
     CC=$CC \
     AR=$AR \
     NM=$NM \
     INSTALL_DIR="$installdir/$TRIPLE" \
     TARGET_TRIPLE="wasm32-wasi" \
     THREAD_MODEL="single" \
     BUILTINS_LIB="$installdir/lib/clang/19/lib/linux/libclang_rt.builtins-something.a" \
     -j"$NP" install

done




for TRIPLE in wasm32-unknown-wasi x86_64-unknown-linux-musl aarch64-unknown-linux-musl ; do
    
    # LINKER_HACK to placate cmake's tests, building static libs here    
    if [ "$TRIPLE" == wasm32-unknown-wasi ]; then
        LINKER_HACK="-Wl,--no-entry"
        RUNTIME_PROJECTS="libcxx;libcxxabi"
        COMPILER_RT_OPTIONS="-DCOMPILER_RT_BUILD_CRT=FALSE"
        # No exceptions, no libunwind
        CMAKE_OPTIONS="-D LIBCXX_HAS_EXTERNAL_THREAD_API:BOOL=OFF -D LIBCXX_BUILD_EXTERNAL_THREAD_LIBRARY:BOOL=OFF -D LIBCXX_HAS_WIN32_THREAD_API:BOOL=OFF -D LIBCXX_ENABLE_EXCEPTIONS:BOOL=OFF -D LIBCXX_ENABLE_ABI_LINKER_SCRIPT:BOOL=OFF -D LIBCXXABI_ENABLE_EXCEPTIONS:BOOL=OFF -D LIBCXXABI_HAS_PTHREAD_API:BOOL=OFF -D LIBCXXABI_HAS_EXTERNAL_THREAD_API:BOOL=OFF -D LIBCXXABI_BUILD_EXTERNAL_THREAD_LIBRARY:BOOL=OFF -D LIBCXXABI_HAS_WIN32_THREAD_API:BOOL=OFF -D LIBCXXABI_USE_LLVM_UNWINDER=OFF"

    else
        LINKER_HACK=""
        RUNTIME_PROJECTS="libcxx;libcxxabi;libunwind"
        COMPILER_RT_OPTIONS="-DCOMPILER_RT_BUILD_CRT=TRUE"
        # Can do exceptions (though might choose not to), build libunwind
        CMAKE_OPTIONS="-D LIBCXX_ENABLE_EXCEPTIONS:BOOL=ON -D LIBCXX_ENABLE_ABI_LINKER_SCRIPT:BOOL=OFF -D LIBCXXABI_ENABLE_EXCEPTIONS:BOOL=ON -D LIBUNWIND_USE_COMPILER_RT=ON -D LIBUNWIND_ENABLE_SHARED=OFF -D LIBCXXABI_USE_LLVM_UNWINDER=ON -D LIBCXXABI_ENABLE_STATIC_UNWINDER=ON"
    fi


    # compiler-rt is installed to clang resource dir
    cd "$thisdir"
    builddir="$thisdir"/build-compiler-rt/$TRIPLE
    rm -rf $builddir && mkdir -p $builddir && cd $builddir

# compiler_target is an unsuccessful attempt to stop compiler-rt using wasm32-unknown-unknown:
# $HOME/xml/wasm/install/bin/clang --target=wasm32-unknown-wasi --sysroot=$HOME/xml/wasm/install/wasm32-unknown-wasi ...  --target=wasm32-unknown-wasi ... -O3 -DNDEBUG --target=wasm32-unknown-unknown -fno-lto ...  -std=c11 -fPIC -fno-builtin ... -o compiler-rt/lib/builtins/CMakeFiles/clang_rt.builtins-wasm32.dir/umoddi3.c.o -c $HOME/llvm-project/compiler-rt/lib/builtins/umoddi3.c
# difficult to find where in the cmake is forcing this, but setting baremetal seems
# to get a successful compilation despite this. Hopefully close enough.

# RUNTIME_TARGETS and various cmake flags did not work as hoped, building them separately
cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_ASM_COMPILER=$CC                                                \
      -D CMAKE_C_COMPILER_TARGET="$TRIPLE"                                     \
      -D CMAKE_CXX_COMPILER_TARGET="$TRIPLE"                                   \
      -D CMAKE_ASM_COMPILER_TARGET="$TRIPLE"                                   \
      -D CMAKE_C_FLAGS="$GLOBALFLAGS --target=$TRIPLE"                         \
      -D CMAKE_CXX_FLAGS="$GLOBALFLAGS --target=$TRIPLE"                       \
      -D CMAKE_ASM_FLAGS="--target=$TRIPLE"                                    \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX=$($CC -print-resource-dir)                       \
      -D CMAKE_C_COMPILER_WORKS=1                                              \
      -D CMAKE_CXX_COMPILER_WORKS=1                                            \
      -D COMPILER_RT_DEFAULT_TARGET_TRIPLE="$TRIPLE"                           \
      -D LLVM_ENABLE_RUNTIMES="compiler-rt"                                    \
      "$COMPILER_RT_OPTIONS"                                                   \
      -D LLVM_ENABLE_PER_TARGET_RUNTIME_DIR=TRUE                               \
      -D LLVM_ENABLE_ASSERTIONS=On                                             \
      -D COMPILER_RT_BAREMETAL_BUILD=TRUE                                      \
      -D COMPILER_RT_BUILD_LIBFUZZER=NO                                        \
      -D COMPILER_RT_BUILD_PROFILE=NO                                          \
      -D COMPILER_RT_BUILD_MEMPROF=NO                                          \
      -D COMPILER_RT_BUILD_ORC=NO                                              \
      -D COMPILER_RT_BUILD_GWP_ASAN=NO                                         \
      -D COMPILER_RT_BUILD_SANITIZERS=NO                                       \
      -D COMPILER_RT_BUILD_XRAY=NO                                             \
      -D CMAKE_SYSROOT="$installdir/$TRIPLE"                                   \
      -D CMAKE_EXE_LINKER_FLAGS="-static -nostdlib -nostartfiles $LINKER_HACK" \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/runtimes
ninja -v && ninja -v install

# C++ libs installed to top level sysroot, not to clang resource dir
cd "$thisdir"
builddir="$thisdir"/build-runtimes/$TRIPLE
rm -rf $builddir && mkdir -p $builddir && cd $builddir

cmake -D CMAKE_BUILD_TYPE=Release                                              \
      -D CMAKE_SYSTEM_NAME=Linux                                               \
      -D CMAKE_C_COMPILER=$CC                                                  \
      -D CMAKE_CXX_COMPILER=$CXX                                               \
      -D CMAKE_ASM_COMPILER=$CC                                                \
      -D CMAKE_C_FLAGS="$GLOBALFLAGS --target=$TRIPLE"                         \
      -D CMAKE_CXX_FLAGS="$GLOBALFLAGS --target=$TRIPLE"                       \
      -D CMAKE_ASM_FLAGS="--target=$TRIPLE"                                    \
      -D CMAKE_INSTALL_LIBDIR=lib                                              \
      -D CMAKE_INSTALL_PREFIX="$installdir/$TRIPLE"                            \
      -D CMAKE_C_COMPILER_WORKS=1                                              \
      -D CMAKE_CXX_COMPILER_WORKS=1                                            \
      -D LLVM_ENABLE_RUNTIMES="$RUNTIME_PROJECTS"                              \
      -D LLVM_ENABLE_ASSERTIONS=On                                             \
      -D LIBCXXABI_USE_COMPILER_RT=ON                                          \
      -D LIBCXX_USE_COMPILER_RT=ON                                             \
      -D LIBUNWIND_ENABLE_SHARED=OFF                                           \
      -D LIBCXXABI_ENABLE_SHARED=OFF                                           \
      -D LIBCXX_ENABLE_SHARED=OFF                                              \
      -D LIBCXX_ENABLE_FILESYSTEM=ON                                           \
      -D LIBCXX_HAS_MUSL_LIBC=ON                                               \
      -D LIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON                                   \
      -D LIBCXX_INCLUDE_BENCHMARKS=OFF                                         \
      -D LIBCXX_CXX_ABI=libcxxabi                                              \
      -D CMAKE_SYSROOT="$installdir/$TRIPLE"                                   \
      -D CMAKE_EXE_LINKER_FLAGS="-static -nostdlib -nostartfiles $LINKER_HACK" \
      $CMAKE_OPTIONS                                                           \
      -G Ninja                                                                 \
      -S $HOME/llvm-project/runtimes
ninja -v && ninja -v install

done

rm -rf "$thisdir"/build-clang "$thisdir"/build-compiler-rt "$thisdir"/build-runtimes
make -C "$thisdir"/wasi-sdk/src/wasi-libc clean
make -C "$musldir" clean

