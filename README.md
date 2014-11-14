YAAF - Yet Another Archive Format
======================================
YAAF is a compressed archive format designed to with one specific goal:
High Speed Rutime Decompression of compressed packed data.

YAAF uses LZ4 to achieve high decompression speeds whilst sacrificing some
compression rate. You can find more information on LZ4 here
http://fastcompression.blogspot.de/p/lz4.html and
ttps://code.google.com/p/lz4/.

Furthermore, YAAF use block based compression to allow for faster seek
operations on the compressed data and the archive is accessed through memory
mapped files. This way the OS's memory manager handles the data transfer,
resulting in less data copies, easier multi-threaded access and an
overall improved performance.

Included with the source is a small tool **yaafcl** which can create, list,
verify and extract YAAF archives.

The library is written in C99 standard. The current version of the library
can be adapted to run under the c89 standard provided the default compression
and hashing algorithm are replaced with c89 compatible code.

Building the code
-----------------
Get the code with the following command in order to checkout all liked repositories:

    git clone --recursive https://github.com/LeanderBB/YAAF.git

YAAF uses [CMake][] as its build system and the following options can be defined during the configuration phase:

 * YAAF_EXCLUDE_TOOLS - Exclude YAAFCL from the build
 * YAAF_INCLUDE_TEST  - Build small test code

Tested Platforms
-----------------
YAAF has currently been tested on:

 * Linux (x86_32, x86_64, arm (32bit) )
 * Mac os X (x86_64)
 * Windows (MSVC 2013 x86_32, x86_64)

[CMake]: http://www.cmake.org/

