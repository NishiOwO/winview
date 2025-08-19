#include <wvcommon.h>

#include <tiffio.h>

//#define MIGHT_BE_SLOW

typedef struct tiffopaque {
	TIFF* tiff;
	TIFFRGBAImage rgba;
#ifndef MIGHT_BE_SLOW
	TIFF_UINT32_T* image;
	int y;
#endif
} tiffopaque_t;

static unsigned char* TIFFDriverRead(void* ptr){
	wvimage_t* img = ptr;
	tiffopaque_t* opaque = img->opaque;
	unsigned char* row = malloc(img->width * 4);
	int i;
#ifdef MIGHT_BE_SLOW
	TIFF_UINT32_T* trow = malloc(img->width * 4);

	TIFFRGBAImageGet(&opaque->rgba, trow, img->width, 1);
	opaque->rgba.row_offset++;
#endif

	for(i = 0; i < img->width; i++){
		unsigned char* px = &row[i * 4];
#ifdef MIGHT_BE_SLOW
		TIFF_UINT32_T tpx = trow[i];
#else
		TIFF_UINT32_T tpx = opaque->image[img->width * opaque->y + i];
#endif
		px[0] = TIFFGetR(tpx);
		px[1] = TIFFGetG(tpx);
		px[2] = TIFFGetB(tpx);
		px[3] = TIFFGetA(tpx);
	}
#ifdef MIGHT_BE_SLOW
	free(trow);
#else
	opaque->y++;
#endif

	return row;
}

static void TIFFDriverClose(void* ptr){
	wvimage_t* img = ptr;
	tiffopaque_t* opaque = img->opaque;

	if(opaque->tiff != NULL){
#ifndef MIGHT_BE_SLOW
		free(opaque->image);
#endif
		TIFFClose(opaque->tiff);
	}
	free(opaque);

	free(img);
}

wvimage_t* TryTIFFDriver(const char* path){
	wvimage_t* img;
	tiffopaque_t* opaque;
	TIFF_UINT32_T width, height;
	char emsg[1024];

	img = AllocateImage();

	img->name = "TIFF";
	img->close = TIFFDriverClose;
	img->read = TIFFDriverRead;

	img->opaque = Allocate(opaque);

	opaque->tiff = TIFFOpen(path, "rb");
	if(opaque->tiff == NULL){
		TIFFDriverClose(img);
		return NULL;
	}
#ifndef MIGHT_BE_SLOW
	opaque->y = 0;
#endif

	TIFFGetField(opaque->tiff, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(opaque->tiff, TIFFTAG_IMAGELENGTH, &height);

	img->width = width;
	img->height = height;

#ifndef MIGHT_BE_SLOW
	opaque->image = malloc(img->width * img->height * 4);
#endif
	TIFFRGBAImageBegin(&opaque->rgba, opaque->tiff, 0, emsg);
	opaque->rgba.req_orientation = ORIENTATION_TOPLEFT;
#ifndef MIGHT_BE_SLOW
	TIFFRGBAImageGet(&opaque->rgba, opaque->image, img->width, img->height);
	TIFFRGBAImageEnd(&opaque->rgba);
#endif

	return img;
}
