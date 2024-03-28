# SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
# SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
# SPDX-License-Identifier: BSD-3-Clause

# This file is called in the following scenarios
# 1. scenario: this file is at the root of a `cmake ..` call.
#       In this case we assume this project is compiled for packaging or testing.
# 2. scenario: another project is adding this directory via "add_subdirectory".
#       In this case we don't want any tests to be included.
cmake_minimum_required (VERSION 3.29)

project(ivio LANGUAGES CXX VERSION "${IVIO_PROJECT_VERSION}"
            DESCRIPTION "ivio -- IO library for file formats used in bioinformatics")



if (PROJECT_IS_TOP_LEVEL)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /EHsc -DNOMINMAX")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wpedantic -Wextra -fPIC -fp-model=strict")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wpedantic -Wextra -fPIC")
    endif()
endif()

include(cmake/CPM.cmake)
include(ivio-config.cmake)

if (${ROOT_PROJECT})

    CPMaddPackage("gh:fmtlib/fmt#10.1.1")
    CPMaddPackage("gh:catchorg/Catch2#v3.4.0")

    enable_testing()
    add_subdirectory(docs/snippets)
    add_subdirectory(src/test_ivio)
    add_subdirectory(src/test_header)
endif()
