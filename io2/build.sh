#!/usr/bin/bash
cd "$(dirname "$0")"

FLAGS="-march=native -O3 -DNDEBUG"
#FLAGS="-O0 -ggdb"

g++ -std=c++20 ${FLAGS} -lz -o io2 -fconcepts-diagnostics-depth=10 \
    io2_demo.cpp \
    -I include \
    -DSEQAN_HAS_ZLIB  -isystem../lib/seqan/include \
    -DSEQAN3_HAS_ZLIB -isystem../lib/seqan3/include -isystem../lib/submodules/sdsl-lite/include
