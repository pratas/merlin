MERLIN: compression boost for any FASTQ compressing tool

## 1. INSTALLATION ##
<pre>
git clone https://github.com/pratas/merlin.git
cd merlin/src/
make
</pre>

## 2. RUNNING ##

### 2.1 Compression ###
Run:
<pre>
./MERLIN -v file.fq > OUTPUT
</pre>
and store file.fq.mindex (you can compress this with gzip).
Then compress the file file.fq with any fastq compressor.

### 2.2 Decompression ###
Decompress the file with the same fastq compressor and them run:
<pre>
./MERLIN -v -d file.fq.mindex OUTPUT > original.fq
</pre>

## 3. USAGE ##
To see the possible options of MERLIN type
<pre>
./MERLIN -h
</pre>
These will print the following options:
<pre>
<p>
Usage: MERLIN [OPTION]... [FILE] &#62 [STDOUT]                          
Compression boost for any FASTQ compressing tool.                    
                                                                     
Non-mandatory arguments:                                             
                                                                     
  -h                   give this help,                               
  -V                   display version number,                       
  -v                   verbose mode (more information),              
  -l                   lossy (does not store read order),            
  -d &#60FILE&#62            unMERLIN (back to the original file),         
                       note: &#60FILE&#62 is &#60FILE&#62.mindex.                
                                                                     
Mandatory arguments:                                                 
                                                                     
  [FILE]               input filename,                               
  &#62 [FILE]             stdout filename,                              
                                                                     
Report issues to &#60{pratas,ap}@ua.pt&#62.
</p>
</pre>

## 4. CITATION ##

On using this software/method please cite: Submitted paper.

## 5. ISSUES ##

For any issue let us know at [issues link](https://github.com/pratas/merlin/issues).

## 6. LICENSE ##

GPL v3.

For more information see LICENSE file or visit
<pre>http://www.gnu.org/licenses/gpl-3.0.html</pre>

