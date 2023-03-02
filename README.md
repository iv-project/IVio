# IVio
**IVio**, pronounced **for you** aka **io4**. Library for reading and writing bioinformatics file formats such as:
 - fasta (rw) + gzip (rw)
 - fastq (ro) + gzip (ro)
 - vcf (ro) / bcf (ro)
 - sam (ro) / bam (ro)

(legend - rw: read/write, ro: read-only)

## Benchmark
|        format           | |  seqan2 |  seqan3 | io2     | bio     | ivio        | direct       |
|:------------------------|-|--------:|:-------:|:-------:|:-------:|:-----------:|:------------:|
| reading .bam            | | 552MB/s | 225MB/s | 511MB/s |         | **707MB/s** |              |
| reading .sam            | | 119MB/s | 135MB/s | 116MB/s |         | **482MB/s**  |             |
| reading .bcf            | |         |         |         |   2MB/s | **12MB/s**  |              |
| reading .vcf            | | 187MB/s |         |         | 165MB/s | **401MB/s** |              |
| reading .fastq          | | 239MB/s | 172MB/s |         | 441MB/s | **887MB/s** |              |
| reading .fastq.gz       | | 185MB/s | 158MB/s |         | 268MB/s | **329MB/s** |              |
| reading short.fasta     | | 398MB/s | 238MB/s | 110MB/s | 338MB/s | **876MB/s** | ___1'294MB/s___ |
| reading short.fasta.gz  | | 170MB/s | 143MB/s |         | 147MB/s | **181MB/s** |              |
| reading long.fasta      | | 512MB/s | 538MB/s | 148MB/s | 399MB/s | **1'228MB/s** |___1'376MB/s___ |
| reading long.fasta.gz   | | 260MB/s | 266MB/s |         | 213MB/s | **301MB/s** |              |
| writing .fasta          | |**1'745MB/s**| 1'233MB/s|      |         |    1'377MB/s |              |
| writing .fasta.gz       | |1013MB/s | 923MB/s |         |         |**1'257MB/s** |              |

* **seqan2**: based on seqan 2.4.0
* **seqan3**: based on seqan3 commit "#a719fb0" from 13.Feb 2023
* **io2**: an io implementation for seqan3 based on seqan2
* **bio**: based on biocpp-core v0.6.0 nad biocpp-io
* **ivio**: implemented in this repo
* **direct**: skipping certain parsing step in ivio ("maximum speed" for this task)

## Usage Example
```c++
auto reader = ivio::fasta::reader{{file}};
for (auto record_view : reader) {
    std::cout << "id: " << record_view.id << "\n";
    std::cout << "seq: " << record_view.seq << "\n";
}
```
