#include <wvcommon.h>

#include <jpeglib.h>
#include <jerror.h>

typedef struct jpegopaque {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr	      jerr;
	jmp_buf			      buf;
	int			      wentbad;
} jpegopaque_t;

static void jpeg_err(j_common_ptr cinfo) {
	jpegopaque_t* opaque = cinfo->client_data;

	opaque->wentbad = 1;

	longjmp(opaque->buf, 1);
}

static unsigned char* JPEGDriverRead(void* ptr) {
	wvimage_t*     img    = ptr;
	jpegopaque_t*  opaque = img->opaque;
	int	       i;
	unsigned char* jrow = malloc(opaque->cinfo.output_width * opaque->cinfo.num_components);
	unsigned char* row  = malloc(opaque->cinfo.output_width * 4);

	jpeg_read_scanlines(&opaque->cinfo, &jrow, 1);
	for(i = 0; i < opaque->cinfo.output_width; i++) {
		memcpy(&row[i * 4], &jrow[i * opaque->cinfo.num_components], 3);
		row[i * 4 + 3] = 255;
	}
	free(jrow);

	return row;
}

static void JPEGDriverClose(void* ptr) {
	wvimage_t*    img    = ptr;
	jpegopaque_t* opaque = img->opaque;

	if(!opaque->wentbad) jpeg_finish_decompress(&opaque->cinfo);
	jpeg_destroy_decompress(&opaque->cinfo);
	free(opaque);

	fclose(img->fp);
	free(img);
}

wvimage_t* TryJPEGDriver(const char* path) {
	FILE*	      f = fopen(path, "rb");
	wvimage_t*    img;
	jpegopaque_t* opaque;
	if(f == NULL) return NULL;

	img = AllocateImage();

	img->name  = "JPEG";
	img->close = JPEGDriverClose;
	img->read  = JPEGDriverRead;

	img->fp	    = f;
	img->opaque = Allocate(opaque);

	opaque->wentbad		  = 0;
	opaque->cinfo.client_data = opaque;
	opaque->cinfo.err	  = jpeg_std_error(&opaque->jerr);
	opaque->jerr.error_exit	  = jpeg_err;

	if(setjmp(opaque->buf)) {
		JPEGDriverClose(img);
		return NULL;
	}

	jpeg_create_decompress(&opaque->cinfo);
	jpeg_stdio_src(&opaque->cinfo, f);
	jpeg_read_header(&opaque->cinfo, TRUE);

	jpeg_start_decompress(&opaque->cinfo);

	img->width  = opaque->cinfo.output_width;
	img->height = opaque->cinfo.output_height;

	return img;
}
