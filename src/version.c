#include <wvcommon.h>
#include <wvresource.h>

#define USE_STAR

const char* wvversion = "0.0";

#ifdef USE_STAR
#define STARS 200
#define FPS 30

typedef struct star {
	double x;
	double y;
	double speed;
	HBRUSH brush;
} star_t;

static star_t stars[STARS];
#endif
static const char* texts[] = {
	"https://github.com/nishiowo/winview",
	"",
	"-Version %s",
	"-Copyright 2025 by WinView developers - All Rights Reserved",
	"",
	"+WinView",
	NULL
};

LRESULT CALLBACK VersionDialog(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	if(msg == WM_CLOSE){
		EndDialog(hWnd, 0);
	}else if(msg == WM_INITDIALOG){
#ifdef USE_STAR
		RECT r;
		int ww, wh;
		int i;

		GetClientRect(hWnd, &r);
		wh = r.bottom - r.top;
		ww = r.right - r.left;

		for(i = 0; i < STARS; i++){
			int c = rand() % 256;
			stars[i].x = rand() % ww;
			stars[i].y = rand() % wh;

			stars[i].speed = (double)c / 255;
			stars[i].brush = GetSolidBrushCached(c, c, c);
		}

		SetTimer(hWnd, 100, 1000/FPS, NULL);
#endif
	}else if(msg == WM_PAINT){
		PAINTSTRUCT ps;
		RECT r;
		HDC dc = BeginPaint(hWnd, &ps);
		int ww, wh;
		int x, y;
		int i;

		GetClientRect(hWnd, &r);
		wh = r.bottom - r.top;
		ww = r.right - r.left;

		SetTextAlign(dc, TA_CENTER | TA_BOTTOM);
		SetBkMode(dc, TRANSPARENT);

#ifdef USE_STAR
		r.left = 0;
		r.right = ww;
		r.top = 0;
		r.bottom = wh;
		FillRect(dc, &r, GetSolidBrushCached(0, 0, 0));
		for(i = 0; i < STARS; i++){
			r.left = stars[i].x;
			r.top = stars[i].y;
			r.right = r.left + 2;
			r.bottom = r.top + 2;
			FillRect(dc, &r, stars[i].brush);

			stars[i].x -= (double)ww / FPS / 4 * stars[i].speed;
			if(stars[i].x <= 0) stars[i].x = ww + stars[i].x;
		}
#else

		/* 200x138 */
		for(y = 0; y < (wh / 138) + 1; y++){
			for(x = 0; x < (ww / 200) + 1; x++){
				ShowBitmapSize(dc, "WVVERSIONBMP", x * 200, y * 138, 200, 138);
			}
		}

		y = 0;
		for(i = 0; texts[i] != NULL; i++)y += texts[i][0] == '+' ? 14*5 : 14;

		r.left = 0;
		r.right = ww;
		r.top = wh - y;
		r.bottom = wh;

		FillRect(dc, &r, GetSolidBrushCached(0, 0, 0));
#endif

		y = 0;
		for(i = 0; texts[i] != NULL; i++){
			int l = strlen(texts[i]) - ((texts[i][0] == '+' || texts[i][0] == '-') ? 1 : 0);
			char t[512];
			if(texts[i][0] == '-'){
				SetTextColor(dc, RGB(0xa0, 0xa0, 0));
			}else{
				SetTextColor(dc, RGB(0xff, 0xff, 0xff));
			}

			if(texts[i][0] == '+'){
				SelectObject(dc, bifixedsys);
			}else{
				SelectObject(dc, fixedsys);
			}

			sprintf(t, texts[i] + ((texts[i][0] == '+' || texts[i][0] == '-') ? 1 : 0), wvversion);

			TextOut(dc, ww / 2, wh - y, t, strlen(t));
			y += texts[i][0] == '+' ? 14*5 : 14;
		}

		EndPaint(hWnd, &ps);
	}else if(msg == WM_TIMER){
#ifdef USE_STAR
		if(wp == 100){
			InvalidateRect(hWnd, NULL, FALSE);
		}
#endif
	}else{
		return FALSE;
	}
	return TRUE;
}
