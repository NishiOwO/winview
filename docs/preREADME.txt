Welcome to WinView Release *BETA*
=================================
WinView is a simple, and lightweight image viewer for Windows, written in C!
This release supports following formats:
 - PNG
 - JPEG
 - TIFF
 - (GIF support is in progress - sit tightly)

with following features:
 - Re-Aspect

Compiling
---------
Source code is available from https://github.com/nishiowo/winview

You can compile WinView following compilers:
 - MinGW-w64
 - Open Watcom

You also need make to build.
Default target is x86_64 Windows, using MinGW-w64.
You can attach `OPTMK=watcom' to target 32-bit Windows using Open Watcom
compiler, or `TARGET=i686-w64-mingw32-' to target 32-bit Windows using
MinGW-w64.

Copyright Notice
----------------
