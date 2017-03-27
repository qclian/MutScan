# MutScan
Detect and visualize target mutations by scanning FastQ files directly
* Ultra sensitive.
* 50X+ faster than normal pipeline (i.e. BWA + Samtools + GATK/VarScan/Mutect).
* Very easy to use and need nothing else. No alignment, no reference genome, no variant call, no...
* Contains most actionable mutation points for cancer, like EGFR p.L858R, BRAF p.V600E...
* Beautiful and informative HTML report with informative pileup visualization.
* Multi-threading support.
* Supports both single-end and pair-end data.
* For pair-end data, MutScan will try to merge each pair, and do quality adjustment and error correction.
* Able to scan the mutations in a VCF file, which can be used to visualize called variants.
* Can be used to filter false-positive mutations. i.e. MutScan can handle highly repetive sequence to avoid false INDEL calling.

# Sample report
http://opengene.org/MutScan/report.html

# Download
Get latest (may be not stable)
```shell
# download use http
https://github.com/OpenGene/MutScan/archive/master.zip

# or download use git
git clone https://github.com/OpenGene/MutScan.git
```
Get the stable releases  
https://github.com/OpenGene/MutScan/releases/latest

# Build
MutScan only depends on `libz`, which is always available on Linux or Mac systems. If your system has no `libz`, install it first.
```shell
cd MutScan
make
```

# Usage
```shell
usage: mutscan -1 <read1_file> -2 <read2_file> [options]...
options:
  -1, --read1                read1 file name (string)
  -2, --read2                read2 file name (string [=])
  -m, --mutation             mutation file name, can be a CSV format or a VCF format (string [=])
  -r, --ref                  reference fasta file name (only needed when mutation file is a VCF) (string [=])
  -h, --html                 filename of html report, default is mutscan.html in work directory (string [=mutscan.html])
  -t, --thread               worker thread number, default is 4 (int [=4])
  -k, --mark                 when mutation file is a vcf file, --mark means only process the records with FILTER column is M
  -l, --legacy               use legacy mode, usually much slower but may be able to find a little more reads in certain case
  -s, --standalone           output standalone HTML report with single file. Don't use this option when scanning too many target mutations (i.e. >1000 mutations)
  -n, --no-original-reads    dont output original reads in HTML and text output. Will make HTML report files a bit smaller
  -?, --help                 print this message
```
The plain text result, which contains the detected mutations and their support reads, will be printed directly. You can use `>` to redirect output to a file, like:
```shell
mutscan -1 <read1_file_name> -2 <read2_file_name> > result.txt
```
MutScan generate a very informative HTML file report, default is `mutscan.html` in the work directory. You can change the file name with `-h` argument, like:
```
mutscan -1 <read1_file_name> -2 <read2_file_name> -h report.html
```
## single-end and pair-end
For single-end sequencing data, `-2` argument is omitted:
```
mutscan -1 <read1_file_name>
```
## multi-threading
`-t` argument specify how many worker threads will be launched. The default thread number is `4`. Suggest to use a number less than the CPU cores of your system.

# Mutation file
* Mutation file, specified by `-m`, can be a `CSV file`, or a `VCF file`. 
* If no `-m` specified, MutScan will use the built-in default mutation file with about 60 cancer related mutation points.
* If a CSV is provided, no reference genome assembly needed.
* If a VCF is provided, corresponding reference genome assembly should be provided (i.e. ucsc.hg19.fasta), and should not be zipped.

## CSV-format mutation file
A CSV file with columns of `name`, `left_seq_of_mutation_point`, `mutation_seq`, `right_seq_of_mutation_point` and `chromosome(optional)`
```csv
#name, left_seq_of_mutation_point, mutation_seq, right_seq_of_mutation_point, chromosome
NRAS-neg-1-115258748-2-c.34G>A-p.G12S-COSM563, GGATTGTCAGTGCGCTTTTCCCAACACCAC, T, TGCTCCAACCACCACCAGTTTGTACTCAGT, chr1
NRAS-neg-1-115252203-2-c.437C>T-p.A146V-COSM4170228, TGAAAGCTGTACCATACCTGTCTGGTCTTG, A, CTGAGGTTTCAATGAATGGAATCCCGTAAC, chr1
BRAF-neg-7-140453136-15-c.1799T>A -V600E-COSM476, AACTGATGGGACCCACTCCATCGAGATTTC, T, CTGTAGCTAGACCAAAATCACCTATTTTTA, chr7
EGFR-pos-7-55241677-18-c.2125G>A-p.E709K-COSM12988, CCCAACCAAGCTCTCTTGAGGATCTTGAAG, A, AAACTGAATTCAAAAAGATCAAAGTGCTGG, chr7
EGFR-pos-7-55241707-18-c.2155G>A-p.G719S-COSM6252, GAAACTGAATTCAAAAAGATCAAAGTGCTG, A, GCTCCGGTGCGTTCGGCACGGTGTATAAGG, chr7
EGFR-pos-7-55241707-18-c.2155G>T-p.G719C-COSM6253, GAAACTGAATTCAAAAAGATCAAAGTGCTG, T, GCTCCGGTGCGTTCGGCACGGTGTATAAGG, chr7
```
`testdata/mutations.csv` gives an example of CSV-format mutation file

## VCF-format mutation file
A standard VCF can be used as a mutation file, with file extension `.vcf` or `.VCF`. If the mutation file is a VCF file, you should specify the `reference assembly file` by `-r <ref.fa>`. For example the command can be:
```shell
mutscan -1 R1.fq -2 R2.fq -m target.vcf -r hg19.fa
```

# HTML output
* A HTML report will be generated, and written to the given filename. See http://opengene.org/MutScan/report.html for an example.
* The default file name is `mutscan.html`, and a folder `mutscan.html.files` will be also generated.
* By default, an indivudal HTML file will be generated for each found mutation. But you can specify `-s` or `--standalone` to contain all mutations in a single HTML file. Be caution with this mode if you are scanning too many records (for example, scanning VCF), it will give you a very big HTML file and is not loadable by browser.
* Here is a screenshot for the pileup of a mutation generated by MutScan:   

![image](http://www.opengene.org/MutScan/indel.jpg)  
* An pileup of an indel is displayed above, from which we can find MutScan can make excellent pileup even in such highly repetitive area. 
* The color of each base indicates its quality, and the quality will be shown when mouse over.
* In first column, d means the edit distance of match, and --> means forward, <-- means reverse 
