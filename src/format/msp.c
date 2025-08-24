#include <wvcommon.h>

#ifdef DOMSP
typedef struct mspopaque {
} mspopaque_t;

static unsigned char* MSPDriverRead(void* ptr) {
	wvimage_t*     img    = ptr;
	mspopaque_t*   opaque = img->opaque;
	unsigned char* row    = malloc(img->width * 4);

	return row;
}

static void MSPDriverClose(void* ptr) {
	wvimage_t*   img    = ptr;
	mspopaque_t* opaque = img->opaque;

	free(opaque);

	fclose(img->fp);
	free(img);
}

wvimage_t* TryMSPDriver(const char* path) {
	FILE*	     f = fopen(path, "rb");
	wvimage_t*   img;
	mspopaque_t* opaque;
	if(f == NULL) return NULL;

	img = AllocateImage();

	img->name  = "MSPaint";
	img->close = MSPDriverClose;
	img->read  = MSPDriverRead;

	img->fp	    = f;
	img->opaque = Allocate(opaque);

	return img;
}
#endif
