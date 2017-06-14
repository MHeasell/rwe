#!/bin/sh
find . -type f -name '*.cpp' -print0 | xargs -0 clang-format -i
find . -type f -name '*.h' -print0 | xargs -0 clang-format -i
