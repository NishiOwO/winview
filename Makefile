TARGET = x86_64-w64-mingw32-
CC = $(TARGET)gcc
AR = $(TARGET)ar
RC = $(TARGET)windres
CFLAGS = -DHAVE_CONFIG_H -I include -mwindows -g
RCFLAGS = -I include
LDFLAGS = -mwindows
LIBS = -lgdi32 -lcomctl32 -lcomdlg32

-include mk/$(OPTMK).mk

.PHONY: all clean get-version
.SUFFIXES: .c .o .rc .res

all: winview.exe

OBJS += src/ds.o src/main.o src/version.o src/util.o src/image.o
OBJS += src/tiff.o src/png.o src/jpeg.o
OBJS += src/winview.res
include deps.mk

winview.exe: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	$(AFTERCMD)

.rc.res:
	$(RC) $(RCFLAGS) -Ocoff $< $@

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

get-version:
	@grep wvversion src/version.c | head -n1 | grep -Eo '".+"' | xargs echo

clean:
	rm -f external/*/src/*.o src/*.o external/*.a *.exe src/*.res
