#!/usr/bin/bash
cd "$(dirname "$0")"

methods=(direct view cont mmap_view mmap_view2 mmap_cont seqan2 seqan3 io2 io2-copy bio best)

file=$1
timing=()
memory=()
for method in ${methods[@]}; do
    l="$(for i in $(seq 3); do
        /usr/bin/time -f "run %e %M" ./benchmark ${method} ${file} 2>&1 | grep run
    done | cut -b 5- | sort -n | head -n 1)"
    t=$(echo $l | cut -d " " -f 1)
    m=$(echo $l | cut -d " " -f 2)
    printf "%s %f %d\n" $method $t $m
    timing+=($t)
    memory+=($m)
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
