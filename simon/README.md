# IO design ideas:

## Fasta - Input:

### sketch how code could look like:
```
auto in = sequence_file_input {
    .input = "myfile.fasta",
    .alphabet = seqan3::dna5{},              // default `seqan3::dna15{}`
    .quality_alphabet = seqan3::phread42{},  // default `seqan3::pthread42{}` (is any think else even use full?)
    .formats = seqan3::format_input_embl {
        .complete_header = false
    } | seqan3::format_input_fasta{} | seqan3::format_input_formatq{} | format::format_input_genbank{} | format_input_sam{}},
    # or
    .formats = seqan3::format_input_all{} | seqan3::format_input_embl {
        .complete_header = false
    };
};

for (auto [id, seq, qual] : in) {
    // id,seq,qual are views
}

# or

for (auto&& record : in) {
    //record.id()
    //record.sequence()
    //record.qualities()
}
```

### Removal from seqan3
*Template arguments*
- lets remove `selected_field_ids` there is no benefit
- also valid_formats is replaced by moving it as a aggregated initialize option
*Type Tratis*
- `sequence_legal_alphabet`: not required, just use `.alphabet = seqan3::dna15{}` and than convert your view like `record.sequence() | views::convert_to<seqan3::dna5>`
  Advantages: less suprises.
- `sequence_container` shouldn't be defined by the user anyways. It should fullfill std::ranges::range concept, but user shouldn't define the container, if the want a certain container, just call `record.sequence() | seqan3::views::to<std::vector>()`
- `id_alphabet` we don't have any other real option than using `char`
- `id_container` also shouldn't be up to the user
- `quality_container` same as the other two containers

*Runtime options*
- `truncate_ids` - this has no value, just use something like `record.id() | std::views::take_until(' ')`
- `fasta_ignore_blanks_before_id` - this is not part of the fasta standard (should always be false), and if wanted just use something like `record.id() | std::views::skip(' ')`

## Fasta - Output:
### sketch how code could look like:
```
auto out = seqan3::sequence_file_output {
    .output = "somefile.fastq",
    .format = seqan3::format_output_fasta {
        .letters_per_line = 80, // by default 80
    } | seqan3::format_output_fastq {
        .double_id = false, // don't know what this is
    } | seqan3::format_output_embl {
        .complete_header = false; // complete header
    } | seqan3::format_output_genbank {
        .complete_header = false; // complete header
    }
};

for (...) {
    // all three variables have to be some std::ranges::range objects
    out.write(id, sequence, qual);
}
```



### Removal from seqan3
*Template arguments*
- lets remove `selected_field_ids` there is no benefit
- also valid_formats is replaced by moving it as a aggregated initialize option
*Type Tratis*
- not a std::ranges::range: it should not be iterated over this object...
*Runtime options*
- `fasta_legacy_id_marker`: lets not support outdated stuff
- `fasta_blank_before_id`: not part of the specification
- `add_carriage_return`: this is for windows, but don't have the reverse (also not sure if this actually works as expected, since iostream in text mode will add `\r` automatically




## Some comparisions to current seqan3 and hannes io
### ex1.cpp
```
// Standard example
===================

// current implementation:
seqan3::sequence_file_input in{"x.fasta"};

for (auto && rec : in)
    seqan3::debug_stream << rec.sequence();

// -------------------------------------------------------------------------------------------------------------------------

// Hannes implementation:
seqan3::seq_io::reader in{"x.fasta"};

for (auto && rec : in)
    seqan3::debug_stream << rec.sequence();

// -------------------------------------------------------------------------------------------------------------------------

// sgg io
seqan3::sgg_io::reader in{.input = "x.fasta"}; // seqan3::sgg_io::reader{"x.fasta"} would also work
for (auto && rec : in)
    seqan3::debug_stream << rec.sequence();
```

### ex2.cpp
```
// structured binding example
========================

// current implementation:
seqan3::sequence_file_input in{"x.fasta"};

for (auto & [ i, s, q ] : in)
    seqan3::debug_stream << "ID:  " << i << '\n';

// -------------------------------------------------------------------------------------------------------------------------

// Hannes implementation:
seqan3::seq_io::reader reader{"example.fasta"};

for (auto & [ i, s, q ] : reader)
    seqan3::debug_stream << "ID:  " << i << '\n';

// -------------------------------------------------------------------------------------------------------------------------

// sgg io
auto in = seqan3::sgg_io::reader{.input = "x.fasta"};
for (auto & [i, s, q] : in)
    seqan3::debug_stream << "ID: " << i << '\n';
```

### ex3.cpp
```
// Simple options example
=========================

// current implementation:
seqan3::sequence_file_input in{"x.fasta"};
in.options.truncate_ids = true; // must (should?) be done before the first record is read

// -------------------------------------------------------------------------------------------------------------------------

// Hannes implementation:
seqan3::seq_io::reader reader{"example.fasta", seqan3::seq_io::reader_options{.truncate_ids = true }};

// -------------------------------------------------------------------------------------------------------------------------

// sgg io
// truncate_ids wont exists anymore, using complete_header example instead
auto in = seqan3::sgg_io::reader{
    .input = "x.fasta",
    .format = seqan3::format_input_all{} | seqan3::format_input_embl {
        .complete_header = true
    }
};
```
### ex4.cpp
```
// Select different types
=========================

// current implementation:
struct my_traits : seqan3::sequence_file_input_default_traits_dna
{
    using sequence_alphabet = char; // instead of dna5

    template <typename alph>
    using sequence_container = std::basic_string<alph>; // must be defined as a template! String_view is not possible
};

seqan3::sequence_file_input<my_traits> fin{"x.fasta"};

// -------------------------------------------------------------------------------------------------------------------------

// Hannes implementation:
seqan3::seq_io::reader reader{"example.fasta",
                              seqan3::seq_io::reader_options{
                                    .field_types = seqan3::ttag<std::string_view, std::string, std::string>};

// -------------------------------------------------------------------------------------------------------------------------


// sgg io
auto in = seqan3::sgg_io::reader{
    .input = "x.fasta",
    .alphabet = char{},
};
```

### ex5.cpp
```
// Select different fields
=========================

// current implementation:
seqan3::sequence_file_input fin{"x.fasta", seqan3::fields<seqan3::field::id, seqan3::field::qual>()};

// -------------------------------------------------------------------------------------------------------------------------

// Hannes implementation:
seqan3::seq_io::reader reader{"example.fasta",
                              seqan3::seq_io::reader_options{
                                    .field_ids   = bio::vtag<seqan3::field::id, seqan3::field::qual>,
                                    .field_types = seqan3::ttag<std::string_view, std::vector<seqan3::phred62>>}; // types have to be also given

// -------------------------------------------------------------------------------------------------------------------------

// sgg io
// not needed, since there is no benefit in skipping these fields, just makes the api bloated
```
