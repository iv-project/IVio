#!/usr/bin/bash
cd "$(dirname "$0")"

methods=(ivio)
files=(../data/illumina.fa ../data/hg38.fa)

source ../utils/benchmark.sh
