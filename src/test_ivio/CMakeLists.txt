# SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
# SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required (VERSION 3.8)

project(test_ivio)

# fmindex-collectionunittests
add_executable(${PROJECT_NAME}
    fasta_reader.cpp
    fasta_writer.cpp
    fastq_reader.cpp
    sam_reader.cpp
    sam_writer.cpp
    vcf_reader.cpp
    vcf_writer.cpp
)
target_link_libraries(${PROJECT_NAME}
    Catch2::Catch2WithMain
    ivio::ivio
)


add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
