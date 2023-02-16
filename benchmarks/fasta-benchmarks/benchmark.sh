#!/usr/bin/bash
cd "$(dirname "$0")"

type=$1

methods=(seqan2 seqan3 io2 bio ivio ivio_mt direct extreme)
files=(../data/illumina.fa ../data/illumina.fa.gz ../data/hg38.fa ../data/hg38.fa.gz)
if [ "${type}" == "write" ]; then
    files=(../data/illumina.fa ../data/hg38.fa)
fi

for file in ${files[@]}; do
    usedMeth=()
    speed=()
    memory=()
    echo "file: $file"
    echo -e "method  \tcorrect \ttotal(MB)\tspeed(MB/s)\tmemory(MB)";
    for method in ${methods[@]}; do
        line="$(./benchmark_${type} ${method} ${file} | tail -n +2)"
        echo "$line"
        c=$(echo $line | awk '{print $2}')
        s=$(echo $line | awk '{print $4}')
        m=$(echo $line | awk '{print $5}')
        if [ "$c" == "true" ]; then
            usedMeth+=($method)
            speed+=($s)
            memory+=($m)
        fi
    done
    if [ "$1" == "--octave" ]; then
        mkdir -p output
        plotF="output/$(basename ${file})_timing.png"
        (
            echo "data=[${speed[@]}];"
            printf "bar(data);\n"
            for i in $(seq ${#usedMeth[@]}); do
                j=$(expr $i - 1)
                m="$(echo "${usedMeth[${j}]}" | tr '_' ' ')"
                printf 'text(%s, -0.025, "  %s", "rotation", -90);\n' $i "${m}"
            done
            printf "xticklabels ([]);\n"
            printf 'ylabel("MB/s")\n'
            printf "print('$plotF')\n"
        ) | octave /dev/stdin --server --silent
        echo "printed plot to $plotF"
        plotF="output/$(basename ${file})_memory.png"
        (
            echo "data=[${memory[@]}];"
            printf "bar(data);\n"
            for i in $(seq ${#usedMeth[@]}); do
                j=$(expr $i - 1)
                m="$(echo "${usedMeth[${j}]}" | tr '_' ' ')"
                printf 'text(%s, -0.025, "  %s", "rotation", -90);\n' $i "${m}"
            done
            printf "xticklabels ([]);\n"
            printf 'ylabel("MB")\n'
            printf "print('$plotF')\n"
#        )
        ) | octave /dev/stdin --server --silent
        echo "printed plot to $plotF"
    fi

done
