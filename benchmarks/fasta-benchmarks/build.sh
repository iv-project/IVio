#!/usr/bin/bash
cd "$(dirname "$0")"

source ../utils/build.sh

build src/read benchmark_read
build src/read_stream benchmark_read_stream
build src/write benchmark_write
