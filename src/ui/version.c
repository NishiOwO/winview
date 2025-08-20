#include <wvcommon.h>
#include <wvresource.h>

const char* wvversion = "0.0";

static const char* texts[] = {"https://github.com/nishiowo/winview", "", "-Version %s", "-Copyright 2025 by WinView developers - All Rights Reserved", "", "+WinView", NULL};

LRESULT CALLBACK VersionDialog(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg == WM_CLOSE) {
		EndDialog(hWnd, 0);
	} else if(msg == WM_KEYDOWN) {
		if(wp == 'Q' || wp == VK_ESCAPE) EndDialog(hWnd, 0);
	} else if(msg == WM_INITDIALOG) {
		RECT r;
		GetWindowRect(hWnd, &r);
		SetWindowPos(hWnd, NULL, 0, 0, 440, 440 / 4 * 3, SWP_NOMOVE);
	} else if(msg == WM_PAINT) {
		PAINTSTRUCT ps;
		RECT	    r;
		HDC	    dc = BeginPaint(hWnd, &ps);
		int	    ww, wh;
		int	    x, y;
		int	    i;
		int	    vw;

		GetClientRect(hWnd, &r);
		wh = r.bottom - r.top;
		ww = r.right - r.left;

		SetTextAlign(dc, TA_CENTER | TA_BOTTOM);
		SetBkMode(dc, TRANSPARENT);

		/* 200x138 */
		for(y = 0; y < (wh / 138) + 1; y++) {
			for(x = 0; x < (ww / 200) + 1; x++) {
				ShowBitmapSize(dc, "WVVERSIONBMP", x * 200, y * 138, 200, 138);
			}
		}

		y = 10;
		for(i = 0; texts[i] != NULL; i++) {
			y += texts[i][0] == '+' ? 14 * 5 : 14;
		}

		r.left	 = 0;
		r.right	 = ww;
		r.top	 = wh - y;
		r.bottom = wh;

		FillRect(dc, &r, GetSolidBrushCached(0, 0, 0));

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

			TextOut(dc, ww / 2, wh - y, t, strlen(t));
			y += texts[i][0] == '+' ? 14 * 5 : 14;
		}

		EndPaint(hWnd, &ps);
	} else {
		return FALSE;
	}
	return TRUE;
}
