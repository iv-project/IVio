# Small demo for fasta benchmarks

1. compile software by running `./build.sh`
2. run benchmark by running `./bench.sh testdata.fa` or `./bench.sh testdata.fa.gz`
  - timing of zero means nothing was done
  - this call is very file extension sensitive, must be ".fa" or ".gz"
  - you can add `--octave plot.png` to get additional plotting

This should output something like:
```
view 1.170000 4136
cont 1.230000 4140
mmap_view 1.390000 4188
mmap_view2 0.000000 4104
mmap_cont 0.000000 4124
seqan2 1.630000 4124
seqan3 2.780000 4672
best 1.290000 4192
```

The benchmark is running a ACGTN counter:
```
template <typename Reader>
void benchmark(Reader&& reader) {
    std::array<int, 5> ctChars{};
    for (auto && [id, seq] : reader) {
        for (auto c : seq) {
            ctChars[c] += 1;
        }
    }

    size_t a{};
    for (size_t i{0}; i<ctChars.size(); ++i) {
        std::cout << i << ": " << ctChars[i] << "\n";
        a += ctChars[i];
    }
    std::cout << "total: " << a << "\n";
}
```

# Implemented techniques
- *view*: a view onto the memory (support .fa and .gz)
- *cont*: a view onto contigous and converted to rank memory (support .fa and .gz)
- *mmap_view*: a view onto memory using mmap (support .fa and .gz)
- *mmap_view2*: a view onto memory using mmap and mmap for zlib (requires overcommit memory) (support .gz)
- *seqan2*: using seqan2 IO
- *seqan3*: using seqan3 IO
- *seqan223*: using seqan2 IO, but using a seqan3 alphabet
- *best*: chooses *mmap_view2* if .gz and available. Other wise chooses *mmap view*. (support .fa and .gz)
- *bio*: using b.i.o. IO

# Results
These results alsways take the best out of three runs. Since this was running on a hot sommer day on a laptop, the timings are quite unstable.
## HG 38
hg38.fa file with only A, C, G and Ts stored.  The sequences are long. The file is around 3GB large. (left .fa, right .fa.gz)
### hg38.fa
![hg38.fa](data/hg38.png)
## Illumina sampled
illumina.fa file with A, C, G, T and Ns. The sequeneces are short, the file is around 1GB large. (left. fa, right .fa.gz)
### illumina.fa
![illumina.fa](data/illumina.png)

## As text
```
           illu.fa     illu.fa.gz  hg38.fa     hg38.fa.gz
view       1.12        3.92        5.40        11.41
cont       1.20        3.96        6.07        12.38
mmap_view  1.31        3.93        2.78        11.11
mmap_view2 -           3.70        -           10.74
mmap_cont  -           4.02        -           12.14
seqan2     1.60        3.84        6.17        12.52
seqan3     2.65        4.31        6.20        12.42
io2        1.55        3.64        5.45        11.76
io2-copy   1.98        3.99        7.68        13.94
bio        1.83        3.98        7.97        14.37
best       1.30        3.69        2.78        10.75
```
