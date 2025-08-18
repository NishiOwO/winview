TARGET = x86_64-w64-mingw32-
CC = $(TARGET)gcc
AR = $(TARGET)ar
RC = $(TARGET)windres
CFLAGS = -DHAVE_CONFIG_H -I include -mwindows -O2
RCFLAGS = -I include
LDFLAGS = -mwindows
LIBS = -lgdi32 -lcomctl32 -lcomdlg32 -lws2_32 -luserenv -lntdll
RUST_TARGET = x86_64-pc-windows-gnu

-include mk/$(OPTMK).mk

.PHONY: all clean
.SUFFIXES: .c .o .rc .res .rs

all: winview.exe

OBJS += src/ds.o src/main.o src/version.o src/util.o src/image.o src/image_rs/image_rs.o src/image_rs/target/$(RUST_TARGET)/release/libimage_rs.a
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

src/image_rs/target/$(RUST_TARGET)/release/libimage_rs.a:
	cargo build --manifest-path src/image_rs/Cargo.toml --target=$(RUST_TARGET) --release

clean:
	rm -f external/*/src/*.o src/*.o external/*.a *.exe src/*.res
	cargo clean --manifest-path src/image_rs/Cargo.toml