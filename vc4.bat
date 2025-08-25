@echo off
rd /s /q build
md build
cd build

md wv
cd wv
cl /c /D__inline__= /DPBS_SMOOTH=0 /DGetWindowLongPtr=GetWindowLong /DULONG_PTR=ULONG /DDOXPM /DDOXBM /DDOPNG /DDOJPEG /DDOTGA /DDOMSP /I..\..\external\libz\include /I..\..\external\libpng\include /I..\..\external\libjpeg\include /I..\..\include ..\..\src\*.c ..\..\src\ui\*.c ..\..\src\format\*.c
rc /fowinview.res /i..\..\include ..\..\src\winview.rc
cd ..

md extern
cd extern
cl /c /D__inline__= /DPBS_SMOOTH=0 /DGetWindowLongPtr=GetWindowLong /DULONG_PTR=ULONG /DDOXPM /DDOXBM /DDOPNG /DDOJPEG /DDOTGA /DDOMSP /I..\..\external\libz\include /I..\..\external\libpng\include /I..\..\external\libjpeg\include /I..\..\include ..\..\external\libz\src\*.c ..\..\external\libpng\src\*.c ..\..\external\libjpeg\src\*.c
cd ..

link /out:winview.exe wv\*.obj extern\*.obj wv\*.res gdi32.lib comctl32.lib comdlg32.lib
