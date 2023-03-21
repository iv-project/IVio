cmake_minimum_required(VERSION 3.14)
project(iviopy LANGUAGES CXX)

pybind11_add_module(iviopy MODULE fasta.cpp)
target_link_libraries(iviopy PRIVATE ivio::ivio)
