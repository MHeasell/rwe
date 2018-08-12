#!/usr/bin/env bash

# Builds and publishes the MSYS2/MinGW64 version of RWE.

set -euo pipefail

# Refresh packages list
pacman -Syq

# Install build tools
pacman -Sq --needed --noconfirm \
    make \
    unzip \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-toolchain

# Install project dependencies
pacman -Sq --needed --noconfirm \
    mingw-w64-x86_64-boost \
    mingw-w64-x86_64-SDL2 \
    mingw-w64-x86_64-SDL2_image \
    mingw-w64-x86_64-SDL2_mixer \
    mingw-w64-x86_64-SDL2_net \
    mingw-w64-x86_64-glew \
    mingw-w64-x86_64-smpeg2 \
    mingw-w64-x86_64-zlib \
    mingw-w64-x86_64-libpng

# Build protobuf
pushd libs
./build-protobuf.sh
popd

mkdir build
pushd build

# Build the project
cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=$Configuration ..
make -j 2
./rwe_test

# Create the build artifacts
make -j 2 package

# Push the build artifacts
pushd dist
for i in *; do
    appveyor PushArtifact "$i"
done
popd

popd
