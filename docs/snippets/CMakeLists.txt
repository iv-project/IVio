# SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
# SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required (VERSION 3.14)

function(test_snippet file_base_name params)
    if (EMSCRIPTEN)
        return()
    endif()
    add_executable(snippet_${file_base_name} ${file_base_name})
    target_link_libraries(snippet_${file_base_name} ivio::ivio fmt::fmt)

    add_test(NAME test_snippet_${file_base_name} COMMAND bash -c "$<TARGET_FILE:snippet_${file_base_name}> ${params} > ${file_base_name}.run.out")
    add_test(test_snippet_${file_base_name}_compare ${CMAKE_COMMAND} -E compare_files --ignore-eol ${file_base_name}.run.out ${CMAKE_CURRENT_SOURCE_DIR}/${file_base_name}.out)
    set_tests_properties(test_snippet_${file_base_name}_compare PROPERTIES DEPENDS test_snippet_${file_base_name})
endfunction()

test_snippet("read_vcf.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/data/vcf.vcf")
test_snippet("fasta_example_01.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/data/demo.fa")
test_snippet("fastq_example_01.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/data/demo.fq")
