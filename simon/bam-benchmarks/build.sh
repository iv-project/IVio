#!/usr/bin/bash
cd "$(dirname "$0")"

g++ -std=c++20 -march=native -O3 -DNDEBUG -lz -o benchmark \
    main.cpp seqan3.cpp seqan2.cpp io2.cpp \
    -I../io2/include \
    -DSEQAN_HAS_ZLIB  -I../../lib/seqan/include \
    -DSEQAN_BGZF_NUM_THREADS=4 \
    -DSEQAN3_HAS_ZLIB -I../../lib/seqan3/include -I../../lib/seqan3/submodules/sdsl-lite/include

#g++ -std=c++20 file.cpp seqan2.cpp -DSEQAN_HAS_ZLIB -lz -O0 -ggdb
