#!/usr/bin/bash
cd "$(dirname "$0")"

methods=(seqan2 seqan3 io3 io2 io2-copy bio)
#methods=(io3)
#methods=(direct io3_file io3_mmap io3_stream seqan2 seqan3 io2 io2-copy bio io3-auto-select)
#methods=(io3_file io3_mmap io3_stream io3_ng_file io3_ng_mmap io3_ng_stream io3)


file=$1
timing=()
memory=()
for method in ${methods[@]}; do
    l="$(for i in $(seq 3); do
        /usr/bin/time -f "run %e %M" ./benchmark ${method} ${file} 2>&1 | grep "^run "
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
