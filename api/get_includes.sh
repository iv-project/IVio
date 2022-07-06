#!/usr/bin/env bash

set -Eeu

# Assumes that the script is in a subdirectory of the root, e.g. api/get_includes.sh

SEQAN_INCLUDE_PATH=$(dirname "$(dirname "$0")")/lib/seqan/include
IO2_INCLUDE_PATH=$(dirname "$(dirname "$0")")/io2/include/io2
INCLUDES=$(grep -h -o -R -P "#include <\Kseqan/.*\.h" ${IO2_INCLUDE_PATH} | sort -u)
TMP_FILE="$(mktemp)"

# Fail: Remove temporary file and kill all sub-processes
trap "rm -f ${TMP_FILE}; trap - SIGTERM && kill -- -$$" INT TERM ERR SIGINT
# Normal exit: Remove temporary file
trap "rm -f ${TMP_FILE}" EXIT

if ! [[ -f ${TMP_FILE} ]]; then
    echo "Temp file ${TMP_FILE} does not exist. Exiting."
    exit 1
fi

if ! [[ -s ${SEQAN_INCLUDE_PATH} ]]; then
    echo "Include path ${SEQAN_INCLUDE_PATH} does not exist. Exiting."
    exit 1
fi

get_include ()
{
    # If file exists and is not yet in temporary file
    if [[ -s ${1} ]] && ! grep -q "${1}" "${TMP_FILE}"; then
        echo ${1} >> ${TMP_FILE}
    fi
    # If there are matches for #include <seqan/. grep with not output would return error code, hence the check.
    if grep -q -P "#include <\Kseqan/.*\.h" "${1}"; then
        # For each seqan include
        for include in $(grep -P -o "#include <\Kseqan/.*\.h" ${1}); do
            include=${SEQAN_INCLUDE_PATH}/${include}
            # If file exists and is not yet in temporary file
            if [[ -s ${include} ]] && ! grep -q "${include}" "${TMP_FILE}"; then
                echo ${include} >> ${TMP_FILE}
                get_include ${include} # Recursively get seqan includes
            fi
        done
    fi
}

for include in ${INCLUDES}; do
    get_include ${SEQAN_INCLUDE_PATH}/${include}
done

# ^^ all includes vv specific includes (either or)

# get_include ${SEQAN_INCLUDE_PATH}/seqan/seq_io.h
# get_include ${SEQAN_INCLUDE_PATH}/seqan/bam_io.h

# sort -u ${TMP_FILE} # Full file path
sort -u ${TMP_FILE} | sed -e "s@^$SEQAN_INCLUDE_PATH/@@" # The ... part of #include <...>, e.g. seqan/align.h
