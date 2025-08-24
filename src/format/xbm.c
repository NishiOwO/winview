#include <wvcommon.h>

#ifdef DOXBM
typedef struct xbmopaque {
	unsigned char leftover;
	int	      shift;
} xbmopaque_t;

static unsigned char* XBMDriverRead(void* ptr) {
	wvimage_t*     img    = ptr;
	xbmopaque_t*   opaque = (xbmopaque_t*)img->opaque;
	unsigned char* row    = malloc(img->width * 4);
	int	       wrote  = 0;
	unsigned char  c;
	memset(row, 0, img->width * 4);

repeat:
	c = 0;
	while(c != '0') {
		if(fread(&c, 1, 1, img->fp) < 1) goto failed;
	}
	fread(&c, 1, 1, img->fp);
	while(1) {
		fread(&c, 1, 1, img->fp);

		if(('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F')) {
			unsigned char d	 = ParseHex(&c, 1);
			opaque->leftover = opaque->leftover << 4;
			opaque->leftover = opaque->leftover | d;
		} else {
			break;
		}
	}

failed:
	while(1) {
		c		   = (opaque->leftover & (1 << opaque->shift)) ? 0 : 255;
		row[4 * wrote + 0] = c;
		row[4 * wrote + 1] = c;
		row[4 * wrote + 2] = c;
		row[4 * wrote + 3] = 255;

		wrote++;
		if((++opaque->shift) == 8) {
			opaque->shift	 = 0;
			opaque->leftover = 0;
			break;
		}
	}

	if(wrote < img->width) goto repeat;

	return row;
}

static void XBMDriverClose(void* ptr) {
	wvimage_t*   img    = ptr;
	xbmopaque_t* opaque = (xbmopaque_t*)img->opaque;

	free(opaque);

	fclose(img->fp);
	free(img);
}

wvimage_t* TryXBMDriver(const char* path) {
	FILE*	     f = fopen(path, "rb");
	wvimage_t*   img;
	xbmopaque_t* opaque;
	char	     buffer[4097]; /* if stuff cannot be found in 4k - we give up */
	int	     i;
	BOOL	     macro     = FALSE;
	BOOL	     hasbits   = FALSE;
	BOOL	     hasstatic = FALSE;
	BOOL	     haschar   = FALSE;
	char	     mode      = 0;
	int	     bitsloc   = 0;
	int	     start     = -1;
	int	     w = -1, h = -1;
	if(f == NULL) return NULL;

	memset(buffer, 0, sizeof(buffer));
	fread(buffer, 1, sizeof(buffer) - 1, f);
	fseek(f, 0, SEEK_SET);

	for(i = 0; buffer[i] != 0; i++) {
		if(macro && buffer[i] == '\n') {
			if(mode == 'w' || mode == 'h') {
				char* dest = malloc(i - start + 1);
				memcpy(dest, &buffer[start], i - start);
				dest[i - start] = 0;

				if(mode == 'w') {
					w = atoi(dest);
				} else if(mode == 'h') {
					h = atoi(dest);
				}

				free(dest);
			}

			macro = FALSE;
			mode  = 0;
			start = -1;

			hasstatic = FALSE;
			haschar	  = FALSE;
		} else if(macro && CompareStringSafely(&buffer[i], "_width")) {
			mode = 'w';
		} else if(macro && CompareStringSafely(&buffer[i], "_height")) {
			mode = 'h';
		} else if((mode == 'w' || mode == 'h') && ('0' <= buffer[i] && buffer[i] <= '9')) {
			if(start == -1) start = i;
		} else if(hasstatic && haschar && CompareStringSafely(&buffer[i], "_bits")) {
			hasbits = TRUE;
			bitsloc = i;
		} else if(CompareStringSafely(&buffer[i], "#define")) {
			macro = TRUE;
		} else if(CompareStringSafely(&buffer[i], "static")) {
			hasstatic = TRUE;
		} else if(CompareStringSafely(&buffer[i], "char")) {
			haschar = TRUE;
		}
	}

	if(w == -1 || h == -1 || !hasbits) {
		fclose(f);
		return NULL;
	}

	fseek(f, bitsloc, SEEK_SET);

	img = AllocateImage();

	img->name  = "XBM";
	img->close = XBMDriverClose;
	img->read  = XBMDriverRead;

	img->fp	    = f;
	img->opaque = Allocate(opaque);

	img->width  = w;
	img->height = h;

	opaque->leftover = 0;
	opaque->shift	 = 0;

	return img;
}
#endif
