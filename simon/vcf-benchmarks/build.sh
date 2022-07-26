#!/usr/bin/bash
cd "$(dirname "$0")"

FLAGS="-march=native -O3 -DNDEBUG"
#FLAGS="-O0 -ggdb -fsanitize=address"
#FLAGS="-O0 -ggdb"
#FLAGS="-march=native -O3 -DNDEBUG -ggdb"

g++ -std=c++20 ${FLAGS} -lz -o benchmark \
    main.cpp seqan2.cpp bio.cpp \
    -I ../../io2/include \
    -I .. \
    -DSEQAN_HAS_ZLIB  -isystem../../lib/seqan/include \
    -DBIO_HAS_ZLIB -isystem../../lib/b.i.o./include \
    -DSEQAN3_HAS_ZLIB -isystem../../lib/seqan3/include -isystem../../lib/submodules/sdsl-lite/include

