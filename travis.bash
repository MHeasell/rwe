#!/bin/bash

set -euo pipefail

cd launcher
echo "nodejs version: $(node --version)"
echo "npm version: $(npm --version)"
npm ci
npm run tsc
npm test
npm run lint
npm run package

cd ../libs
./build-protobuf.sh
cd ..

mkdir build
cd build
cmake --version
cmake ..
make -j 2

./rwe_test
