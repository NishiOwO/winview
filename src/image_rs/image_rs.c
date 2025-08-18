#include <wvcommon.h>

typedef struct imagersopaque imagersopaque_t;

imagersopaque_t *ImageRSAllocate(const char * path);
unsigned char* ImageRSDriverRead(void* ptr);
void ImageRSDriverClose(void* ptr);
const char * ImageRSGetImageValues(imagersopaque_t * ptr, int * width, int * height);

wvimage_t* TryImageRSDriver(const char* path){
	FILE* f = fopen(path, "rb");
	if(f == NULL) return NULL;
	wvimage_t* img;

	img = AllocateImage();

	img->opaque = ImageRSAllocate(path);
	img->close = ImageRSDriverClose;
	img->read = ImageRSDriverRead;

	img->fp = f;
	if(img->opaque != NULL) {
		img->name = ImageRSGetImageValues(img->opaque, &img->width, &img->height);
	}

	return img;
}
