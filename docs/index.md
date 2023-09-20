# Introduction

**IVio**, pronounced **for you** aka **io4**.
A c++20 library for **bioinformatics**. Providing functions for reading and writing different file formats.

## Support

A list of currently supported file types:

|                   | read support | write support |
|-------------------|--------------|---------------|
| **fasta**         |  ✔           | ✔             |
| **fasta**+**gzip**|  ✔           | ✔             |
| **indexed fasta** |  ✔           | ✘             |
| **fastq**         |  ✔           | ✘             |
| **fastq**+**gzip**|  ✔           | ✘             |
| **vcf**           |  ✔           | ✔             |
| **bcf**           |  ✔           | ✘             |
| **sam**           |  ✔           | ✔             |
| **bam**           |  ✔           | ✘             |


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
target_link_libraries(your_project ivio::ivio)
```

You can now happily use IVio


## Example Code
Reading a fasta file:
```c++
{% include-markdown "snippets/simple_example.cpp" %}
```
