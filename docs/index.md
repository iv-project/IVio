# Introduction

**IVio**, pronounced **for you** aka **io4**. 
A c++20 library for **bioinformatics**. Providing functions for reading and writing different file formats.

## Support

 - **fasta** (rw) + **gzip** (rw)
 - **fastq** (ro) + **gzip** (ro)
 - **vcf** (ro) / **bcf** (ro)
 - **sam** (ro) / **bam** (ro)

(description - rw: read/write, ro: read-only)


## Basic concepts

Each file type consist of different parts. Here we discuss it for the fasta format.

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

## Usage Python
We also offer a subset of functions for python. For
this use the package **iviopy**. See [Python API-Reference](python-bindings.html) for more information.
