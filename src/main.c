#include <wvcommon.h>
#include <wvresource.h>

HINSTANCE hInst;
HFONT fixedsys, bifixedsys;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	if(msg == WM_COMMAND){
		if(LOWORD(wp) == IDM_ABOUT_VERSION) DialogBox(hInst, "WVVERSION", hWnd, (DLGPROC)VersionDialog);
	}else if(msg == WM_CLOSE){
		DestroyWindow(hWnd);
	}else if(msg == WM_DESTROY){
		PostQuitMessage(0);
	}else{
		return DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}

BOOL InitClass(void){
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(hInst, "WVLOGO");
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wc.lpszMenuName = "wvmenu";
	wc.lpszClassName = "winview";
	wc.hIconSm = LoadIcon(hInst, "WVLOGO");
	return RegisterClassEx(&wc);
}

BOOL InitWindow(int nCmdShow){
	HWND hWnd = CreateWindow("winview", "WinView", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, 0, hInst, NULL);
	HDC dc;

	if(hWnd == NULL) return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

int WINAPI WinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst, LPSTR lpsCmdLine, int nCmdShow){
	BOOL bret;
	MSG msg;

	hInst = hCurInst;
	if(!InitClass()){
		return FALSE;
	}

	fixedsys = CreateFont(14, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, NULL);
	bifixedsys = CreateFont(14*5, 0, 0, 0, FW_BOLD, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, NULL);

	if(!InitWindow(nCmdShow)){
		return FALSE;
	}

	while((bret = GetMessage(&msg, NULL, 0, 0)) != 0){
		if(bret == -1) break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
