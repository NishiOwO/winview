#include <wvcommon.h>
#include <wvresource.h>

#define FPS 30

const char* wvversion = "0.0";

LRESULT CALLBACK VersionDialog(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	if(msg == WM_CLOSE){
		EndDialog(hWnd, 0);
	}else if(msg == WM_INITDIALOG){
		SetTimer(hWnd, 100, 1000/FPS, NULL);
	}else if(msg == WM_PAINT){
		PAINTSTRUCT ps;
		RECT r;
		HDC dc = BeginPaint(hWnd, &ps);

		GetClientRect(hWnd, &r);
		r.bottom = r.bottom - r.top;
		r.right = r.right - r.left;

		r.left = 0;
		r.top = 0;

		FillRect(dc, &r, GetBrushCached(0, 0, 0));
		EndPaint(hWnd, &ps);
	}else if(msg == WM_TIMER){
		if(wp == 100){
			InvalidateRect(hWnd, NULL, FALSE);
		}
	}else{
		return FALSE;
	}
	return TRUE;
}
