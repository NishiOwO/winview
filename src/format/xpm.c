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
	char* line = malloc(1);
	int dq = 0;
	line[0] = 0;
	
	memset(row, 0, img->width * 4);
	while(1){
		char c;
		char* old = line;
		fread(&c, 1, 1, img->fp);

		if(c == '"'){
			dq = dq ? 0 : 1;
			if(!dq){
				int i;
				char* ch = malloc(opaque->cpp + 1);
				ch[opaque->cpp] = 0;

				for(i = 0; i < strlen(line) && i < opaque->cpp * img->width; i += opaque->cpp){
					DWORD c;
					int j;
					memcpy(ch, line + i, opaque->cpp);
					c = shget(opaque->colors, ch);
					for(j = 0; j < 4; j++){
						row[i / opaque->cpp * 4 + j] = (c >> 24) & 0xff;
						c = c << 8;
					}
				}

				free(ch);

				free(line);
				line = malloc(1);
				line[0] = 0;
				break;
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
								DWORD r, g, b, a = 255;
								DWORD rgba;

								memcpy(str, line, opaque->cpp);
								str[opaque->cpp] = 0;

								if(strlen(c) == 4){
									r = ParseHex(c + 1, 1);
									g = ParseHex(c + 2, 1);
									b = ParseHex(c + 3, 1);

									r = r | (r << 4);
									g = g | (g << 4);
									b = b | (b << 4);
								}else if(strlen(c) == 7){
									r = ParseHex(c + 1, 2);
									g = ParseHex(c + 3, 2);
									b = ParseHex(c + 5, 2);
								}else{
									free(str);
									XPMDriverClose(img);
									free(line);
									return NULL;
								}

								rgba = CreateRGBA(r, g, b, a);
								shput(opaque->colors, str, rgba);

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
