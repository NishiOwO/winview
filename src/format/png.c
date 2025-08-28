#include <wvcommon.h>

#ifdef DOPNG
#include <png.h>

typedef struct pngopaque {
	png_structp png;
	png_infop   info;
} pngopaque_t;

static void PNGCAPI user_error(png_structp png, const char* str) { longjmp(png_jmpbuf(png), 1); }

static void PNGCAPI user_warning(png_structp png, const char* str) {}

static unsigned char* PNGDriverRead(void* ptr) {
	wvimage_t*     img    = ptr;
	pngopaque_t*   opaque = img->opaque;
	unsigned char* row    = malloc(png_get_rowbytes(opaque->png, opaque->info));

	png_read_row(opaque->png, (png_bytep)row, NULL);

	return row;
}

static void PNGDriverClose(void* ptr) {
	wvimage_t*   img    = ptr;
	pngopaque_t* opaque = img->opaque;

	png_destroy_read_struct(&opaque->png, &opaque->info, NULL);
	free(opaque);

	fclose(img->fp);
	free(img);
}

STATIC_IF_INTEGRATE const char* DriverName = "PNG";
STATIC_IF_INTEGRATE const char* DriverExts = "*.png";

wvimage_t* TryPNGDriver(const char* path) {
	FILE*	     f = fopen(path, "rb");
	wvimage_t*   img;
	pngopaque_t* opaque;
	int	     depth, type;
	if(f == NULL) return NULL;

	img = AllocateImage();

	img->name  = DriverName;
	img->close = PNGDriverClose;
	img->read  = PNGDriverRead;

	img->fp	    = f;
	img->opaque = Allocate(opaque);

	opaque->png  = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	opaque->info = png_create_info_struct(opaque->png);
	if(setjmp(png_jmpbuf(opaque->png))) {
		PNGDriverClose(img);
		return NULL;
	}

	png_set_error_fn(opaque->png, NULL, user_error, user_warning);

	png_init_io(opaque->png, f);
	png_read_info(opaque->png, opaque->info);

	img->width  = png_get_image_width(opaque->png, opaque->info);
	img->height = png_get_image_height(opaque->png, opaque->info);
	depth	    = png_get_bit_depth(opaque->png, opaque->info);
	type	    = png_get_color_type(opaque->png, opaque->info);

	if(depth == 16) png_set_strip_16(opaque->png);
	if(type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(opaque->png);
	if(type == PNG_COLOR_TYPE_GRAY && depth < 8) png_set_expand_gray_1_2_4_to_8(opaque->png);
	if(png_get_valid(opaque->png, opaque->info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(opaque->png);
	if(type == PNG_COLOR_TYPE_RGB || type == PNG_COLOR_TYPE_GRAY || type == PNG_COLOR_TYPE_PALETTE) png_set_filler(opaque->png, 0xFF, PNG_FILLER_AFTER);
	if(type == PNG_COLOR_TYPE_GRAY || type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(opaque->png);
	png_read_update_info(opaque->png, opaque->info);

	return img;
}

END_FORMAT;
#endif
