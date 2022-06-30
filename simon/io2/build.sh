#!/usr/bin/bash
cd "$(dirname "$0")"

FLAGS="-march=native -O3 -DNDBEUG"
#FLAGS="-O0 -ggdb"

g++ -std=c++20 ${FLAGS} -lz -o io2 \
    io2_demo.cpp \
    -I include \
    -DSEQAN_HAS_ZLIB  -I../../lib/seqan/include \
    -DSEQAN3_HAS_ZLIB -I../../lib/seqan3/include -I../../lib/seqan3/submodules/sdsl-lite/include
