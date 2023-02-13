#!/usr/bin/bash
cd "$(dirname "$0")"

FLAGS="-march=native -O3 -DNDEBUG -s -Wall"
#FLAGS="-O0 -ggdb -fsanitize=address -Wall"
#FLAGS="-march=native -O3 -DNDEBUG -ggdb"
#
INCLUDES="-I ../../io2/include \
    -I ../ \
    -DSEQAN_HAS_ZLIB  -isystem../../lib/seqan/include \
    -DBIO_HAS_ZLIB -isystem../../lib/b.i.o./include \
    -DSEQAN3_HAS_ZLIB -isystem../../lib/seqan3/include -isystem../../lib/submodules/sdsl-lite/include"
ARGS="-std=c++20 ${FLAGS} ${INCLUDES}"


mkdir -p obj/io3/fasta obj/io3/bcf obj/io3/vcf
g++ ${ARGS} -c ../io3/fasta/reader.cpp -o obj/io3/fasta/reader.o
g++ ${ARGS} -c ../io3/fasta/reader_mt.cpp -o obj/io3/fasta/reader_mt.o
g++ ${ARGS} -c ../io3/fasta/writer.cpp -o obj/io3/fasta/writer.o

for f in $(find src/read | grep .cpp\$); do
    mkdir -p $(dirname obj/$f)
    g++ ${ARGS} -c $f -o obj/$f.o
done
g++ $(find obj/src/read | grep \.o\$) $(find obj/io3 | grep \.o\$) -lz-ng -lz ${FLAGS} -o benchmark_read

for f in $(find src/read_stream | grep .cpp\$); do
    mkdir -p $(dirname obj/$f)
    g++ ${ARGS} -c $f -o obj/$f.o
done
g++ $(find obj/src/read_stream | grep \.o\$) $(find obj/io3 | grep \.o\$) -lz-ng -lz ${FLAGS} -o benchmark_read_stream
