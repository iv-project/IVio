#!/usr/bin/bash
cd "$(dirname "$0")"

methods=(bio ivio)
files=(../data/sampled.bcf)

source ../utils/benchmark.sh
