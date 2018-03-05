#!/bin/sh

script_dir="$(readlink -f "$(dirname "$0")")"

reformat() {
    expand -t 4 "$1" | clang-format > "$1.__reformat-tmp"
    mv "$1.__reformat-tmp" "$1"
}

cd "$script_dir/src"
find . -type f \( -name '*.cpp' -o -name '*.h' \) -print0 |
while read -r -d '' file; do
    "$script_dir/fix-includes.pl" "$file"
    reformat "$file"
done

# Note: perl -i on cygwin always creates bak files.
perl -i "$script_dir/fix-cmakelists.pl" "$script_dir/CMakeLists.txt" && rm -f "$script_dir/CMakeLists.txt.bak"
