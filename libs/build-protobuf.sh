#!/bin/bash

set -euo pipefail

install_dir="${PWD}/_protobuf-install"
path_marker="$install_dir/build_path"

pushd protobuf

source_protobuf_version="$(git rev-parse HEAD)"

built_protobuf_version=""
built_path=""

if [ -f "$install_dir/done" ]; then
    built_protobuf_version="$(cat "$install_dir/done")"
fi

if [ -f "$path_marker" ]; then
    built_path="$(cat "$path_marker")"
fi

if [ "$built_path" != "$install_dir" ]; then
    echo "repo path has changed (was: ${built_path:-<none>}, now: $install_dir)"
    echo "cleaning protobuf source tree"
    git clean -fdx
    rm -rf "$install_dir"
    built_protobuf_version=""
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
    echo "$source_protobuf_version" > "$install_dir/done"
    echo "$install_dir" > "$path_marker"
    echo "finished building protobuf: $source_protobuf_version"
else
    echo "built protobuf matches source protobuf, skipping protobuf build"
    echo "built protobuf: $built_protobuf_version"
fi

popd
