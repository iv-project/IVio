# FASTA

Support reading and writing [fasta files](https://blast.ncbi.nlm.nih.gov/Blast.cgi?CMD=Web&PAGE_TYPE=BlastDocs&DOC_TYPE=BlastHelp).

## Record and Record-View
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

## Reading
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

## Writing
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

### Example 1
In this example a file is being opened and print to command line
```c++
#include <ivio/fasta/reader.h>

int main(int argc, char** argv) {
    auto inputFile = std::filesystem::path{argv[1]};
    auto reader = ivio::fasta::reader{{.input = file,
                                       .compressed = false, // false by default, if true a gzip file is expected
    }};
    for (auto record_view : reader) {
        std::cout << "id: " << record_view.id << "\n";
        std::cout << "seq: " << record_view.seq << "\n";
    }
}
```
### Example 2
In this example the data is read from the standard input and writen to standard output
```c++
#include <iostream>
#include <ivio/fasta/reader.h>

int main(int, char**) {
    auto reader = ivio::fasta::reader{{.input = std::cin}};
    auto writer = ivio::fasta::writer{{.input = std::cout}};

    for (auto record_view : reader) {
        writer.write(record_view);
    }
}
```

### Example 3
Load complete fasta file into memory:
```c++
#include <iostream>
#include <ivio/fasta/reader.h>

int main(int, char**) {
    auto reader = ivio::fasta::reader{{.input = std::cin,
                                       .compressed = false, // false by default, if true a gzip file is expected
    }};

    // creates a vector of type `std::vector<ivio::fasta::record>` even though reader returns `record_view`
    auto vec = std::vector{begin(reader), end(reader)};
}
```

