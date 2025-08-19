TARGET = x86_64-w64-mingw32-
CC = $(TARGET)gcc
AR = $(TARGET)ar
RC = $(TARGET)windres
CFLAGS = -DHAVE_CONFIG_H -I include -mwindows -O2
RCFLAGS = -I include
LDFLAGS = -mwindows
LIBS = -lgdi32 -lcomctl32 -lcomdlg32

-include mk/$(OPTMK).mk

.PHONY: all clean get-version
.SUFFIXES: .c .o .rc .res

all: winview.exe

OBJS += src/ds.o src/util.o
OBJS += src/ui/main.o src/ui/version.o src/ui/image.o src/ui/credits.o
OBJS += src/format/tiff.o src/format/png.o src/format/jpeg.o src/format/xpm.o
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
	@grep wvversion src/ui/version.c | head -n1 | grep -Eo '".+"' | xargs echo

clean:
	rm -f external/*/src/*.o src/*.o src/*/*.o external/*.a *.exe src/*.res
