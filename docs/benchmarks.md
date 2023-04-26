## Benchmarks

|        format           | | seqan2        | seqan3    | io2     | bio     | seqtk       | ivio          | ivio (idx) | direct         |
|:------------------------|-|--------------:|:---------:|:-------:|:-------:|:-----------:|:-------------:|:----------:|:--------------:|
| reading .bam            | |     552MB/s   |   225MB/s | 511MB/s |         |             |   **707MB/s** |            |                |
| reading .sam            | |     119MB/s   |   135MB/s | 116MB/s |         |             |   **482MB/s** |            |                |
| reading .bcf            | |               |           |         |   2MB/s |             |    **12MB/s** |            |                |
| reading .vcf            | |     187MB/s   |           |         | 165MB/s |             |   **401MB/s** |            |                |
| reading .fastq          | |     239MB/s   |   172MB/s |         | 441MB/s |             |   **887MB/s** |            |                |
| reading .fastq.gz       | |     185MB/s   |   158MB/s |         | 268MB/s |             |   **329MB/s** |            |                |
| reading short.fasta     | |     398MB/s   |   243MB/s | 114MB/s | 342MB/s |   494MB/s   |   **888MB/s** |   515MB/s  |___1'294MB/s___ |
| reading short.fasta.gz  | |     170MB/s   |   148MB/s |         | 154MB/s | **184MB/s** |     182MB/s   |            |                |
| reading long.fasta      | |     512MB/s   |   547MB/s | 150MB/s | 404MB/s | 1'045MB/s   | **1'253MB/s** | 1'264MB/s  |___1'470MB/s___ |
| reading long.fasta.gz   | |     260MB/s   |   266MB/s |         | 218MB/s | **312MB/s** |     301MB/s   |            |                |
| writing .fasta          | | **1'745MB/s** | 1'233MB/s |         |         |             |   1'377MB/s   |            |                |
| writing .fasta.gz       | |   1'013MB/s   |   923MB/s |         |         |             | **1'257MB/s** |            |                |

* **seqan2**: based on seqan 2.4.0
* **seqan3**: based on seqan3 commit "#a719fb0" from 13. Feb 2023
* **io2**: an io implementation for seqan3 based on seqan2
* **bio**: based on biocpp-core v0.6.0 nad biocpp-io
* **ivio**: implemented in this repo
* **ivio (idx)**: implemented in this repo, but using an index when accessing each record
* **direct**: skipping certain parsing step in ivio ("maximum speed" for this task)
