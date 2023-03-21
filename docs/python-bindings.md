# IViopy

For python we have a package called **iviopy**.
This can be easily installed via `pip install iviopy`.


Here an example of reading a fasta file:
```python
import iviopy as ivio

for record in ivio.fasta.reader('file.fa'):
    print(record.id)
    print(record.seq)
```


Following classes are currently available:

  + `iviopy.fasta.record`
  + `iviopy.fasta.reader`

