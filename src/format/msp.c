#include <wvcommon.h>

#ifdef DOMSP
typedef struct mspopaque {
	unsigned char* stack;
	unsigned char  shift;
	unsigned char  leftover;
	int	       version;
} mspopaque_t;

static unsigned char* MSPDriverRead(void* ptr) {
	wvimage_t*     img    = ptr;
	mspopaque_t*   opaque = img->opaque;
	unsigned char* row    = malloc(img->width * 4);
	int	       wrote  = 0;
	unsigned char  c;
	int	       i;

	if(opaque->version == 1) {
		if(opaque->shift > 0) {
			for(; opaque->shift < 8; opaque->shift++) {
				c		   = (opaque->leftover & (1 << (7 - opaque->shift))) ? 255 : 0;
				row[4 * wrote + 0] = c;
				row[4 * wrote + 1] = c;
				row[4 * wrote + 2] = c;
				row[4 * wrote + 3] = 255;
			}
			opaque->shift = 0;
		}
		while(wrote < img->width) {
			fread(&opaque->leftover, 1, 1, img->fp);

			for(opaque->shift = 0; opaque->shift < 8; opaque->shift++) {
				c		   = (opaque->leftover & (1 << (7 - opaque->shift))) ? 255 : 0;
				row[4 * wrote + 0] = c;
				row[4 * wrote + 1] = c;
				row[4 * wrote + 2] = c;
				row[4 * wrote + 3] = 255;
				wrote++;
				if(wrote == img->width) break;
			}
		}
	} else {
		unsigned char type, count;

	repeat:
		fread(&type, 1, 1, img->fp);
		if(type == 0) {
			/* RLE */
			fread(&count, 1, 1, img->fp);
			fread(&c, 1, 1, img->fp);
			for(i = 0; i < count; i++) arrput(opaque->stack, c);
		} else {
			count = type;
			for(i = 0; i < count; i++) {
				fread(&c, 1, 1, img->fp);
				arrput(opaque->stack, c);
			}
		}

		for(i = 0; wrote < img->width && arrlen(opaque->stack) > 0; i++) {
			c		   = (opaque->stack[0] & (1 << (7 - opaque->shift))) ? 255 : 0;
			row[4 * wrote + 0] = c;
			row[4 * wrote + 1] = c;
			row[4 * wrote + 2] = c;
			row[4 * wrote + 3] = 255;
			wrote++;
			if(img->width == wrote) break;

			if((++opaque->shift) == 8) {
				arrdel(opaque->stack, 0);
				opaque->shift = 0;
			}
		}

		if(wrote < img->width) goto repeat;

		if(opaque->shift > 0) {
			opaque->shift = 0;
			arrdel(opaque->stack, 0);
		}
	}

	return row;
}

static void MSPDriverClose(void* ptr) {
	wvimage_t*   img    = ptr;
	mspopaque_t* opaque = img->opaque;

	if(opaque->stack != NULL) free(opaque->stack);
	free(opaque);

	fclose(img->fp);
	free(img);
}

wvimage_t* TryMSPDriver(const char* path) {
	FILE*	      f = fopen(path, "rb");
	wvimage_t*    img;
	mspopaque_t*  opaque;
	unsigned char header[32];
	BOOL	      v1;
	BOOL	      v2;
	if(f == NULL) return NULL;

	if(fread(header, 1, 32, f) < 32) {
		fclose(f);
		return NULL;
	}

	v1 = ReadAsLittleWORD(header, 0) == 0x6144 && ReadAsLittleWORD(header, 2) == 0x4d6e;
	v2 = ReadAsLittleWORD(header, 0) == 0x694c && ReadAsLittleWORD(header, 2) == 0x536e;
	if(!v1 && !v2) {
		fclose(f);
		return NULL;
	}

	img = AllocateImage();

	img->name  = "MSPaint";
	img->close = MSPDriverClose;
	img->read  = MSPDriverRead;

	img->fp	    = f;
	img->opaque = Allocate(opaque);

	img->width  = ReadAsLittleWORD(header, 4);
	img->height = ReadAsLittleWORD(header, 6);

	opaque->stack	 = NULL;
	opaque->shift	 = 0;
	opaque->version	 = v1 ? 1 : 2;
	opaque->leftover = 0;

	if(opaque->version == 2) fseek(f, img->height * 2, SEEK_CUR);

	return img;
}
#endif
