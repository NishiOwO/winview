LIBZ_OBJS = external/libz/src/adler32.o external/libz/src/compress.o external/libz/src/crc32.o external/libz/src/deflate.o external/libz/src/gzclose.o external/libz/src/gzlib.o external/libz/src/gzread.o external/libz/src/gzwrite.o external/libz/src/infback.o external/libz/src/inffast.o external/libz/src/inflate.o external/libz/src/inftrees.o external/libz/src/trees.o external/libz/src/uncompr.o external/libz/src/zutil.o external/libz/src/gzclose.o external/libz/src/compress.o external/libz/src/adler32.o external/libz/src/gzlib.o external/libz/src/gzread.o external/libz/src/crc32.o external/libz/src/gzwrite.o external/libz/src/inffast.o external/libz/src/deflate.o external/libz/src/inftrees.o external/libz/src/uncompr.o external/libz/src/infback.o external/libz/src/trees.o external/libz/src/zutil.o external/libz/src/inflate.o
CFLAGS += -Iexternal/libz/include
OBJS += external/libz.a
external/libz.a: $(LIBZ_OBJS)
	$(AR) rcs $@ $(LIBZ_OBJS)

