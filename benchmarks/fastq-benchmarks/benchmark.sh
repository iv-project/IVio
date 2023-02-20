#!/usr/bin/bash
cd "$(dirname "$0")"

methods=(seqan2 seqan3 bio ivio)
files=(../data/reads.fq ../data/reads.fq.gz)
source ../utils/benchmark.sh
