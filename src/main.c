#include <windows.h>

#include <wvcommon.h>

HINSTANCE hInst;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	return DefWindowProc(hWnd, msg, wp, lp);
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
