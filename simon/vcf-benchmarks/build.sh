#!/usr/bin/bash
cd "$(dirname "$0")"

FLAGS="-march=native -O3 -DNDEBUG -s"
#FLAGS="-O0 -ggdb -fsanitize=address"
#FLAGS="-O0 -ggdb"
#FLAGS="-march=native -O3 -DNDEBUG -ggdb"

INCLUDES="-I ../../io2/include \
    -I ../ \
    -DSEQAN_HAS_ZLIB  -isystem../../lib/seqan/include \
    -DBIO_HAS_ZLIB -isystem../../lib/b.i.o./include \
    -DSEQAN3_HAS_ZLIB -isystem../../lib/seqan3/include -isystem../../lib/submodules/sdsl-lite/include"
ARGS="-std=c++20 ${FLAGS} ${INCLUDES}"


mkdir -p obj/io3/fasta obj/io3/bcf obj/io3/vcf
g++ ${ARGS} -c ../io3/fasta/reader.cpp -o obj/io3/fasta/reader.o
g++ ${ARGS} -c ../io3/fasta/writer.cpp -o obj/io3/fasta/writer.o
g++ ${ARGS} -c ../io3/vcf/reader.cpp -o obj/io3/vcf/reader.o
g++ ${ARGS} -c ../io3/vcf/writer.cpp -o obj/io3/vcf/writer.o
g++ ${ARGS} -c ../io3/bcf/writer.cpp -o obj/io3/bcf/writer.o
g++ ${ARGS} -c main.cpp -o obj/main.o
g++ ${ARGS} -c seqan2.cpp -o obj/seqan2.o
g++ ${ARGS} -c bio.cpp -o obj/bio.o

#g++ $(find obj | grep \.o\$) -lasan -lz-ng -lz  -o benchmark
g++ $(find obj | grep \.o\$) -lz-ng -lz  -o benchmark -s
