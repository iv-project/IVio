cmake_minimum_required (VERSION 3.15)

project (enrico CXX)

include (seqan_require_benchmark)
seqan_require_benchmark ()

add_executable (fasta_benchmark fasta_benchmark.cpp)
target_link_libraries (fasta_benchmark io_interface seqan3::seqan3 seqan::seqan benchmark_main)
