#!/bin/sh

script_dir="$(readlink -f "$(dirname "$0")")"

reformat() {
    expand -t 4 "$1" | clang-format > "$1.__reformat-tmp"
    mv "$1.__reformat-tmp" "$1"
}

find src -type f -name '*.cpp' -print0 |
while read -r -d '' file; do
    reformat "$file"
done
find src -type f -name '*.h' -print0 |
while read -r -d '' file; do
    reformat "$file"
done

# Note: perl -i on cygwin always creates bak files.
perl -i fix-cmakelists.pl CMakeLists.txt && rm -f CMakeLists.txt.bak
