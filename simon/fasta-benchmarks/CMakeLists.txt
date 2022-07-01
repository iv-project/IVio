cmake_minimum_required (VERSION 3.15)

project (simon_fasta_benchmark CXX)

add_executable (${PROJECT_NAME}
    main.cpp
    io2.cpp
    bio.cpp
    seqan2.cpp
    seqan3.cpp
)

target_link_libraries (${PROJECT_NAME}
    io_interface
    seqan::seqan
    seqan3::seqan3
    io2::io2
    bio::bio
    benchmark_main)
