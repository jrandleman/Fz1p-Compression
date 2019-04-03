# Zip_Text
_Lossless Compression &amp; Encryption For Text Files!_
---------------------------------------
**_ziptxt compression => txt <--> txt, with encryption: locally in textfile_**</br>
**_fzip compression => txt <--> bin, more compression: 5 letters/4 bytes via bit packing (same cmd syntax)_**

$ gcc -o fzip fzip.c</br>
$ ./fzip textfilename.txt yourpassword (hide/show) (info)

* Enter \<hide> to compress and encrypt your text file
* Enter \<show> to decrypt and decompress text file
* Enter \<info> to print compression/encryption information during run
  * _unique password per text file!_
  
**===============================================================**<br/>
**_COMPRESSION VIA REPLACED REPEATED SUBSTRINGS:_**<br/>
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
