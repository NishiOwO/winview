TARGET = x86_64-w64-mingw32-
CC = $(TARGET)gcc
AR = $(TARGET)ar
RC = $(TARGET)windres
CFLAGS = -DHAVE_CONFIG_H -g -I include -mwindows
RCFLAGS = -I include
LDFLAGS =
LIBS = -lgdi32 -lcomctl32 -lcomdlg32

.PHONY: all clean
.SUFFIXES: .c .o .rc .res

all: winview.exe

OBJS += src/ds.o src/main.o src/version.o src/util.o src/image.o
OBJS += src/tiff.o src/png.o src/jpeg.o
OBJS += src/winview.res
include deps.mk

winview.exe: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.rc.res:
	$(RC) $(RCFLAGS) -Ocoff $< $@

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f external/*/src/*.o src/*.o external/*.a *.exe
