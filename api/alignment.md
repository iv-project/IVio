## List of features seqan3
* read from file
* read from stream
* select fields -- unnecessary if we use (string_)views on buffer
* change all possible types (alphabets, containers)
* legal alphabet
* select format (at compile time, allows the user to put in custom formats)
* structured binding
* you can access the header information
* You can work with the sam_record

## List of features seqan2
* read from file
* read from stream
* no "legal alphabet", always Iupac
* you can access the header information
* You can work with the BamAlignmentRecord

## List of features b.i.o.
* You can set any container type to `string_view` (will point to buffer)

## List of features ReMa
* Set types via record

## List of features that we want
* read from file
* read from stream
* access header
* we want a record, be it tuple or view or struct!
* You can set any container type to `string_view` (will point to buffer)
* * Or always views?
* * How about pushing records to a list (cannot push views)?
* * record_view that returns containers?
* set types (alphabets, containers)
* set format (especially for streams)
* easy access to file extensions, compression extensions, combined extensions (decide which)
* pass config in constructor or make class itself config
