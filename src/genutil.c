#include <wvcommon.h>

char* DuplicateString(const char* str) {
	char* r = malloc(strlen(str) + 1);
	strcpy(r, str);
	return r;
}

wvimage_t* AllocateImage(void) {
	wvimage_t* r = malloc(sizeof(*r));
	memset(r, 0, sizeof(*r));

	return r;
}

HANDLE CreateWinViewMutex(void) { return CreateEvent(NULL, FALSE, TRUE, NULL); }

void DestroyWinViewMutex(HANDLE mutex) { CloseHandle(mutex); }

void LockWinViewMutex(HANDLE mutex) { WaitForSingleObject(mutex, INFINITE); }

void UnlockWinViewMutex(HANDLE mutex) { SetEvent(mutex); }

void CreateWinViewBitmap(int w, int h, HBITMAP* bmp, RGBQUAD** quad) {
	HDC		 dc = GetDC(NULL);
	BITMAPINFOHEADER bmih;

	bmih.biSize	     = sizeof(bmih);
	bmih.biWidth	     = w;
	bmih.biHeight	     = -(LONG)h;
	bmih.biPlanes	     = 1;
	bmih.biBitCount	     = 32;
	bmih.biCompression   = BI_RGB;
	bmih.biSizeImage     = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed	     = 0;
	bmih.biClrImportant  = 0;

	*bmp = CreateDIBSection(dc, (BITMAPINFO*)&bmih, DIB_RGB_COLORS, (void**)quad, NULL, (DWORD)0);
	ReleaseDC(NULL, dc);
}

DWORD ParseHex(const char* str, int len) {
	int   i;
	DWORD n = 0;
	for(i = 0; i < len; i++) {
		DWORD c = 0;
		if('0' <= str[i] && str[i] <= '9') {
			c = str[i] - '0';
		} else if('a' <= str[i] && str[i] <= 'f') {
			c = str[i] - 'a' + 10;
		} else if('A' <= str[i] && str[i] <= 'F') {
			c = str[i] - 'A' + 10;
		}

		n = n << 4;
		n = n | c;
	}
	return n;
}

WORD ReadAsLittleWORD(unsigned char* ptr, int start) {
	WORD r = 0;
	int  i;
	for(i = 1; i >= 0; i--) {
		r = r << 8;
		r = r | ptr[start + i];
	}

	return r;
}

DWORD ReadAsLittleDWORD(unsigned char* ptr, int start) {
	DWORD r = 0;
	int   i;
	for(i = 3; i >= 0; i--) {
		r = r << 8;
		r = r | ptr[start + i];
	}

	return r;
}

WORD ReadAsBigWORD(unsigned char* ptr, int start) {
	WORD r = 0;
	int  i;
	for(i = 0; i < 2; i++) {
		r = r << 8;
		r = r | ptr[start + i];
	}

	return r;
}

DWORD ReadAsBigDWORD(unsigned char* ptr, int start) {
	DWORD r = 0;
	int   i;
	for(i = 0; i < 4; i++) {
		r = r << 8;
		r = r | ptr[start + i];
	}

	return r;
}

BOOL CompareStringSafely(const char* target, const char* comp) {
	int i;
	for(i = 0; comp[i] != 0; i++) {
		if(target[i] == 0) return FALSE;
		if(target[i] != comp[i]) return FALSE;
	}

	return TRUE;
}

double CeilNumber(double n) {
	if(n == (int)n) return n;
	if(n > 0) return 1 + (int)n;
	if(n < 0) return -1 + (int)n;
	return 0;
}
