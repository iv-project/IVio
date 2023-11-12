<!--
    SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
    SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
    SPDX-License-Identifier: CC-BY-4.0
-->
# FASTQ

Support reading and writing [fastq files 🔗](https://en.wikipedia.org/wiki/FASTQ_format).
All classes/functions are available in the `ivio::fastq` namespace.

[TOC]

## Record and RecordView (`record`, `record_view`) { #markdown data-toc-label="Record and RecordView" }
IVio provides the datastructures `ivio::fastq::record` and `ivio::fastq::record_view`. These are the typical input/output arguments of the readers and writers.
These datastructures convertible to each other.

The `record_view` layout
``` c++
struct record_view {
    std::string_view id;
    std::string_view seq;
    std::string_view id2;
    std::string_view qual;
};
```
The `record` layout
```c++
struct record {
    std::string id;
    std::string seq;
    std::string id2;
    std::string qual;
};
```

## Reader (`reader`, `reader::config`) { #markdown data-toc-label="Reader" }
The `ivio::fastq::reader` accepts a `ivio::fastq::reader::config` object for initialization. It initializes an object
that enables you to iterate over the records of a file. It is is for-range compatible, meaning it fulfills c++ concepts of  [range](https://en.cppreference.com/w/cpp/ranges/range) and [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator).
When looping over a reader it returns `record_view` objects that are only valid until next record is being requested from the reader.
To get ownership of the data, it is required to create an object of type `record`.

To configure the `reader` one must fill the `ivio::fastq::reader::config` struct which declares the input file and if gzip compression is expected.
```c++
struct ivio::fastq::reader::config {
    // Source: file or stream
    std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;

    // This is only relevant if a stream is being used
    bool compressed{};
};
```

Overview of the member functions of `ivio::fastq::reader`
```c++
struct ivio::fastq::reader {
    /*...*/
    auto next() -> std::optional<record_view>; // reads the next record
    void close() const; // closes the read file
};
```

## Examples
### Example - Reading record by record
In this example a file is being opened and print to command line
```c++
{% include-markdown "snippets/fastq_example_01.cpp" start="CC0-1.0" %}
```
**Output**
```sh
{% include-markdown "snippets/fastq_example_01.cpp.out" %}
```
