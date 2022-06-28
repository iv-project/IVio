#!/usr/bin/bash
cd "$(dirname "$0")"

files=(illumina.fa illumina.fa.gz hg38.fa hg38.fa.gz)
methods=(view cont mmap_view mmap_view2 mmap_cont seqan2 seqan3 best)

for file in ${files[@]}; do
    #echo "file: $file"
    for method in ${methods[@]}; do
        #echo "  method: $method"
        printf "%s %s" $file $method
        for i in $(seq 3); do
            /usr/bin/time -f "run %e %M" ./benchmark ${method} ${file} 2>&1 | grep run
        done | cut -b 4- | sort -n | head -n 1
    done
done
