TARGET = x86_64-w64-mingw32-
CC = $(TARGET)gcc
AR = $(TARGET)ar
RC = $(TARGET)windres
CFLAGS = -DHAVE_CONFIG_H -O2
LDFLAGS =

.PHONY: all clean
.SUFFIXES: .c .o .rc .res

all: winview.exe

OBJS += src/main.o src/winview.res
include deps.mk

winview.exe: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.rc.res:
	$(RC) -Ocoff $< $@

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f */src/*.o src/*.o *.a *.exe
