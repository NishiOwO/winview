#include <wvcommon.h>

#ifdef DOTGA
typedef struct tgaopaque {
	BOOL	  rle;
	int	  type;
	int	  color_origin;
	int	  color_length;
	int	  image_bits;
	RGBAPack* color_map;
	RGBAPack* color_stack;
} tgaopaque_t;

static char* tgaext = ".tga";

static void ReadColor(FILE* fp, int bits, RGBAPack* q) {
	int i;
	if(bits == 24) {
		unsigned char rgb[3];
		fread(&rgb[0], 1, 3, fp);

		q->red	 = rgb[2];
		q->green = rgb[1];
		q->blue	 = rgb[0];
		q->alpha = 255;
	} else if(bits == 32) {
		unsigned char rgb[4];
		fread(&rgb[0], 1, 4, fp);

		q->red	 = rgb[2];
		q->green = rgb[1];
		q->blue	 = rgb[0];
		q->alpha = rgb[3] ? 0 : 255;
	} else if(bits == 16) {
		unsigned char rgb[2];
		WORD	      w;

		fread(&rgb[0], 1, 2, fp);
		w = ReadAsLittleWORD(rgb, 0);

		q->red	 = (double)((w >> 10) & 31) / 31 * 255;
		q->green = (double)((w >> 5) & 31) / 31 * 255;
		q->blue	 = (double)((w >> 0) & 31) / 31 * 255;
		q->alpha = w & (1 << 15) ? 0 : 255;
	}
}

static RGBAPack* GetColorMap(wvimage_t* img) {
	tgaopaque_t*  opaque = (tgaopaque_t*)img->opaque;
	unsigned char d[2];
	int	      index = 0;

	if(opaque->image_bits == 16) {
		fread(&d[0], 2, 1, img->fp);

		index = ReadAsLittleWORD(d, 0);
	} else if(opaque->image_bits == 8) {
		fread(&d[0], 1, 1, img->fp);

		index = d[0];
	}

	index = index - opaque->color_origin;
	if(index >= opaque->color_length) index = 0;

	return &opaque->color_map[index];
}

static unsigned char* TGADriverRead(void* ptr) {
	wvimage_t*     img    = ptr;
	tgaopaque_t*   opaque = (tgaopaque_t*)img->opaque;
	unsigned char* row    = malloc(img->width * 4);
	int	       i;

	memset(row, 0, img->width * 4);
	if(opaque->type == 1) {
		if(opaque->color_map != NULL) {
			for(i = 0; i < img->width; i++) {
				RGBAPack* p = GetColorMap(img);

				row[4 * i + 0] = p->red;
				row[4 * i + 1] = p->green;
				row[4 * i + 2] = p->blue;
				row[4 * i + 3] = p->alpha;
			}
		}
	} else if(opaque->type == 2) {
		for(i = 0; i < img->width; i++) {
			RGBAPack rgba;
			ReadColor(img->fp, opaque->image_bits, &rgba);

			row[4 * i + 0] = rgba.red;
			row[4 * i + 1] = rgba.green;
			row[4 * i + 2] = rgba.blue;
			row[4 * i + 3] = rgba.alpha;
		}
	} else if(opaque->type == 3) {
		for(i = 0; i < img->width; i++) {
			unsigned char c;
			fread(&c, 1, 1, img->fp);

			row[4 * i + 0] = c;
			row[4 * i + 1] = c;
			row[4 * i + 2] = c;
			row[4 * i + 3] = 255;
		}
	} else if(opaque->type == 9 || opaque->type == 10) {
		unsigned char c;
		int	      count;
		RGBAPack      p;
		int	      wrote = 0;

	repeat:
		fread(&c, 1, 1, img->fp);

		count = c & 0x7f;
		if(c & (1 << 7)) {
			if(opaque->type == 9) {
				RGBAPack* ptr = GetColorMap(img);
				memcpy(&p, ptr, sizeof(p));
			} else {
				ReadColor(img->fp, opaque->image_bits, &p);
			}

			for(i = 0; i < count + 1; i++) {
				arrput(opaque->color_stack, p);
			}
		} else {
			if(opaque->type == 9) {
				for(i = 0; i < count; i++) {
					RGBAPack* ptr = GetColorMap(img);
					memcpy(&p, ptr, sizeof(p));
					arrput(opaque->color_stack, p);
				}
			} else {
				for(i = 0; i < count; i++) {
					ReadColor(img->fp, opaque->image_bits, &p);
					arrput(opaque->color_stack, p);
				}
			}
		}

		for(i = 0; wrote < img->width && arrlen(opaque->color_stack) > 0; i++) {
			row[4 * wrote + 0] = opaque->color_stack[0].red;
			row[4 * wrote + 1] = opaque->color_stack[0].green;
			row[4 * wrote + 2] = opaque->color_stack[0].blue;
			row[4 * wrote + 3] = opaque->color_stack[0].alpha;
			wrote++;
			arrdel(opaque->color_stack, 0);
		}

		if(wrote < img->width) goto repeat;
	}

	return row;
}

static void TGADriverClose(void* ptr) {
	wvimage_t*   img    = ptr;
	tgaopaque_t* opaque = (tgaopaque_t*)img->opaque;

	if(opaque->color_stack != NULL) arrfree(opaque->color_stack);
	if(opaque->color_map != NULL) free(opaque->color_map);
	free(opaque);

	fclose(img->fp);
	free(img);
}

wvimage_t* TryTGADriver(const char* path) {
	FILE*	      f = fopen(path, "rb");
	wvimage_t*    img;
	tgaopaque_t*  opaque;
	char*	      ext;
	int	      i;
	unsigned char header[18];
	int	      color_bits;
	if(f == NULL) return NULL;

	/**
	 * please let me know if there is a better way!
	 * i don't know how to identify TGA other than this way
	 */
	ext = strrchr(path, '.'); /* must be .tga */
	if(ext == NULL || strlen(ext) != 4) {
		fclose(f);
		return NULL;
	}
	for(i = 0; i < 4; i++) {
		if(tolower(ext[i]) != tgaext[i]) {
			fclose(f);
			return NULL;
		}
	}

	if(fread(header, 1, 18, f) < 18) {
		fclose(f);
		return NULL;
	}

	/* more supports appreciated */
	if(!(1 <= header[2] && header[2] <= 10)) {
		fclose(f);
		return NULL;
	}

	img = AllocateImage();

	img->name  = "TGA";
	img->close = TGADriverClose;
	img->read  = TGADriverRead;

	img->fp	    = f;
	img->opaque = Allocate(opaque);

	img->width  = ReadAsLittleWORD(header, 12);
	img->height = ReadAsLittleWORD(header, 14);

	img->direction = (header[17] & (1 << 5)) ? 0 : 1;

	color_bits	     = header[7];
	opaque->type	     = header[2];
	opaque->rle	     = (header[2] == 9 || header[2] == 10) ? TRUE : FALSE;
	opaque->color_origin = ReadAsLittleWORD(header, 3);
	opaque->color_length = ReadAsLittleWORD(header, 5);
	opaque->color_map    = NULL;
	opaque->image_bits   = header[16];
	opaque->color_stack  = NULL;

	if(opaque->color_length > 0) {
		opaque->color_map = malloc(sizeof(*opaque->color_map) * opaque->color_length);
	}

	/* skip identification field */
	fseek(f, header[0], SEEK_CUR);

	for(i = 0; i < opaque->color_length; i++) {
		ReadColor(f, color_bits, &opaque->color_map[i]);
	}

	return img;
}
#endif
