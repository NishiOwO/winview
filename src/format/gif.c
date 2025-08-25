#include <wvcommon.h>

#ifdef DOGIF
#include <gif_lib.h>

typedef struct gifopaque {
	int	     y;
	GifFileType* file;
} gifopaque_t;

#define EXTRACT_BITS(x, n, w) ((x >> n) & ((1U << w) - 1))
#define FRAME_NUM (0) // nish didnt implement animation api boo

static unsigned char* GIFDriverRead(void* ptr) {
	wvimage_t*     img    = ptr;
	gifopaque_t*   opaque = img->opaque;
	unsigned char* row;
	int i;

	if(opaque->y >= img->height) return NULL;

	row = malloc(img->width * 4);
	memset(row, 0, img->width * 4);

	for(i = 0; i <= FRAME_NUM; i++) {
		SavedImage*	image	   = &opaque->file->SavedImages[i];
		GifImageDesc*	image_desc = &image->ImageDesc;
		ColorMapObject* color_map  = opaque->file->SColorMap;
		int		transp = 0, transp_idx = -1, dispose = DISPOSAL_UNSPECIFIED;
		int		yy = opaque->y - image_desc->Top, j;

		if(image_desc->ColorMap) color_map = image_desc->ColorMap;
		if(color_map == NULL) return NULL;

		// according to the docs, if there are multiple graphics extensions, the last one is applied
		for(j = 0; j < image->ExtensionBlockCount; j++) {
			ExtensionBlock* blk = &image->ExtensionBlocks[j];

			// filter only graphics extensions
			if(blk->Function != GRAPHICS_EXT_FUNC_CODE || blk->ByteCount != 4) continue;

			transp	= EXTRACT_BITS(blk->Bytes[0], 0, 1);
			dispose = EXTRACT_BITS(blk->Bytes[0], 2, 3);
			if(transp) transp_idx = blk->Bytes[3];
		}

		// apply disposal
		switch(dispose) {

		// fill row with background color before processing
		case DISPOSAL_UNSPECIFIED:
		case DISPOSE_BACKGROUND: {
			GifWord	      bg_idx = opaque->file->SBackGroundColor;
			GifColorType* rgb;

			if(opaque->file->SColorMap == NULL) return NULL;			     /* cant fill with bg color without global pallete */
			if(bg_idx < 0 || bg_idx >= opaque->file->SColorMap->ColorCount) return NULL; /* invalid color index */

			rgb = &opaque->file->SColorMap->Colors[bg_idx];

			for(j = 0; j < img->width; j++) {
				unsigned char* px = &row[j * 4];

				px[0] = rgb->Red;
				px[1] = rgb->Green;
				px[2] = rgb->Blue;
				px[3] = (!transp || bg_idx != transp_idx) * 255;
			}

			break;
		}

		// restore to previous canvas content is usually done by keeping a copy of the canvas
		// before this frame and then copying it back after processing.
		// however one way to implement this is just to... skip this frame.
		// we dont care about it since it doesnt affect the final frame.
		case DISPOSE_PREVIOUS: {
			if(i == FRAME_NUM) break; /* cannot skip if we ARE the nth frame */
			continue;
		}

		// DISPOSE_DO_NOT is basically no-op, leave the frame data as-is
		default:
			break;
		}

		if(yy < 0 || yy >= image_desc->Height) continue; /* frame is not in view */

		for(j = 0; j < img->width; j++) {
			unsigned char* px = &row[j * 4];
			int	       xx = j - image_desc->Left;
			GifByteType    idx;
			GifColorType*  rgb;

			if(xx < 0 || xx >= image_desc->Width) continue; /* frame is not in view */

			idx = image->RasterBits[image_desc->Width * yy + xx];

			if(idx >= color_map->ColorCount) return NULL; /* invalid color index */
			rgb   = &color_map->Colors[idx];
			px[0] = rgb->Red;
			px[1] = rgb->Green;
			px[2] = rgb->Blue;
			px[3] = (!transp || idx != transp_idx) * 255;
		}
	}

	opaque->y++;
	return row;
}

static void GIFDriverClose(void* ptr) {
	wvimage_t*   img    = ptr;
	gifopaque_t* opaque = img->opaque;
	int	     error;

	DGifCloseFile(opaque->file, &error);
	free(opaque);
	free(img);
}

wvimage_t* TryGIFDriver(const char* path) {
	wvimage_t*   img;
	gifopaque_t* opaque;
	GifFileType* file;
	int	     error;

	file = DGifOpenFileName(path, &error);
	if(!file) return NULL;

	img = AllocateImage();

	img->close = GIFDriverClose;
	img->read  = GIFDriverRead;

	img->fp	    = NULL;
	img->opaque = Allocate(opaque);

	if(DGifSlurp(file) == GIF_ERROR) {
		GIFDriverClose(img);
		return NULL;
	}

	/* Not sure if possible, check anyways */
	if(!file->ImageCount) {
		GIFDriverClose(img);
		return NULL;
	}

	opaque->y    = 0;
	opaque->file = file;

	img->name   = "GIF";
	img->width  = file->SWidth;
	img->height = file->SHeight;

	return img;
}
#endif
