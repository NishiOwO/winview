#include <wvcommon.h>
#include <wvresource.h>

HWND hStatus, hProgress, hListbox;
HINSTANCE hInst;
HFONT fixedsys, bifixedsys;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	if(msg == WM_COMMAND){
		if(LOWORD(wp) == IDM_FILE_QUIT) DestroyWindow(hWnd);
		if(LOWORD(wp) == IDM_ABOUT_VERSION) DialogBox(hInst, "WVVERSION", hWnd, (DLGPROC)VersionDialog);
	}else if(msg == WM_CLOSE){
		DestroyWindow(hWnd);
	}else if(msg == WM_DESTROY){
		PostQuitMessage(0);
	}else if(msg == WM_SIZE){
		int parts[2];
		RECT r, r2;
		SendMessage(hStatus, WM_SIZE, wp, lp);

		parts[0] = LOWORD(lp) - 100;
		parts[1] = -1;

		SendMessage(hStatus, SB_SETPARTS, 2, (LPARAM)parts);

		SendMessage(hStatus, SB_GETRECT, 1, (LPARAM)&r);

		SetWindowPos(hProgress, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, 0);

		GetClientRect(hStatus, &r2);

		GetClientRect(hWnd, &r);
		r.bottom -= r2.bottom - r2.top;
		r.right -= 100;

		SetWindowPos(hListbox, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, 0);
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
	HWND hWnd = CreateWindow("winview", "WinView", WS_OVERLAPPEDWINDOW ^ (WS_MAXIMIZEBOX | WS_THICKFRAME), CW_USEDEFAULT, CW_USEDEFAULT, 440, 440 / 4 * 3, NULL, 0, hInst, NULL);
	HDC dc;
	int parts[2];
	char txt[256];

	if(hWnd == NULL) return FALSE;

	parts[0] = 0;
	parts[1] = 100;
	hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | CCS_BOTTOM, 0, 0, 0, 0, hWnd, (HMENU)IDM_STATUS, hInst, NULL);
	SendMessage(hStatus, SB_SIMPLE, FALSE, 0);
	SendMessage(hStatus, SB_SETPARTS, 2, (LPARAM)parts);

	hProgress = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 0, 0, 0, 0, hStatus, (HMENU)IDM_STATUS_PROGRESS, hInst, NULL);
	SendMessage(hProgress, PBM_SETRANGE, 0, MAKELONG(0, 100));
	SendMessage(hProgress, PBM_SETPOS, 0, 0);

	sprintf(txt, "Ready - Version %s", wvversion);
	SetStatus(txt);

	hListbox = CreateWindow("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, 0, 0, 0, 0, hWnd, (HMENU)IDM_LISTBOX, hInst, NULL);

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

	InitCommonControls();

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
