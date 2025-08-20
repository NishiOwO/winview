#include <wvcommon.h>

#include <tiffio.h>

typedef struct tiffopaque {
	TIFF* tiff;
} tiffopaque_t;

static unsigned char* TIFFDriverRead(void* ptr) {
	wvimage_t*     img    = ptr;
	tiffopaque_t*  opaque = img->opaque;
	unsigned char* dat    = malloc(img->width * img->height * 4);
	TIFF_UINT32_T* raster = malloc(img->width * img->height * 4);
	int	       i, j;

	TIFFReadRGBAImage(opaque->tiff, img->width, img->height, raster, 0);
	for(i = 0; i < img->height; i++) {
		for(j = 0; j < img->width; j++) {
			unsigned char* d = &dat[(i * img->width + j) * 4];
			DWORD	       r = raster[(img->height - i - 1) * img->width + j];
			d[0]		 = TIFFGetR(r);
			d[1]		 = TIFFGetG(r);
			d[2]		 = TIFFGetB(r);
			d[3]		 = TIFFGetA(r);
		}
	}

	free(raster);

	return dat;
}

static void TIFFDriverClose(void* ptr) {
	wvimage_t*    img    = ptr;
	tiffopaque_t* opaque = img->opaque;

	if(opaque->tiff != NULL) {
		TIFFClose(opaque->tiff);
	}
	free(opaque);

	free(img);
}

static void TIFFDriverError(const char* module, const char* fmt, va_list ap) {}

wvimage_t* TryTIFFDriver(const char* path) {
	wvimage_t*    img;
	tiffopaque_t* opaque;
	TIFF_UINT32_T width, height;
	char	      emsg[1024];

	img = AllocateImage();

	img->type  = WVIMAGE_READ_FRAME;
	img->name  = "TIFF";
	img->close = TIFFDriverClose;
	img->read  = TIFFDriverRead;

	img->opaque = Allocate(opaque);

	TIFFSetErrorHandler(TIFFDriverError);

	opaque->tiff = TIFFOpen(path, "rb");
	if(opaque->tiff == NULL) {
		TIFFDriverClose(img);
		return NULL;
	}

	TIFFGetField(opaque->tiff, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(opaque->tiff, TIFFTAG_IMAGELENGTH, &height);

	img->width  = width;
	img->height = height;

	return img;
}
