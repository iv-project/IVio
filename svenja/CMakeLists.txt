cmake_minimum_required (VERSION 3.15)

project (svenja CXX)

add_executable (seqan3_fasta seqan3_fasta.cpp)
target_link_libraries (seqan3_fasta io_interface seqan3::seqan3 seqan::seqan benchmark_main)

add_executable (bio_fasta bio_fasta.cpp)
target_link_libraries (bio_fasta io_interface bio::bio seqan3::seqan3 seqan::seqan benchmark_main)
