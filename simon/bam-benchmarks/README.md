# Small demo for sam/bam benchmarks

1. compile software by runnig `./build.sh`
2. run benchmark by running
    - `time -v ./benchmark seqan2 somefile.bam`
    - `time -v ./benchmark seqan3 somefile.sam`

# Some results:
Using a file `sampled.bam` (433Mb)  and the same uncompressed `sampled.sam` (1.4GB).
Implementation can use 4 threads for BGZF compression
## sampled.bam
 - seqan2:  1.61  ( 8512kb maxres)
 - seqan3:  6.28  ( 9228kb maxres)

## sampled.sam
 - seqan2:  3.86  ( 4188kb maxres)
 - seqan3:  7.93  ( 4680kb maxres)
