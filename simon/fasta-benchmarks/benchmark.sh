#!/usr/bin/bash
cd "$(dirname "$0")"

methods=(seqan2 seqan3 io3 io3_mt io2 io2-copy bio direct extreme)
files=(data/illumina.fa data/illumina.fa.gz data/hg38.fa data/hg38.fa.gz)

timing=()
memory=()
for file in ${files[@]}; do
    echo -e "method  \tcorrect \ttotal(MB)\tspeed(MB/s)\tmemory(MB)";
    for method in ${methods[@]}; do
        ./benchmark ${method} ${file} | tail -n +2
        timing+=($t)
        memory+=($m)
    done
done

if [ "$2" == "--octave" ]; then
    (
    echo "timing=[${timing[@]}];"
    echo "memory=[${memory[@]}];"
    printf "bar(timing);\n"
    for i in $(seq ${#methods[@]}); do
        j=$(expr $i - 1)
        m="$(echo "${methods[${j}]}" | tr '_' ' ')"
        printf 'text(%s, -0.025, "%s", "rotation", -90);\n' $i "${m}"
    done
    printf "xticklabels ([]);\n"
    printf "print('$3')\n"
    ) | octave /dev/stdin --server --silent
    echo "printed plot to $3"
fi
