Default parsing:
```
fasta_read_from_disk_seqan3/50          bytes_per_second=308.396M/s
fasta_read_from_disk_seqan3/1000        bytes_per_second=599.05M/s
fasta_read_from_disk_stream_seqan3/50   bytes_per_second=621.233M/s
fasta_read_from_disk_stream_seqan3/1000 bytes_per_second=592.356M/s
```

Change parsing to consume stream with a for loop (no take_until_view):
```
fasta_read_from_disk_seqan3/50          bytes_per_second=428.317M/s
fasta_read_from_disk_seqan3/1000        bytes_per_second=1.31234G/s
fasta_read_from_disk_stream_seqan3/50   bytes_per_second=1.43709G/s
fasta_read_from_disk_stream_seqan3/1000 bytes_per_second=1.51128G/s
```
