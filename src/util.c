#include <wvcommon.h>

typedef struct brushcache {
	int r;
	int g;
	int b;
	HBRUSH brush;
} brushcache_t;

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
