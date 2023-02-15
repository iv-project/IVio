#!/usr/bin/bash
cd "$(dirname "$0")"

set -e

FLAGS="-march=native -O3 -DNDEBUG -s -fconcepts-diagnostics-depth=10 -Wall"
#FLAGS="-O0 -ggdb -Wall"
#FLAGS="-O0 -ggdb -fsanitize=address -Wall"
#FLAGS="-march=native -O3 -DNDEBUG -ggdb"

INCLUDES="-I ../../src/ \
    -isystem../../lib/io2/include \
    -DSEQAN_HAS_ZLIB  -isystem../../lib/seqan/include \
    -DBIO_HAS_ZLIB -isystem../../lib/b.i.o./include \
    -DSEQAN3_HAS_ZLIB -isystem../../lib/seqan3/include -isystem../../lib/submodules/sdsl-lite/include"
ARGS="-std=c++20 ${FLAGS} ${INCLUDES}"

# Building io3 library
for f in $(find ../../src/io3 | grep .cpp\$); do
    g=$(echo $f | cut -b 11-)
    mkdir -p $(dirname obj/$g)
    g++ ${ARGS} -c $f -o obj/$g.o
done

# Building benchmark_read
for f in $(find src/read | grep .cpp\$); do
    mkdir -p $(dirname obj/$f)
    g++ ${ARGS} -c $f -o obj/$f.o
done
g++ $(find obj/src/read | grep \.o\$) $(find obj/io3 | grep \.o\$) -lz-ng -lz ${FLAGS} -o benchmark_read

# Building benchmark_read_stream
for f in $(find src/read_stream | grep .cpp\$); do
    mkdir -p $(dirname obj/$f)
    g++ ${ARGS} -c $f -o obj/$f.o
done
g++ $(find obj/src/read_stream | grep \.o\$) $(find obj/io3 | grep \.o\$) -lz-ng -lz ${FLAGS} -o benchmark_read_stream

# Building benchmark_write
for f in $(find src/write | grep .cpp\$); do
    mkdir -p $(dirname obj/$f)
    g++ ${ARGS} -c $f -o obj/$f.o
done
g++ $(find obj/src/write | grep \.o\$) $(find obj/io3 | grep \.o\$) -lz-ng -lz ${FLAGS} -o benchmark_write
