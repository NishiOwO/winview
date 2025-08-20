#include <wvcommon.h>

#include <gif_lib.h>

typedef struct gifopaque {
	GifFileType* file;
} gifopaque_t;

static unsigned char* GIFDriverRead(void* ptr) {
	wvimage_t*   img    = ptr;
	gifopaque_t* opaque = img->opaque;

	return NULL; /* return row, we will need proper animation api */
}

static void GIFDriverClose(void* ptr) {
	wvimage_t*   img    = ptr;
	gifopaque_t* opaque = img->opaque;

	free(opaque);

	fclose(img->fp);
	free(img);
}

wvimage_t* TryGIFDriver(const char* path) {
	FILE*	     f = fopen(path, "rb");
	wvimage_t*   img;
	gifopaque_t* opaque;
	if(f == NULL) return NULL;

	img = AllocateImage();

	img->name  = "GIF";
	img->close = GIFDriverClose;
	img->read  = GIFDriverRead;

	img->fp	    = f;
	img->opaque = Allocate(opaque);

	/* return img; */
	return NULL;
}
