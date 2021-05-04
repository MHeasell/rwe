#!/bin/sh
set -e

install_dir="${PWD}/_protobuf-install"

if [ ! -f "$install_dir/done" ]; then
    cd protobuf
    ./autogen.sh
    ./configure "--prefix=$install_dir"
    make -j`nproc`
    make install
    touch "$install_dir/done"
fi
