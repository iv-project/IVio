# SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
# SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required (VERSION 3.14)

add_library(ivio bam/reader.cpp
                 bcf/reader.cpp
                 bcf/writer.cpp
                 fasta/reader.cpp
                 fasta/writer.cpp
                 faidx/reader.cpp
                 fastq/reader.cpp
                 sam/reader.cpp
                 sam/writer.cpp
                 vcf/reader.cpp
                 vcf/writer.cpp)
add_library(ivio::ivio ALIAS ivio)

target_include_directories(ivio
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(ivio
    zlibstatic
)
target_compile_features(ivio PUBLIC cxx_std_20)
