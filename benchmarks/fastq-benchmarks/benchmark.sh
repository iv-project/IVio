#!/usr/bin/bash
# -----------------------------------------------------------------------------------------------------
# Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
# Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
# This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
# shipped with this file.
# -----------------------------------------------------------------------------------------------------
cd "$(dirname "$0")"

methods=(seqan2 seqan3 bio ivio)
files=(../data/reads.fq ../data/reads.fq.gz)
source ../utils/benchmark.sh
