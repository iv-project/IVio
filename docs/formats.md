# Formats

## FASTA

Support reading and writing [fasta files](https://blast.ncbi.nlm.nih.gov/Blast.cgi?CMD=Web&PAGE_TYPE=BlastDocs&DOC_TYPE=BlastHelp).

### Record and Record-View
IVio provides the data structs `ivio::fasta::record` and `ivio::fasta::record_view`.
A `record_view` has following layout:
``` c++
struct record_view {
    std::string_view id;
    std::string_view seq;
};
```
The type `record` looks like this:
```c++
struct record {
    std::string id;
    std::string seq;
};
```

### Reading
The `ivio::fasta::reader` is for-range compatible class. It fulfils c++ concepts of  [range](https://en.cppreference.com/w/cpp/ranges/range) and [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator).
When looping over a reader it returns `record_view` that are only valid until next element are being requested from the reader.
To get persistent data, it is required to create a `record`.

As an input it takes an `ivio::fasta::reader::config` struct which declares the input file and if gzip compression is wanted.
```c++
struct ivio::fasta::reader::config {
    // Source: file or stream
    std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;

    // This is only relevant if a stream is being used
    bool compressed{};
};
```

### Writing
The `ivio::fasta::writer` provides a single function `write` which takes a `ivio::fasta::record_view` as input.
The class is initialized with a `ivio::fasta::writer::config` object which has the options:
```
struct ivio::fasta::writer::config {
    // Source: file or stream
    std::variant<std::filesystem::path, std::reference_wrapper<std::ostream>> output;

    // This is only relevant if a stream is being used
    bool compressed{};

    size_t length{80}; // Break after 80 characters
};
```
### Examples
**Example 1**
In this example a file is being opened and print to command line
```c++
{% include-markdown "snippets/fasta_example_01.cpp" %}
```
**Output**
```sh
{% include-markdown "snippets/fasta_example_01.cpp.out" %}
```
**Example 2**
In this example the data is read from the standard input and writen to standard output
```c++
{% include-markdown "snippets/fasta_example_02.cpp" %}
```

**Example 3**
Load complete fasta file into memory:
```c++
{% include-markdown "snippets/fasta_example_03.cpp" %}
```


## VCF
VCF provides 5 classes:

 - `header`
 - `reader`, `writer`
 - `record` and `record_view`


### Example
```cpp
{% include-markdown "snippets/read_vcf.cpp" %}
```
### Output
```sh
{% include-markdown "snippets/read_vcf.cpp.out" %}
```
