#include <wvcommon.h>

typedef struct color {
	char* key;
	DWORD value;
} color_t;

typedef struct xpmopaque {
	int cpp;
	color_t* colors;
} xpmopaque_t;

static char* signature = "/* XPM */";

static unsigned char* XPMDriverRead(void* ptr){
	wvimage_t* img = ptr;
	xpmopaque_t* opaque = (xpmopaque_t*)img->opaque;
	unsigned char* row = malloc(img->width * 4);
	
	memset(row, 0, img->width * 4);

	return row;
}

static void XPMDriverClose(void* ptr){
	wvimage_t* img = ptr;
	xpmopaque_t* opaque = (xpmopaque_t*)img->opaque;

	if(opaque->colors != NULL) shfree(opaque->colors);
	free(opaque);

	fclose(img->fp);
	free(img);
}

wvimage_t* TryXPMDriver(const char* path){
	FILE* f = fopen(path, "rb");
	wvimage_t* img;
	xpmopaque_t* opaque;
	char sig[9];
	char* line;
	int lines = 0;
	int colors = 0;
	int dq = 0;
	if(f == NULL) return NULL;

	if(fread(sig, 1, 9, f) < 9){
		fclose(f);
		return NULL;
	}else if(memcmp(sig, signature, 9) != 0){
		fclose(f);
		return NULL;
	}

	fseek(f, 0, SEEK_SET);

	img = AllocateImage();

	img->name = "XPM";
	img->close = XPMDriverClose;
	img->read = XPMDriverRead;

	img->fp = f;
	img->opaque = Allocate(opaque);


	sh_new_strdup(opaque->colors);
	shdefault(opaque->colors, 0);

	line = malloc(1);
	line[0] = 0;

	while(1){
		char c;
		char* old = line;
		if(fread(&c, 1, 1, f) < 1){
			XPMDriverClose(img);
			free(line);
			return NULL;
		}

		if(c == '"'){
			dq = dq ? 0 : 1;
			if(!dq){
				if(lines == 0){
					char* indh = strchr(line, ' ');
					char* indc = indh == NULL ? NULL : strchr(indh + 1, ' ');
					char* indcpp = indc == NULL ? NULL : strchr(indc + 1, ' ');
					if(indh == NULL || indc == NULL || indcpp == NULL){
						XPMDriverClose(img);
						free(line);
						return NULL;
					}
					img->width = atoi(line);
					img->height = atoi(indh + 1);
					opaque->cpp = atoi(indcpp + 1);
					colors = atoi(indc + 1);
				}else if(lines <= colors){
					char* arg = line + opaque->cpp + 1;
					char* l = strchr(arg, ' ');
					int repeat = 1;
repeat:
					if(l != NULL && strlen(l) > 1){
						l[0] = 0;
						if(strcmp(arg, "c") == 0){
							char* c = l + 1;
							if(c[0] == '#'){
								char* str = malloc(opaque->cpp + 1);

								memcpy(str, line, opaque->cpp);
								str[opaque->cpp] = 0;

								if(strlen(c) == 4){
									/* TODO: parse color */
								}else if(strlen(c) == 7){
									/* TODO: parse color */
								}else{
									free(str);
									XPMDriverClose(img);
									free(line);
									return NULL;
								}
								free(str);
								repeat = 0;
							}else{
								XPMDriverClose(img);
								free(line);
								return NULL;
							}
						}
						if(repeat && (arg = strchr(l, ' ')) != NULL){
							arg++;
							goto repeat;
						}
					}else{
						XPMDriverClose(img);
						free(line);
						return NULL;
					}
				}
				
				lines++;

				free(line);
				line = malloc(1);
				line[0] = 0;
				if(lines > colors) break;
			}
		}else if(dq){
			line = malloc(strlen(old) + 2);
			strcpy(line, old);
			line[strlen(old)] = c;
			line[strlen(old) + 1] = 0;
			free(old);
		}
	}
	free(line);

	if(img->width == 0 || img->height == 0){
		XPMDriverClose(img);
		free(line);
		return NULL;
	}

	return img;
}
