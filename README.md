# Fz1p.c
## _Lossless Text Compression Via Substring Substitution & Bit Packing!_
---------------------------------------
## Compilation and Initialization:
\<COMPILE> `$ gcc -o fz1p fz1p.c`</br>
\<COMPRESSOR> `$ ./fz1p filename.txt`</br>
\<DECOMPRESS> `$ ./fz1p filename.txt.FZ1P`

* Enter `-l` to print compression information during run:
  * `$ ./fz1p -l ...`
  
---------------------------------------
## Compression Information:
| AVG COMPRESSION | ORIGINAL FILE | COMPRESSED FILE |
|:---------------:|:-------------:|:---------------:|
|       50%       |      TXT      |       BIN       |

## Compression Technique:
1. Abbreviates Commonly Repeated Substrings Found in the File
2. Replaces Chars of the Following String w/ Keys Generated Using the "Reserved Char Sequences":
   * Thus Spaces, the Lowercase Alphabet, and `.,!?-` are the Only Printable Chars Left in the File
 ```c
 "\"#$%&()*+,/0123456789:;<=>@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`{|}~\n\t"
 ```
3. All Spaces are Converted to Underscores
4. W/ Only 32 Different Printable Characters Left, Each are Bit-Packed into 5 Bits Rather than 8
5. ***Mind = Blown***
---------------------------------------
## 3 Reserved Character Sequences for Compression:
* "`qx`", "`qy`", _&_ "`qz`" _are **NOT** to be used in any_ `.txt` _being compressed_
* `fz1p.c` _will preemptively scan files to warn the client if any of the above are found_
