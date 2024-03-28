# SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
# SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
# SPDX-License-Identifier: BSD-3-Clause

# This file is called in the following scenarios
# 1. scenario: this file is at the root of a `cmake ..` call.
#       In this case we assume this project is compiled for packaging or testing.
# 2. scenario: another project is adding this directory via "add_subdirectory".
#       In this case we don't want any tests to be included.
cmake_minimum_required (VERSION 3.25)

# !TODO this can be removed if cmake is changed to version 3.29
if (NOT DEFINED PROJECT_NAME)
    set(PROJECT_IS_TOP_LEVEL TRUE)
else()
    set(PROJECT_IS_TOP_LEVEL FALSE)
endif()

project(ivio LANGUAGES CXX VERSION "${IVIO_PROJECT_VERSION}"
            DESCRIPTION "ivio -- IO library for file formats used in bioinformatics")

if (PROJECT_IS_TOP_LEVEL)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()


include(cmake/CPM.cmake)
include(ivio-config.cmake)

if (PROJECT_IS_TOP_LEVEL)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(ivio PUBLIC /bigobj /EHsc -DNOMINMAX)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
        target_compile_options(ivio PUBLIC -Wall -Werror -Wpedantic -Wextra -fPIC -fp-model=strict)
    else()
        target_compile_options(ivio PUBLIC -Wall -Werror -Wpedantic -Wextra -fPIC)
    endif()

    CPMaddPackage("gh:fmtlib/fmt#10.1.1")
    CPMaddPackage("gh:catchorg/Catch2#v3.4.0")

    enable_testing()
    add_subdirectory(docs/snippets)
    add_subdirectory(src/test_ivio)
    add_subdirectory(src/test_header)
endif()
