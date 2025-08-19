#include <wvcommon.h>
#include <wvresource.h>

#include <zlib.h>
#include <png.h>
#include <jversion.h>
#include <tiffio.h>
#include <gif_lib.h>

LRESULT CALLBACK CreditsDialog(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	if(msg == WM_CLOSE){
		EndDialog(hWnd, 0);
	}else if(msg == WM_COMMAND){
		if(LOWORD(wp) == 100) EndDialog(hWnd, 0);
	}else if(msg == WM_INITDIALOG){
		RECT r;
		HWND text;
		char txt[1024];

		GetWindowRect(hWnd, &r);
		SetWindowPos(hWnd, NULL, 0, 0, 330, 330 / 4 * 3, SWP_NOMOVE);

		GetClientRect(hWnd, &r);

		CreateWindow("BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, (r.right - r.left) - 100 - 5, (r.bottom - r.top) - 25 - 5, 100, 25, hWnd, (HMENU)100, hInst, NULL);
		txt[0] = 0;

		sprintf(txt + strlen(txt), "Thanks to:\r\n");
		sprintf(txt + strlen(txt), "  zlib %s\r\n", ZLIB_VERSION);
		sprintf(txt + strlen(txt), "  libpng %s\r\n", PNG_LIBPNG_VER_STRING);
		sprintf(txt + strlen(txt), "  libjpeg %s\r\n", JVERSION);
		sprintf(txt + strlen(txt), "  libtiff %s\r\n", TIFFLIB_VERSION_STR_MAJ_MIN_MIC);
		sprintf(txt + strlen(txt), "  giflib %d.%d.%d\r\n", GIFLIB_MAJOR, GIFLIB_MINOR, GIFLIB_RELEASE);

		text = CreateWindow("EDIT", txt, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY, 5, 5, (r.right - r.left) - 10, (r.bottom - r.top) - 15 - 25, hWnd, 0, hInst, NULL);
	}else{
		return FALSE;
	}
	return TRUE;
}
