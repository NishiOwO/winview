#include <wvcommon.h>

#ifdef DOTGA
typedef struct tgaopaque {
	BOOL	  rle;
	int	  type;
	int	  color_origin;
	int	  color_length;
	int	  image_bits;
	RGBAPack* color_map;
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
		w = ReadAsWORD(rgb, 0);

		q->red	 = (double)((w >> 10) & 31) / 31 * 255;
		q->green = (double)((w >> 5) & 31) / 31 * 255;
		q->blue	 = (double)((w >> 0) & 31) / 31 * 255;
		q->alpha = w & (1 << 15) ? 0 : 255;
	}
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
				unsigned char d[2];
				int	      index = 0;

				if(opaque->image_bits == 16) {
					fread(&d[0], 2, 1, img->fp);

					index = ReadAsWORD(d, 0);
				} else if(opaque->image_bits == 8) {
					fread(&d[0], 1, 1, img->fp);

					index = d[0];
				}

				index = index - opaque->color_origin;
				if(index >= opaque->color_length) index = 0;

				row[4 * i + 0] = opaque->color_map[index].red;
				row[4 * i + 1] = opaque->color_map[index].green;
				row[4 * i + 2] = opaque->color_map[index].blue;
				row[4 * i + 3] = opaque->color_map[index].alpha;
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
	}

	return row;
}

static void TGADriverClose(void* ptr) {
	wvimage_t*   img    = ptr;
	tgaopaque_t* opaque = (tgaopaque_t*)img->opaque;

	if(opaque->color_map != NULL) free(opaque->color_map);
	free(opaque);

	fclose(img->fp);
	free(img);
}

wvimage_t* TryTGADriver(const char* path) {
	FILE*	       f = fopen(path, "rb");
	wvimage_t*     img;
	tgaopaque_t*   opaque;
	char*	       ext;
	int	       i;
	unsigned char* header;
	int	       color_bits;
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

	header = malloc(18);
	if(fread(header, 1, 18, f) < 18) {
		fclose(f);
		free(header);
		return NULL;
	}

	/* more supports appreciated */
	if(!(1 <= header[2] && header[2] <= 10)) {
		fclose(f);
		free(header);
		return NULL;
	}

	img = AllocateImage();

	img->name  = "TGA";
	img->close = TGADriverClose;
	img->read  = TGADriverRead;

	img->fp	    = f;
	img->opaque = Allocate(opaque);

	img->width  = ReadAsWORD(header, 12);
	img->height = ReadAsWORD(header, 14);

	img->direction = (header[17] & (1 << 5)) ? 0 : 1;

	color_bits	     = header[7];
	opaque->type	     = header[2];
	opaque->rle	     = (header[2] == 9 || header[2] == 10) ? TRUE : FALSE;
	opaque->color_origin = ReadAsWORD(header, 3);
	opaque->color_length = ReadAsWORD(header, 5);
	opaque->color_map    = NULL;
	opaque->image_bits   = header[16];

	if(opaque->color_length > 0) {
		opaque->color_map = malloc(sizeof(*opaque->color_map) * opaque->color_length);
	}

	/* skip identification field */
	fseek(f, header[0], SEEK_CUR);

	for(i = 0; i < opaque->color_length; i++) {
		ReadColor(f, color_bits, &opaque->color_map[i]);
	}

	free(header);

	return img;
}
#endif
