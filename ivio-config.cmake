# SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
# SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required (VERSION 3.12)
if (TARGET ivio::ivio)
    return()
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/ivio ${CMAKE_CURRENT_BINARY_DIR}/ivio)
