# Small demo for fasta benchmarks

1. compile software by running `./build.sh`
2. run benchmark by running `./bench.sh testdata.fa` or `./bench.sh testdata.fa.gz`
  - timing of zero means nothing was done
  - this call is very file extension sensitive, must be ".fa" or ".gz"
  - you can add `--octave plot.png` to get additional plotting

This should output something like:
```
iew 1.170000 4136
cont 1.230000 4140
mmap_view 1.390000 4188
mmap_view2 0.000000 4104
mmap_cont 0.000000 4124
seqan2 1.630000 4124
seqan3 2.780000 4672
best 1.290000 4192

```
