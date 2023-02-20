#!/usr/bin/bash
cd "$(dirname "$0")"

methods=(seqan2 bio ivio)
files=(../data/sampled.vcf)

source ../utils/benchmark.sh
