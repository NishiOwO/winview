Welcome to WinView Release *BETA*
=================================
WinView is a simple, and lightweight image viewer for Windows, written in C!
This release supports following formats:
 - PNG
 - JPEG
 - TIFF
 - XPM
 - TGA
 - BMP (TODO)
 - GIF (no animations yet)

with following features:
 - Re-Aspect
 - Scaling

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
Copyright (c) 2025, WinView developers
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, 
  this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, 
  this list of conditions and the following disclaimer in the documentation 
  and/or other materials provided with the distribution.
* Neither the name of the <organization> nor the names of its contributors 
  may be used to endorse or promote products derived from this software 
  without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
