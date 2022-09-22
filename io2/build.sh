#!/usr/bin/bash
cd "$(dirname "$0")"

#FLAGS="-march=native -O3 -DNDEBUG"
FLAGS="-O0 -ggdb"

g++ -std=c++20 ${FLAGS} -lz -o io2 -fconcepts-diagnostics-depth=10 \
    -Wall -Wextra \
    io2_demo.cpp \
    -I include \
    -DSEQAN_HAS_ZLIB  -isystem../lib/seqan/include \
    -DSEQAN3_HAS_ZLIB -isystem../lib/seqan3/include -isystem../lib/submodules/sdsl-lite/include


if [ "$1" == "--header" ]; then
    for i in $(ls include/io2); do
        echo $i
        g++ -fsyntax-only -std=c++20 ${FLAGS} -lz -o io2 -fconcepts-diagnostics-depth=10 \
            include/io2/$i \
            -I include \
            -DSEQAN_HAS_ZLIB  -isystem../lib/seqan/include \
            -DSEQAN3_HAS_ZLIB -isystem../lib/seqan3/include -isystem../lib/submodules/sdsl-lite/include
    done
fi
