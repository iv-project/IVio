#!/usr/bin/bash
cd "$(dirname "$0")"

methods=(seqan2 seqan3 io2 ivio)
files=(../data/sampled.bam)

source ../utils/benchmark.sh "$@"
