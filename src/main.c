#include <wvcommon.h>
#include <wvresource.h>

typedef struct button {
	const char* text;
	int id;
	HMENU menu;
	HWND wnd;
} button_t;

button_t buttons[] = {
	{"&Next", 'N', 0, NULL},
	{"&Prev", 'P', 0, NULL},
	{"&Load", 'L', 0, NULL},
	{"&Delete", 'D', 0, NULL}
};

HWND hMain, hStatus, hProgress, hListbox;
HINSTANCE hInst;
HFONT fixedsys, bifixedsys;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	if(msg == WM_COMMAND){
		int m = LOWORD(wp);
		if(m == IDM_FILE_QUIT) DestroyWindow(hWnd);
		if(m == IDM_ABOUT_VERSION) DialogBox(hInst, "WVVERSION", hWnd, (DLGPROC)VersionDialog);
		if(m == IDM_LISTBOX){
			if(HIWORD(wp) == LBN_DBLCLK){
				ShowImage(SendMessage(hListbox, LB_GETCURSEL, 0, 0));
			}
		}
		if(m >= IDM_BUTTONS && m < (IDM_BUTTONS + 100)){
			int id = buttons[m - IDM_BUTTONS].id;
			if(id == 'L'){
				OPENFILENAME of;
				char path[MAX_PATH + 1];
				char title[MAX_PATH + 1];
				memset(&of, 0, sizeof(of));

				path[0] = 0;
				title[0] = 0;

				of.lStructSize = sizeof(of);
				of.hwndOwner = hWnd;
				of.lpstrFilter = "JPEG\0*.jpg;*.jpeg\0PNG\0*.png\0TIFF\0*.tiff\0\0";
				of.lpstrFile = path;
				of.nMaxFile = MAX_PATH;
				of.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				of.lpstrDefExt = "png";
				of.lpstrFileTitle = title;
				of.nMaxFileTitle = MAX_PATH;
				of.lpstrTitle = "Select image(s)";
				if(GetOpenFileName(&of)){
					QueueImage(path, title);
				}
			}else if(id == 'D'){
				LRESULT s = SendMessage(hListbox, LB_GETCURSEL, 0, 0);
				if(s != LB_ERR) DeleteImage(s);
			}
		}
	}else if(msg == WM_CLOSE){
		DestroyWindow(hWnd);
	}else if(msg == WM_DESTROY){
		PostQuitMessage(0);
	}else if(msg == WM_SB_SETTEXT){
		SendMessage(hStatus, SB_SETTEXT, wp, lp);
	}else if(msg == WM_SIZE){
		int parts[2];
		RECT r, r2;
		int pad = 5;
		int w = 100;
		int i;

		SendMessage(hStatus, WM_SIZE, wp, lp);

		parts[0] = LOWORD(lp) - 100;
		parts[1] = -1;

		SendMessage(hStatus, SB_SETPARTS, 2, (LPARAM)parts);

		SendMessage(hStatus, SB_GETRECT, 1, (LPARAM)&r);

		SetWindowPos(hProgress, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, 0);

		GetClientRect(hStatus, &r2);

		GetClientRect(hWnd, &r);
		r.bottom -= r2.bottom - r2.top + pad;
		r.right -= w + pad;
		r.left += pad;
		r.top += pad;

		r2 = r;

		SetWindowPos(hListbox, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, 0);

		for(i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++){
			SetWindowPos(buttons[i].wnd, NULL, r.right + pad, r.top + (i * (pad + 25)), w - pad, 25, 0);
		}
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
	int i;

	if(hWnd == NULL) return FALSE;

	hMain = hWnd;

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

	for(i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++){
		buttons[i].menu = (HMENU)(ULONG_PTR)(IDM_BUTTONS + i);
		buttons[i].wnd = CreateWindow("Button", buttons[i].text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd, buttons[i].menu, hInst, NULL);
	}

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

	if(!InitImageClass()){
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
