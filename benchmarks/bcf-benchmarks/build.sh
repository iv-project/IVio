#!/usr/bin/bash
cd "$(dirname "$0")"

source ../utils/build.sh

build src/read benchmark_read
build src/copy benchmark_copy
