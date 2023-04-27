#!/usr/bin/bash
# -----------------------------------------------------------------------------------------------------
# Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
# Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
# This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
# shipped with this file.
# -----------------------------------------------------------------------------------------------------
cd "$(dirname "$0")"

methods=(seqan2 seqan3 io2 bio seqtk ivio ivio_mt direct extreme)
files=(../data/illumina.fa ../data/illumina.fa.gz ../data/hg38.fa ../data/hg38.fa.gz)

if [ "${type}" == "write" ]; then
    files=(../data/illumina.fa ../data/hg38.fa)
fi

source ../utils/benchmark.sh
