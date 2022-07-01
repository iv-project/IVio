#!/usr/bin/bash
cd "$(dirname "$0")"

FLAGS="-march=native -O3 -DNDEBUG"
#FLAGS="-O0 -ggdb"

g++ -std=c++20 ${FLAGS} -lz -o benchmark \
    main.cpp seqan3.cpp seqan2.cpp io2.cpp \
    -I../io2/include \
    -DSEQAN_HAS_ZLIB  -I../../lib/seqan/include \
    -DSEQAN_BGZF_NUM_THREADS=4 \
    -DSEQAN3_HAS_ZLIB -I../../lib/seqan3/include -I../../lib/submodules/sdsl-lite/include
