# Fzip && Ziptxt
_Lossless Compression &amp; Encryption For Text Files!_
---------------------------------------
**_fzip compression => txt <--> bin, more compression: via bit-packing (same cmd syntax w/o password)_**</br>
**_ziptxt compression => txt <--> txt, with encryption: locally in textfile_**

$ gcc -o fzip fzip.c</br>
\<COMPRESSOR> $ ./fzip filename.txt</br>
\<DECOMPRESS> $ ./fzip filename.txt.FZ1P

$ gcc -o ziptxt ziptxt.c</br>
\<ENCRYPT-COMPRESSOR> $ ./ziptxt filename.txt yourpassword</br>
\<DECRYPT-DECOMPRESS> $ ./ziptxt filename.TZ1P.txt yourpassword

* _unique password per text file!_
* Enter \<-l> to print compression/encryption information during run:
  * $ ./fzip -l ...
  * $ ./ziptxt -l ...
  
  
**===============================================================**<br/>
**_ZIPTXT COMPRESSION VIA REPLACED REPEATED SUBSTRINGS:_**<br/>
**===============================================================**<br/>
**LENGTH:** 50, **STRING:** yo how are you howard hope your homework went well<br/>
**LENGTH:** 25, **STRING:** AC DECDdBpErBmeworkFntFll<br/>
**KEY (_Stored in Password as Binary File_):**
* A: yo
* B: \_ho
* C: Bw
* D: ar
* E: e_Au
* F: \_we
