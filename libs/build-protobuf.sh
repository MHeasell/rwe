#!/bin/bash

set -euo pipefail

install_dir="${PWD}/_protobuf-install"

pushd protobuf

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

    cmake . "-DCMAKE_INSTALL_PREFIX=$install_dir" -DCMAKE_CXX_STANDARD=14
    cmake --build . --parallel=`nproc`
    cmake --install .

    echo "$source_protobuf_version" > "$install_dir/done"
    echo "finished building protobuf: $source_protobuf_version"
else
    echo "built protobuf matches source protobuf, skipping protobuf build"
    echo "built protobuf: $built_protobuf_version"
fi

popd
