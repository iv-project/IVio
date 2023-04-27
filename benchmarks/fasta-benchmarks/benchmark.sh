#!/usr/bin/bash
cd "$(dirname "$0")"

methods=(seqan2 seqan3 io2 bio seqtk ivio ivio_mt direct extreme)
files=(../data/illumina.fa ../data/illumina.fa.gz ../data/hg38.fa ../data/hg38.fa.gz)

if [ "${type}" == "write" ]; then
    files=(../data/illumina.fa ../data/hg38.fa)
fi

source ../utils/benchmark.sh
