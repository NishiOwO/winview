#include <wvcommon.h>
#include <wvresource.h>

typedef struct button {
	const char* text;
	int	    id;
	HMENU	    menu;
	HWND	    wnd;
} button_t;

button_t buttons[] = {
    {"&Next", 'N', 0, NULL},  /**/
    {"&Prev", 'P', 0, NULL},  /**/
    {"&Load", 'L', 0, NULL},  /**/
    {"&Delete", 'D', 0, NULL} /**/
};

const char* exts[] = {
#ifdef DOJPEG
    "JPEG",
    "*.jpg;*.jpeg", /**/
#endif
#ifdef DOPNG
    "PNG",
    "*.png", /**/
#endif
#ifdef DOTIFF
    "TIFF",
    "*.tiff;*.tif", /**/
#endif
#ifdef DOXPM
    "XPM",
    "*.xpm", /**/
#endif
#ifdef DOGIF
    "GIF",
    "*.gif", /**/
#endif
#ifdef DOMSP
    "MSPaint (Windows 1.x/2.x Paint)",
    "*.msp", /**/
#endif
#ifdef DOTGA
    "TGA",
    "*.tga", /**/
#endif
};
char exttext[1024];
int  extseek = 0;

HWND	  hMain, hStatus, hProgress, hListbox;
HINSTANCE hInst;
HFONT	  fixedsys, bifixedsys, betafont;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg == WM_COMMAND) {
		int m = LOWORD(wp);
		if(m == IDM_FILE_QUIT) DestroyWindow(hWnd);
		if(m == IDM_VIEW_REASPECT) {
			if(hImage != NULL) {
				AdjustImageWindowSize();
			}
		}
		if(m == IDM_VIEW_10_LARGER || m == IDM_VIEW_10_SMALLER) {
			if(hImage != NULL) {
				ScaleImage(m == IDM_VIEW_10_LARGER ? 1 : -1);
			}
		}
		if(m == IDM_ABOUT_CREDITS) DialogBox(hInst, "WVCREDITS", hWnd, (DLGPROC)CreditsDialog);
		if(m == IDM_ABOUT_VERSION) DialogBox(hInst, "WVVERSION", hWnd, (DLGPROC)VersionDialog);
		if(m == IDM_LISTBOX) {
			if(HIWORD(wp) == LBN_DBLCLK) {
				ShowImage(SendMessage(hListbox, LB_GETCURSEL, 0, 0));
			}
		}
		if(m >= IDM_BUTTONS && m < (IDM_BUTTONS + 100)) {
			int id = buttons[m - IDM_BUTTONS].id;
			if(id == 'L') {
				OPENFILENAME of;
				char	     path[MAX_PATH + 1];
				char	     title[MAX_PATH + 1];
				memset(&of, 0, sizeof(of));

				path[0]	 = 0;
				title[0] = 0;

				of.lStructSize	  = sizeof(of);
				of.hwndOwner	  = hWnd;
				of.lpstrFilter	  = exttext;
				of.lpstrFile	  = path;
				of.nMaxFile	  = MAX_PATH;
				of.Flags	  = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				of.lpstrDefExt	  = "png";
				of.lpstrFileTitle = title;
				of.nMaxFileTitle  = MAX_PATH;
				of.lpstrTitle	  = "Select image(s)";
				if(GetOpenFileName(&of)) {
					QueueImage(path, title);
				}
			} else if(id == 'D') {
				LRESULT s = SendMessage(hListbox, LB_GETCURSEL, 0, 0);
				if(s != LB_ERR) DeleteImage(s);
			} else if(id == 'P') {
				PreviousImage();
			} else if(id == 'N') {
				NextImage();
			}
		}
	} else if(msg == WM_CLOSE) {
		DestroyWindow(hWnd);
	} else if(msg == WM_DESTROY) {
		PostQuitMessage(0);
	} else if(msg == WM_SB_SETTEXT) {
		SendMessage(hStatus, SB_SETTEXT, wp, lp);
	} else if(msg == WM_SIZE) {
		int  parts[2];
		RECT r, r2;
		int  pad = 5;
		int  w	 = 100;
		int  i;

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

		for(i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
			SetWindowPos(buttons[i].wnd, NULL, r.right + pad, r.top + (i * (pad + 25)), w - pad, 25, 0);
		}
	} else {
		return DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}

BOOL InitClass(void) {
	WNDCLASSEX wc;
	wc.cbSize	 = sizeof(wc);
	wc.style	 = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInst;
	wc.hIcon	 = LoadIcon(hInst, "WVLOGO");
	wc.hCursor	 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wc.lpszMenuName	 = "wvmenu";
	wc.lpszClassName = "winview";
	wc.hIconSm	 = LoadIcon(hInst, "WVLOGO");
	return RegisterClassEx(&wc);
}

BOOL InitWindow(int nCmdShow) {
	HWND hWnd = CreateWindow("winview", "WinView", WS_OVERLAPPEDWINDOW ^ (WS_MAXIMIZEBOX | WS_THICKFRAME), CW_USEDEFAULT, CW_USEDEFAULT, 440, 440 / 4 * 3, NULL, 0, hInst, NULL);
	HDC  dc;
	int  parts[2];
	int  i;

	if(hWnd == NULL) return FALSE;

	hMain = hWnd;

	parts[0] = 0;
	parts[1] = 100;
	hStatus	 = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | CCS_BOTTOM, 0, 0, 0, 0, hWnd, (HMENU)IDM_STATUS, hInst, NULL);
	SendMessage(hStatus, SB_SIMPLE, FALSE, 0);
	SendMessage(hStatus, SB_SETPARTS, 2, (LPARAM)parts);

	hProgress = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 0, 0, 0, 0, hStatus, (HMENU)IDM_STATUS_PROGRESS, hInst, NULL);
	SendMessage(hProgress, PBM_SETRANGE, 0, MAKELONG(0, 100));
	SendMessage(hProgress, PBM_SETPOS, 0, 0);

	ReadyStatus();

	hListbox = CreateWindow("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, 0, 0, 0, 0, hWnd, (HMENU)IDM_LISTBOX, hInst, NULL);

	for(i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
		buttons[i].menu = (HMENU)(ULONG_PTR)(IDM_BUTTONS + i);
		buttons[i].wnd	= CreateWindow("BUTTON", buttons[i].text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd, buttons[i].menu, hInst, NULL);
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

char** ParseArgs(const char* str) {
	char** r = NULL;
	char   p[MAX_PATH + 1];
	int    i;
	int    dq = 0;

	p[0] = 0;

	for(i = 0; str[i] != 0; i++) {
		if(str[i] == '"') {
			dq = dq == 1 ? 0 : 1;
		} else if(str[i] == ' ' && !dq) {
			if(strlen(p) > 0) {
				char* p2 = DuplicateString(p);
				arrput(r, p2);
			}
			p[0] = 0;
		} else {
			int l	 = strlen(p);
			p[l]	 = str[i];
			p[l + 1] = 0;
		}
	}
	if(strlen(p) > 0) {
		char* p2 = DuplicateString(p);
		arrput(r, p2);
	}

	return r;
}

void AddEntry(const char* name, const char* exts) {
	char* exts_c = DuplicateString(exts);
	int   i;
	for(i = 0; exts_c[i] != 0; i++) {
		if(exts_c[i] == ';') exts_c[i] = ' ';
	}

	strcpy(exttext + extseek, name);
	extseek += strlen(name);

	strcpy(exttext + extseek, " (");
	extseek += 2;

	strcpy(exttext + extseek, exts_c);
	extseek += strlen(exts_c);

	strcpy(exttext + extseek, ")");
	extseek += 2;

	strcpy(exttext + extseek, exts);
	extseek += strlen(exts) + 1;

	free(exts_c);
}

int WINAPI WinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst, LPSTR lpsCmdLine, int nCmdShow) {
	BOOL   bret;
	MSG    msg;
	char** args = ParseArgs(lpsCmdLine);
	int    i;
	char   allsupport[1024];

	allsupport[0] = 0;

	memset(exttext, 0, sizeof(exttext));
	for(i = 0; i < sizeof(exts) / sizeof(exts[0]); i += 2) {
		if(strlen(allsupport) == 0) {
			strcpy(allsupport, exts[i + 1]);
		} else {
			strcat(allsupport, ";");
			strcat(allsupport, exts[i + 1]);
		}
	}

	AddEntry("All Supported Formats", allsupport);

	for(i = 0; i < sizeof(exts) / sizeof(exts[0]); i += 2) {
		AddEntry(exts[i], exts[i + 1]);
	}

	AddEntry("All files", "*.*");

	hInst = hCurInst;
	if(!InitClass()) {
		return FALSE;
	}

	if(!InitImageClass()) {
		return FALSE;
	}

	InitCommonControls();

	fixedsys   = CreateFont(14, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, NULL);
	bifixedsys = CreateFont(14 * 7.5, 0, 0, 0, FW_BOLD, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, NULL);
	betafont   = CreateFont(14 * 3, 0, 112, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, NULL);

	if(!InitWindow(nCmdShow)) {
		return FALSE;
	}

	for(i = 0; i < arrlen(args); i++) {
		FILE* f = fopen(args[i], "rb");
		if(f != NULL) {
			char* name = args[i];
			int   j;

			for(j = strlen(args[i]) - 1; j >= 0; j--) {
				if(args[i][j] == '/' || args[i][j] == '\\') {
					name = args[i] + j + 1;
					break;
				}
			}

			QueueImage(args[i], name);
			fclose(f);
		}
	}

	while((bret = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if(bret == -1) break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
