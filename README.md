YAAF - Yet Another Archive Format
======================================

YAAF is yet another archive format designed for data packing. It does not intend to replace 
other general purpose archiving alternatives such as 7z, ZIP, tar, etc... It was specifically
designed to suit the needs of data packing in regards to video games: High Speed unpacking of 
compressed data in order to reduce loading times.

YAAF uses the LZ4 compression algorithm (https://code.google.com/p/lz4/) to provide high speed
decompression of the compressed data contained in the archive. libYAAF provides functions akin
to C's FILE (_YAAF_File*_) to operate on the data in the archive.

YAAFCL is a command line utility which allows you to create compatible YAAF archives. Furthermore
it also allows you to list and extract files from a YAAF archive.



Tested Platforms
-----------------

Currently, YAAF has been tested on Linux (64Bits) and Mac OS X (64Bits). The code compiles on 
Windows with MSVC but the compile flags need to adjusted (contributions welcome ;)). The code 
should work on 32bit platforms, provided the C compiler has support for a 64bit unsigned and
signed integer.


Future work
-----------
* Multithread compression and decompression in yaafcl
* Support for builds on Android and iOS (32bit and 64bit)
* Support for builds with mingw on Windows
* TLS for thread specific error messages in libYAAF
