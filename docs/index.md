# Introduction

A c++20 library for **bioinformatics**. It provides functions to load and save typical
file formats.

## Basic concepts

Each file type consist of four parts. Here we discuss it for the fasta format.

 - `fasta::reader`
 - `fasta::writer`
 - `fasta::record`
 - `fasta::record_view`

## Integration CMake
Easiest way to use this repository is to clone this as a subrepo into your project, for example to
`lib/IVio`, and then edit your `CMakeLists.txt`:

```cmake
add_subdirectory(lib/IVio)

# ...
target_link_library(your_project ivio::ivio)
```

You can now happily use IVio


**Code**
```c++
{% include-markdown "snippets/simple_example.cpp" %}
```
