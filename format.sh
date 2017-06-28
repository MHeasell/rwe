#!/bin/sh
find src -type f -name '*.cpp' -print0 | xargs -0 clang-format -i
find src -type f -name '*.h' -print0 | xargs -0 clang-format -i
