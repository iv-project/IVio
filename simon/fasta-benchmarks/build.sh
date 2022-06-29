#!/usr/bin/bash
cd "$(dirname "$0")"

FLAGS="-march=native -O3 -DNDBEUG"
#FLAGS="-O0 -ggdb"

g++ -std=c++20 ${FLAGS} -lz -o benchmark \
    main.cpp seqan2.cpp seqan3.cpp bio.cpp \
    -DSEQAN_HAS_ZLIB  -I../../lib/seqan/include \
    -DBIO_HAS_ZLIB -I../../lib/b.i.o./include \
    -DSEQAN3_HAS_ZLIB -I../../lib/seqan3/include -I../../lib/seqan3/submodules/sdsl-lite/include
