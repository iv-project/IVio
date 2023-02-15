# IO design ideas:

## Motivation/Goals
The SeqAn3 library targets 2 audiences:
1. People who want to use SeqAn3 to write their own tool.
2. People who want to implement a new cool algorithm for research (bachelor, master, phd, papers).
n. It is not targeted at people who can get elaborated training session on how to use the software (like in enterprise systems for companies)

From this we can derive a few ideas:
- reduce the number of configuration points (best just having one)
- reduce the amount of configuration parameters (best having perfect default parameters and not making them configurable ;-))
- names for options, everything that is named is much easier to use and remember


Old Ideas for IO2 can be found [here](IO2.md)


## IO3
Additional Ideas of io3 vesus the io2 are:
 - as plain as possible, no alphabets at all
 - emphasise on string_view, guaranteeing every contigous memory.
 - giving up header only, for fast include/compilation


### Fasta - Input/Output:
sketch how code could look like:
```
/* Iterating over the reader gives us a io3::fasta::record_view
 * struct record_view {
 *     std::string_view id;
 *     std::string_view seq;
 * };
 */

auto in = io3::fasta::reader {
    .input = "myfile.fasta", // could also be a std::istream
};
for (auto const& record : in) {
    record.id;
    record,seq;
}
```
```
/* For writting we also need to pass a io3::fasta::record_view or anything that is convertible to one. For convinience a io3::fasta::record is provided:
 *  struct record {
 *     std::string id;
 *     std::string seq;
 *  };
 */
auto out = io3::fasta::writer {
    .output = "myfile.out.fasta", // could also be a std::ostream
};
out.write(record{.id = "someid", .seq = "ACGTACGTACGT"});
```
```
/* Reading a a file and replacing all 'A' with 'N's
 */
{
auto in = io3::fasta::reader {
    .input = "myfile.fasta", // could also be a std::istream
};
auto out = io3::fasta::writer {
    .output = "myfile.out.fasta", // could also be a std::ostream
};
for (auto const& view : in) {
    auto record = io3::fasta::record{view};
    for (auto& c: record.seq) {
        if (c == 'A') c = 'N';
    }
    out.write(record);
}
```

###
