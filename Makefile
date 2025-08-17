TARGET = x86_64-w64-mingw32-
CC = $(TARGET)gcc
AR = $(TARGET)ar
CFLAGS = -DHAVE_CONFIG_H
LDFLAGS =

.PHONY: all clean
.SUFFIXES: .c .o

OBJS = libjpeg.a libpng.a libtiff.a libz.a

all: winview.exe

include deps.mk

winview.exe: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f */src/*.o *.a *.exe
