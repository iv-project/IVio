#!/usr/bin/bash
cd "$(dirname "$0")"

FLAGS="-march=native -O3 -DNDEBUG -s -Wall"
#FLAGS="-O0 -ggdb -Wall"
#FLAGS="-O0 -ggdb -fsanitize=address -Wall"
#FLAGS="-march=native -O3 -DNDEBUG -ggdb"

INCLUDES="-I . -I ../../src/ \
    -isystem../../lib/io2/include \
    -DSEQAN_HAS_ZLIB  -isystem../../lib/seqan/include \
    -DBIO_HAS_ZLIB -isystem../../lib/biocpp-io/include \
                   -isystem../../lib/biocpp-core/include \
    -DSEQAN3_HAS_ZLIB -isystem../../lib/seqan3/include -isystem../../lib/submodules/sdsl-lite/include"
ARGS="-std=c++20 ${FLAGS} ${INCLUDES}"


# Building ivio library
for f in $(find ../../src/ivio | grep .cpp\$); do
    g=$(echo $f | cut -b 11-)
    mkdir -p $(dirname obj/$g)
    g++ ${ARGS} -c $f -o obj/$g.o
done

# Building benchmark
for f in $(find src/read | grep .cpp\$); do
    mkdir -p $(dirname obj/$f)
    g++ ${ARGS} -c $f -o obj/$f.o
done
g++ $(find obj/src/read | grep \\\.o\$) $(find obj/ivio | grep \\\.o\$) -lz-ng -lz ${FLAGS} -o benchmark_read
