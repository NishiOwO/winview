#include <wvcommon.h>
#include <wvresource.h>

#define USE_STAR

#ifdef USE_STAR
#define FPS 30

typedef struct star {
	double x;
	double y;
	double c;
	HBRUSH brush;
} star_t;

static star_t stars[256];
#endif

const char* wvversion = "0.0";

static const char* texts[] = {
    "https://github.com/nishiowo/winview",			   /**/
    "",								   /**/
    "-Version %s",						   /**/
    "-Copyright 2025 by WinView developers - All Rights Reserved", /**/
    "",								   /**/
    "+WinView",							   /**/
    NULL							   /**/
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
		SetWindowPos(hWnd, NULL, 0, 0, 440, 440 / 4 * 3, SWP_NOMOVE);

#ifdef USE_STAR
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

		SetTimer(hWnd, 100, 1000 / FPS, NULL);
#endif
	} else if(msg == WM_ERASEBKGND) {
#ifdef USE_STAR
	} else if(msg == WM_TIMER) {
		InvalidateRect(hWnd, NULL, FALSE);
#endif
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

#ifdef USE_STAR
		for(i = 0; i < sizeof(stars) / sizeof(stars[0]); i++) {
			r.left	 = stars[i].x;
			r.top	 = stars[i].y;
			r.right	 = r.left + 2;
			r.bottom = r.top + 2;

			FillRect(dc, &r, stars[i].brush);

			stars[i].x -= (double)ww / FPS * stars[i].c / 4;
			if(stars[i].x <= -2) stars[i].x = ww + stars[i].x + 2;
		}
#else
		/* 200x138 */
		for(y = 0; y < (wh / 138) + 1; y++) {
			for(x = 0; x < (ww / 200) + 1; x++) {
				ShowBitmapSize(dc, "WVVERSIONBMP", x * 200, y * 138, 200, 138);
			}
		}

		y = 10;
		for(i = 0; texts[i] != NULL; i++) {
			y += texts[i][0] == '+' ? 14 * 7.5 : 14;
		}

		r.left	 = 0;
		r.right	 = ww;
		r.top	 = wh - y;
		r.bottom = wh;

		FillRect(dc, &r, GetSolidBrushCached(0, 0, 0));
#endif

		y = 5;
		for(i = 0; texts[i] != NULL; i++) {
			int  l = strlen(texts[i]) - ((texts[i][0] == '+' || texts[i][0] == '-') ? 1 : 0);
			char t[512];
			if(texts[i][0] == '-') {
				SetTextColor(dc, RGB(0xa0, 0xa0, 0));
			} else {
				SetTextColor(dc, RGB(0xff, 0xff, 0xff));
			}

			if(texts[i][0] == '+') {
				SelectObject(dc, bifixedsys);
			} else {
				SelectObject(dc, fixedsys);
			}

			sprintf(t, texts[i] + ((texts[i][0] == '+' || texts[i][0] == '-') ? 1 : 0), wvversion);

			if(texts[i][0] == '+') savy = y;
			TextOut(dc, ww / 2, wh - y, t, strlen(t));
			y += texts[i][0] == '+' ? 14 * 7.5 : 14;
		}

#ifdef BETA
		SetTextColor(dc, RGB(0xe0, 0, 0));
		SelectObject(dc, betafont);

		TextOut(dc, ww / 2, wh - savy - (14 * 7.5 - 14 * 3) / 2, "BETA VERSION", 4 + 1 + 7);
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
