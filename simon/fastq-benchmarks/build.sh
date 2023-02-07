#!/usr/bin/bash
cd "$(dirname "$0")"

FLAGS="-march=native -O3 -DNDEBUG -s -Wall"
#FLAGS="-O0 -ggdb -Wall"
#FLAGS="-O0 -ggdb -fsanitize=address -Wall"
#FLAGS="-march=native -O3 -DNDEBUG -ggdb"
#
INCLUDES="-I ../../io2/include \
    -I ../ \
    -DSEQAN_HAS_ZLIB  -isystem../../lib/seqan/include \
    -DBIO_HAS_ZLIB -isystem../../lib/b.i.o./include \
    -DSEQAN3_HAS_ZLIB -isystem../../lib/seqan3/include -isystem../../lib/submodules/sdsl-lite/include"
ARGS="-std=c++20 ${FLAGS} ${INCLUDES}"


mkdir -p obj/io3/fastq obj/io3/bcf obj/io3/vcf obj/src
g++ ${ARGS} -c ../io3/fastq/reader.cpp -o obj/io3/fastq/reader.o
g++ ${ARGS} -c src/main.cpp -o obj/src/main.o
g++ ${ARGS} -c src/seqan2.cpp -o obj/src/seqan2.o
g++ ${ARGS} -c src/seqan3.cpp -o obj/src/seqan3.o
g++ ${ARGS} -c src/bio.cpp -o obj/src/bio.o
g++ ${ARGS} -c src/io3.cpp -o obj/src/io3.o

g++ $(find obj | grep \.o\$) -lz-ng -lz ${FLAGS} -o benchmark
