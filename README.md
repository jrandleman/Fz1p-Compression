# Fz1p && Ziptxt
## _Lossless Compression &amp; Encryption For Text Files!_
---------------------------------------
### `fz1p.c` and `ziptxt.c` Trade-offs:
**_fz1p compression => txt <--> bin, more compression: via bit-packing_**</br>
**_ziptxt compression => txt <--> txt, with encryption: locally in textfile_**</br>

|   FILE   | AVG COMPRESSION | ENCRYPTION? |
|:---------|:---------------:|:-----------:|
| fz1p.c   |       50%       |      NO!    |
| ziptxt.c |       20%       |     YES!    |

---------------------------------------
### Compilation and Initialization:
\<COMPILE> `$ gcc -o fz1p fz1p.c`</br>
\<COMPRESSOR> `$ ./fz1p filename.txt`</br>
\<DECOMPRESS> `$ ./fz1p filename.txt.FZ1P`

\<COMPILE> `$ gcc -o ziptxt ziptxt.c`</br>
\<(D)ENCRYPT-(DE)COMPRESSOR> `$ ./ziptxt filename.txt yourpassword`

* _unique password per text file!_
* Enter `-l` to print compression/encryption information during run:
  * `$ ./fz1p -l ...`
  * `$ ./ziptxt -l ...`
  
---------------------------------------
### Both `fz1p.c` and `ziptxt.c` Reserve 3 Char Sequences for Compression:
* "`qx`", "`qy`", _&_ "`qz`" _are **NOT** to be used in any_ `.txt` _being compressed_
* _Both_ `fz1p.c` _&_ `ziptxt.c` _preemptively scan files to warn users if any of the above are found_

---------------------------------------
### `ziptxt.c` Compression via Replaced Repeated Substrings:
**LENGTH:** 50, **STRING:** yo how are you howard hope your homework went well<br/>
**LENGTH:** 25, **STRING:** AC DECDdBpErBmeworkFntFll<br/>
**KEY (_Stored in Password as Binary File_):**
* A: yo
* B: \_ho
* C: Bw
* D: ar
* E: e_Au
* F: \_we
