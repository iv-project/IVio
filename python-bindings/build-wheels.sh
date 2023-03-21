#!/usr/bin/bash
set -e -u -x

cd /code/python-bindings

rm -rf wheelhouse
# Compile wheels
for PYBIN in /opt/python/*/bin; do
    if [ "/opt/python/cp36-cp36m/bin" == "${PYBIN}" ]; then
        continue
    fi
    "${PYBIN}/pip" wheel . --no-deps -w wheelhouse/tmp
done

# Bundle external shared libraries into the wheels
for wheel in wheelhouse/tmp/*.whl; do
    if ! auditwheel show "$wheel"; then
        echo "Skipping non-platform wheel $wheel"
    else
        auditwheel repair "$wheel" --plat "$PLAT" -w wheelhouse
    fi
done
