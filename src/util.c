#include <wvcommon.h>

typedef struct brushcache {
	int    r;
	int    g;
	int    b;
	HBRUSH brush;
} brushcache_t;

typedef struct dibcache {
	char*	key;
	HBITMAP value;
	int	width;
	int	height;
	char*	status;
	time_t	mtime;
} dibcache_t;

void ShowBitmapSize(HDC hdc, const char* name, int x, int y, int w, int h) {
	HBITMAP hBitmap = LoadBitmap(hInst, name);
	BITMAP	bmp;
	HDC	hmdc;
	GetObject(hBitmap, sizeof(bmp), &bmp);
	hmdc = CreateCompatibleDC(hdc);
	SelectObject(hmdc, hBitmap);
	if(w == 0 && h == 0) {
		StretchBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hmdc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	} else {
		StretchBlt(hdc, x, y, w, h, hmdc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	}
	DeleteDC(hmdc);
	DeleteObject(hBitmap);
}

static brushcache_t* solidcache = NULL;

HBRUSH GetSolidBrushCached(int r, int g, int b) {
	int	     i;
	brushcache_t c;

	for(i = 0; i < arrlen(solidcache); i++) {
		if(solidcache[i].r == r && solidcache[i].g == g && solidcache[i].b == b) {
			return solidcache[i].brush;
		}
	}

	c.r	= r;
	c.g	= g;
	c.b	= b;
	c.brush = CreateSolidBrush(RGB(r, g, b));

	arrput(solidcache, c);

	return c.brush;
}

static brushcache_t* hatchcache = NULL;

HBRUSH GetHatchBrushCached(int r, int g, int b) {
	int	     i;
	brushcache_t c;

	for(i = 0; i < arrlen(hatchcache); i++) {
		if(hatchcache[i].r == r && hatchcache[i].g == g && hatchcache[i].b == b) {
			return hatchcache[i].brush;
		}
	}

	c.r	= r;
	c.g	= g;
	c.b	= b;
	c.brush = CreateHatchBrush(HS_DIAGCROSS, RGB(r, g, b));

	arrput(hatchcache, c);

	return c.brush;
}

void SetProgress(int value) { PostMessage(hProgress, PBM_SETPOS, value, 0); }

void SetStatus(const char* text) { PostMessage(hMain, WM_SB_SETTEXT, 0, (LPARAM)text); }

static char txt[256];

void ReadyStatus(void) {
	sprintf(txt, "Ready - Version %s", wvversion);
	SetStatus(txt);
}

void AdjustImageWindowSize(void) {
	RECT   r;
	int    style;
	int    ww, wh, wx, wy;
	double scale;
#ifdef REASPECT_BY_SCALE
	double wws, whs;
#endif
	HWND deskwnd = GetDesktopWindow();
	int  dw, dh;
	int  d = 0;

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
	if(wws > whs) {
#else
	if(ww > wh) {
#endif
		scale = (double)ww / ImageWidth;
	} else {
		scale = (double)wh / ImageHeight;
	}

	SetRect(&r, 0, 0, ImageWidth * scale, ImageHeight * scale);

readjust:
	style = (DWORD)GetWindowLongPtr(hImage, GWL_STYLE);
	AdjustWindowRect(&r, style, FALSE);

	if((wy + (r.bottom - r.top)) > dh) {
		scale = (double)(dh - wy - d) / ImageHeight;
		SetRect(&r, 0, 0, ImageWidth * scale, ImageHeight * scale);
		d += 1;
		goto readjust;
	}

	SetWindowPos(hImage, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOMOVE);
}

void FillRectRotated(HDC dc, RECT* r, double angle, HBRUSH brush) {
	double rad = angle / 180 * M_PI;
	int    y, x;
	for(y = 0; y < r->bottom - r->top; y++) {
		for(x = 0; x < r->right - r->left; x++) {
			RECT   rc;
			double px = x - (r->right - r->left) / 2;
			double py = y - (r->bottom - r->top) / 2;
			double rx = r->left + (px * cos(rad) - py * sin(rad));
			double ry = r->top + (px * sin(rad) + py * cos(rad));

			rc.left	  = rx;
			rc.top	  = ry;
			rc.right  = CeilNumber(rx + 1);
			rc.bottom = CeilNumber(ry + 1);

			FillRect(dc, &rc, brush);
		}
	}
}
