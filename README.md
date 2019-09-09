# Fz1p.c
## _Lossless Text Compression Via Substring Substitution & Bit Packing!_
---------------------------------------
## Compilation and Initialization:
\<COMPILE> `$ gcc -o fz1p fz1p.c`</br>
\<COMPRESSOR> `$ ./fz1p filename.txt`</br>
\<DECOMPRESS> `$ ./fz1p filename.txt.FZ1P`

* Enter `-l` to print compression information during run:
  * `$ ./fz1p -l ...`
* Try compressing the `Macbeth.txt` sample file provided!
  
---------------------------------------
## Compression Information:
| AVG COMPRESSION | ORIGINAL FILE | COMPRESSED FILE |
|:---------------:|:-------------:|:---------------:|
|       50%       |      TXT      |       BIN       |

## Compression Technique (Decompression in Reverse):
1. Replace Chars of the Following String w/ Keys Generated Using the "Reserved Char Sequences":
   * Thus Spaces, the Lowercase Alphabet, and `.!?` are the Only Printable Chars Left in the File
 ```c
 "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-\'\",/:;<=>#$%&()*+@[\\]^_`{|}~\n\t"
 ```
2. Replace the 2 Most Common "Reserved Char Sequences" w/ '`-`' and '`'`'
3. Convert Spaces to Underscores
4. Abbreviate Commonly Repeated Substrings Found in the File
5. W/ Only 32 Different Printable Characters Left (Steps 1-3), Bit-Pack Each Into 5 Bits Rather Than 8
6. ***Mind = Blown***
---------------------------------------
## 3 Reserved Character Sequences for Compression:
* "`qx`", "`qy`", _&_ "`qz`" _are **NOT** to be used in any_ `.txt` _being compressed (lower **or** uppercase!)_
* `fz1p.c` _preemptively scans files to warn the client if any of the above are found_
