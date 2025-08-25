#include <wvcommon.h>
#include <wvresource.h>

#define FPS 30

typedef struct star {
	double x;
	double y;
	double c;
	HBRUSH brush;
} star_t;

static star_t stars[256];
static double passed = 0;

const char* wvversion = "0.0";

static const char* texts[] = {
    "https://github.com/nishiowo/winview",   /**/
    "",					     /**/
    "-All Rights Reserved",		     /**/
    "-Copyright 2025 by WinView developers", /**/
    "",					     /**/
    "Version %s",			     /**/
    "+WinView",				     /**/
    NULL				     /**/
};

static HBITMAP bufferbmp;
static HDC     bufferdc;

LRESULT CALLBACK VersionDialog(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg == WM_CLOSE) {
		EndDialog(hWnd, 0);
	} else if(msg == WM_KEYDOWN) {
		if(wp == 'Q' || wp == VK_ESCAPE) EndDialog(hWnd, 0);
	} else if(msg == WM_DESTROY) {
		DeleteDC(bufferdc);
		DeleteObject(bufferbmp);
	} else if(msg == WM_INITDIALOG) {
		RECT r;
		HDC  dc;
		int  i;

		GetWindowRect(hWnd, &r);
		SetWindowPos(hWnd, NULL, 0, 0, 550, 550 / 2, SWP_NOMOVE);

		dc = GetDC(hWnd);

		GetClientRect(hWnd, &r);

		bufferbmp = CreateCompatibleBitmap(dc, r.right - r.left, r.bottom - r.top);
		bufferdc  = CreateCompatibleDC(dc);

		SelectObject(bufferdc, bufferbmp);

		ReleaseDC(hWnd, dc);

		for(i = 0; i < sizeof(stars) / sizeof(stars[0]); i++) {
			stars[i].x     = rand() % (r.right - r.left);
			stars[i].y     = rand() % (r.bottom - r.top);
			stars[i].c     = rand() % 256;
			stars[i].brush = GetSolidBrushCached(stars[i].c, stars[i].c, stars[i].c);

			stars[i].c /= 255;
		}

		passed = 0;

		SetTimer(hWnd, 100, 1000 / FPS, NULL);
	} else if(msg == WM_ERASEBKGND) {
	} else if(msg == WM_TIMER) {
		InvalidateRect(hWnd, NULL, FALSE);
	} else if(msg == WM_PAINT) {
		PAINTSTRUCT ps;
		RECT	    r;
		HDC	    dc = bufferdc;
		int	    ww, wh;
		int	    x, y;
		int	    i;
		int	    vw;
		int	    savy;

		GetClientRect(hWnd, &r);
		wh = r.bottom - r.top;
		ww = r.right - r.left;

		SetTextAlign(dc, TA_CENTER | TA_BOTTOM);
		SetBkMode(dc, TRANSPARENT);

		PatBlt(dc, 0, 0, ww, wh, BLACKNESS);

		for(i = 0; i < sizeof(stars) / sizeof(stars[0]); i++) {
			r.left	 = stars[i].x;
			r.top	 = stars[i].y;
			r.right	 = r.left + 2;
			r.bottom = r.top + 2;

			FillRect(dc, &r, stars[i].brush);

			stars[i].x -= (double)ww / FPS * stars[i].c / 4;
			if(stars[i].x <= -2) stars[i].x = ww + stars[i].x + 2;
		}

		passed += 1.0 / FPS;

		y = 5;
		for(i = 0; texts[i] != NULL; i++) {
			int    l = strlen(texts[i]) - ((texts[i][0] == '+' || texts[i][0] == '-') ? 1 : 0);
			char   t[512];
			int    r, g, b;
			double fsz = texts[i][0] == '+' ? 14 * 4 : 14;

			if(texts[i][0] == '-') {
				r = 0xa0;
				g = 0xa0;
				b = 0;
			} else {
				r = 0xff;
				g = 0xff;
				b = 0xff;
			}

			sprintf(t, texts[i] + ((texts[i][0] == '+' || texts[i][0] == '-') ? 1 : 0), wvversion);
			if(texts[i][0] == '+') savy = y;

			Draw8x8Text(dc, t, 0, ww / 2, wh - y - fsz / 2, fsz / 8, r, g, b);
			y += fsz;
		}

#ifdef BETA
		Draw8x8Text(dc, "BETA VERSION", sin(passed * M_PI * 1) * 11.25, ww / 2, wh - savy - (14 * 4) / 2, 22.0 / 8, 0xff, 0, 0);
#endif

		dc = BeginPaint(hWnd, &ps);
		SetStretchBltMode(dc, HALFTONE);
		StretchBlt(dc, 0, 0, ww, wh, bufferdc, 0, 0, ww, wh, SRCCOPY);
		EndPaint(hWnd, &ps);
	} else {
		return FALSE;
	}
	return TRUE;
}
