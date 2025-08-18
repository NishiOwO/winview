AR = ./tools/wlib-ar.pl
CC = ./tools/owcc.pl
CFLAGS += -b win32 -I$(WATCOM)/h/nt -std=c99
LDFLAGS += -b win32
AFTERCMD = wrc -q -i=include -i=$(WATCOM)/h/nt -bt=nt src/winview.rc winview.exe
