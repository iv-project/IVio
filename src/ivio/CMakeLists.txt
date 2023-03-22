cmake_minimum_required (VERSION 3.14)

project(ivio)

add_library(ivio bam/reader.cpp
                 bcf/reader.cpp
                 bcf/writer.cpp
                 fasta/reader.cpp
                 fasta/reader_mt.cpp
                 fasta/writer.cpp
                 fastq/reader.cpp
                 sam/reader.cpp
                 sam/writer.cpp
                 vcf/reader.cpp
                 vcf/writer.cpp)

add_library(ivio::ivio ALIAS ivio)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(${PROJECT_NAME}
#//!TODO must have a better way to link against z library
    z
)
