#!/usr/bin/bash
cd "$(dirname "$0")"

docker run -u $(id -u) -ti --rm -e PLAT="manylinux_2_28_x86_64" -v $(pwd)/..:/code quay.io/pypa/manylinux_2_28_x86_64 /code/python-bindings/build-wheels.sh
