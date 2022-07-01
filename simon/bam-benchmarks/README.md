# Small demo for sam/bam benchmarks

1. compile software by runnig `./build.sh`
2. run benchmark by running
    - `time -v ./benchmark seqan2 somefile.bam`
    - `time -v ./benchmark seqan3 somefile.bam`
    - `time -v ./benchmark io2 somefile.bam`

# Some results:
Using a file `sampled.bam` (433Mb)  and the same uncompressed `sampled.sam` (1.4GB).
Implementation can use 4 threads for BGZF compression
## sampled.bam
 - seqan2:  0.88  ( 8540kb maxres)
 - seqan3:  6.38  ( 9228kb maxres)
 - io2:     0.89  ( 8524kb maxres)

## sampled.sam
 - seqan2:  3.44  ( 4188kb maxres)
 - seqan3:  7.24  ( 4680kb maxres)
 - io2:     3.54  ( 4216kb maxres)
