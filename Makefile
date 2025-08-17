TARGET = x86_64-w64-mingw32-
CC = $(TARGET)gcc
AR = $(TARGET)ar
CFLAGS = -DHAVE_CONFIG_H -O2
LDFLAGS =

.PHONY: all clean
.SUFFIXES: .c .o

all: winview.exe

OBJS += src/main.o
include deps.mk

winview.exe: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f */src/*.o src/*.o *.a *.exe
