DEFS = -DINTEGRATE

TARGET = x86_64-w64-mingw32-
CC = $(TARGET)gcc
AR = $(TARGET)ar
RC = $(TARGET)windres
CFLAGS = -DHAVE_CONFIG_H -I include -mwindows -O2
RCFLAGS = -I include
LDFLAGS = -mwindows
LIBS = -lgdi32 -lcomctl32 -lcomdlg32

-include mk/$(OPTMK).mk

.PHONY: all clean get-version format
.SUFFIXES: .c .o .rc .res

all: winview.exe $(TARGETS)

OBJS += src/ds.o src/util.o src/font.o
OBJS += src/ui/main.o src/ui/version.o src/ui/image.o src/ui/credits.o
OBJS += src/winview.res
include config.mk

winview.exe: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	$(AFTERCMD)

.rc.res:
	$(RC) $(RCFLAGS) -Ocoff $< $@

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

get-version:
	@grep wvversion src/ui/version.c | head -n1 | grep -Eo '".+"' | xargs echo

format:
	clang-format --verbose -i `find include src "(" -name "*.c" -or -name "*.h" ")" -and -not -name "stb_ds.h"`

clean:
	rm -f external/*/src/*.o src/*.o src/*/*.o external/*.a *.exe src/*.res
