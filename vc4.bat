@echo off

set ARGS=/c /D__inline__= /DPBS_SMOOTH=0 /DGetWindowLongPtr=GetWindowLong /DULONG_PTR=ULONG /DDOXPM /DDOXBM /DDOPNG /DDOJPEG /DDOTGA /DDOMSP /DDOGIF /I..\..\external\libz\include /I..\..\external\libpng\include /I..\..\external\libjpeg\include /I..\..\external\libgif\include /I..\..\include

rd /s /q build
md build
cd build

md wv
cd wv
cl %ARGS% ..\..\src\*.c ..\..\src\ui\*.c ..\..\src\format\*.c
rc /fowinview.res /i..\..\include ..\..\src\winview.rc
cd ..

md extern
cd extern
cl %ARGS% ..\..\external\libz\src\*.c ..\..\external\libpng\src\*.c ..\..\external\libjpeg\src\*.c ..\..\external\libgif\src\*.c
cd ..

link /out:winview.exe wv\*.obj extern\*.obj wv\*.res gdi32.lib user32.lib comctl32.lib comdlg32.lib
