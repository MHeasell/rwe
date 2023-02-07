#!/bin/sh
set -e

install_dir="${PWD}/_protobuf-install"

cd protobuf
source_protobuf_version="$(git rev-parse HEAD)"

built_protobuf_version=""

if [ -f "$install_dir/done" ]; then
    built_protobuf_version="$(cat "$install_dir/done")"
fi

if [ "$source_protobuf_version" != "$built_protobuf_version" ]; then
    echo "built protobuf is a different version than source"
    echo "source protobuf: $source_protobuf_version"
    echo "built protobuf:  $built_protobuf_version"
    echo "cleaning and rebuilding protobuf"
    rm -rf "$install_dir"
    ./autogen.sh
    ./configure "--prefix=$install_dir"
    make -j`nproc`
    make install
    echo "$protobuf_version" > "$install_dir/done"
else
    echo "built protobuf matches source protobuf, skipping protobuf build"
    echo "built protobuf: $built_protobuf_version"
fi
