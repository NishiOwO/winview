#include <wvcommon.h>

typedef struct brushcache {
	int r;
	int g;
	int b;
	HBRUSH brush;
} brushcache_t;

typedef struct dibcache {
	char* key;
	HBITMAP value;
	int width;
	int height;
	char* status;
	time_t mtime;
} dibcache_t;

void ShowBitmapSize(HDC hdc, const char* name, int x, int y, int w, int h) {
	HBITMAP hBitmap = LoadBitmap(hInst, name);
	BITMAP bmp;
	HDC hmdc;
	GetObject(hBitmap, sizeof(bmp), &bmp);
	hmdc = CreateCompatibleDC(hdc);
	SelectObject(hmdc, hBitmap);
	if(w == 0 && h == 0) {
		StretchBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hmdc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	}else{
		StretchBlt(hdc, x, y, w, h, hmdc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	}
	DeleteDC(hmdc);
	DeleteObject(hBitmap);
}

static brushcache_t* solidcache = NULL;
HBRUSH GetSolidBrushCached(int r, int g, int b){
	int i; \
	brushcache_t c;

	for(i = 0; i < arrlen(solidcache); i++){
		if(solidcache[i].r == r && solidcache[i].g == g && solidcache[i].b == b){
			return solidcache[i].brush;
		}
	}

	c.r = r;
	c.g = g;
	c.b = b;
	c.brush = CreateSolidBrush(RGB(r, g, b));

	arrput(solidcache, c);

	return c.brush;
}

static brushcache_t* hatchcache = NULL;
HBRUSH GetHatchBrushCached(int r, int g, int b){
	int i;
	brushcache_t c;

	for(i = 0; i < arrlen(hatchcache); i++){
		if(hatchcache[i].r == r && hatchcache[i].g == g && hatchcache[i].b == b){
			return hatchcache[i].brush;
		}
	}

	c.r = r;
	c.g = g;
	c.b = b;
	c.brush = CreateHatchBrush(HS_DIAGCROSS, RGB(r, g, b));

	arrput(hatchcache, c);

	return c.brush;
}

void SetProgress(int value){
	PostMessage(hProgress, PBM_SETPOS, value, 0);
}

void SetStatus(const char* text){
	PostMessage(hMain, WM_SB_SETTEXT, 0, (LPARAM)text);
}

char* DuplicateString(const char* str){
	char* r = malloc(strlen(str) + 1);
	strcpy(r, str);
	return r;
}

wvimage_t* AllocateImage(void){
	wvimage_t* r = malloc(sizeof(*r));
	memset(r, 0, sizeof(*r));

	return r;
}

HANDLE CreateWinViewMutex(void){
	return CreateEvent(NULL, FALSE, TRUE, NULL);
}

void DestroyWinViewMutex(HANDLE mutex){
	CloseHandle(mutex);
}

void LockWinViewMutex(HANDLE mutex){
	WaitForSingleObject(mutex, INFINITE);
}

void UnlockWinViewMutex(HANDLE mutex){
	SetEvent(mutex);
}

void CreateWinViewBitmap(int w, int h, HBITMAP* bmp, RGBQUAD** quad){
	HDC dc = GetDC(NULL);
	BITMAPINFOHEADER bmih;

	bmih.biSize = sizeof(bmih);
	bmih.biWidth = w;
	bmih.biHeight = -(LONG)h;
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	*bmp = CreateDIBSection(dc, (BITMAPINFO*)&bmih, DIB_RGB_COLORS, (void**)quad, NULL, (DWORD)0);
	ReleaseDC(NULL, dc);
}

static char txt[256];
void ReadyStatus(void){
	sprintf(txt, "Ready - Version %s", wvversion);
	SetStatus(txt);
}

void AdjustImageWindowSize(void){
	RECT r;
	int style;
	int ww, wh, wx, wy;
	double scale;
#ifdef REASPECT_BY_SCALE
	double wws, whs;
#endif
	HWND deskwnd = GetDesktopWindow();
	int dw, dh;
	int d = 0;

	GetClientRect(deskwnd, &r);
	dw = r.right - r.left;
	dh = r.bottom - r.top;

	GetClientRect(hImage, &r);

	ww = r.right - r.left;
	wh = r.bottom - r.top;

	GetWindowRect(hImage, &r);
	wx = r.left;
	wy = r.top;

#ifdef REASPECT_BY_SCALE
	wws = (double)ww / ImageWidth;
	whs = (double)wh / ImageHeight;
	if(wws > whs){
#else
	if(ww > wh){
#endif
		scale = (double)ww / ImageWidth;
	}else{
		scale = (double)wh / ImageHeight;
	}

	SetRect(&r, 0, 0, ImageWidth * scale, ImageHeight * scale);

readjust:
	style = (DWORD)GetWindowLongPtr(hImage, GWL_STYLE);
	AdjustWindowRect(&r, style, FALSE);

	if((wy + (r.bottom - r.top)) > dh){
		scale = (double)(dh - wy - d) / ImageHeight;
		SetRect(&r, 0, 0, ImageWidth * scale, ImageHeight * scale);
		d += 1;
		goto readjust;
	}

	SetWindowPos(hImage, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOMOVE);
}

static dibcache_t* dibcache = NULL;
HBITMAP GetDIBCache(const char* path, int* width, int* height, char* status){
	dibcache_t c;
	struct _stat s;
	if(shgeti(dibcache, path) == -1) return NULL;
	if(_stat(path, &s) != 0){
		DestroyDIBCache(path);
		return NULL;
	}

	c = shgets(dibcache, path);
	if(c.mtime != s.st_mtime){
		DestroyDIBCache(path);
		return NULL;
	}

	*width = c.width;
	*height = c.height;
	strcpy(status, c.status);

	return c.value;
}

void InitDIBCache(void){
	sh_new_strdup(dibcache);
	shdefault(dibcache, NULL);
}

void SaveDIBCache(const char* path, HBITMAP bmp, int width, int height, char* status){
	dibcache_t c;
	struct _stat s;

	_stat(path, &s);

	c.key = (char*)path;
	c.value = bmp;
	c.width = width;
	c.height = height;
	c.status = DuplicateString(status);
	c.mtime = s.st_mtime;

	shputs(dibcache, c);
}

void DestroyDIBCache(const char* path){
	dibcache_t c;
	if(shgeti(dibcache, path) == -1) return;
	c = shgets(dibcache, path);

	DeleteObject(c.value);
	free(c.status);

	shdel(dibcache, path);
}
