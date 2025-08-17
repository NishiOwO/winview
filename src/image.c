#include <wvcommon.h>

static char** path_list;

void QueueImage(const char* path, const char* title){
	LRESULT count = SendMessage(hListbox, LB_GETCOUNT, 0, 0);
	char* s = DuplicateString(path);
	SendMessage(hListbox, LB_ADDSTRING, 0, (LPARAM)title);

	arrput(path_list, s);
	if(count == 0){
		SendMessage(hListbox, LB_SETCURSEL, 0, 0);
		ShowImage(0);
	}
}

HWND hImage = NULL;
LRESULT CALLBACK ImageWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	if(msg == WM_PAINT){
		PAINTSTRUCT ps;
		RECT r;
		HDC dc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &r);
		ShowBitmapSize(dc, "WVFAILED", 0, 0, r.right - r.left, r.bottom - r.top);
		EndPaint(hWnd, &ps);
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

BOOL InitImageClass(void){
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = ImageWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(hInst, "WVLOGO");
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_MENU);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "winviewimage";
	wc.hIconSm = LoadIcon(hInst, "WVLOGO");
	return RegisterClassEx(&wc);
}

void ShowImage(int index){
	if(hImage == NULL){
		RECT r;

		GetWindowRect(hMain, &r);

		hImage = CreateWindow("winviewimage", "Image", WS_OVERLAPPEDWINDOW, r.right, r.top, 320, 240, NULL, 0, hInst, NULL);

		ShowWindow(hImage, SW_NORMAL);
		UpdateWindow(hImage);

		SetFocus(hMain);
	}
}

void DeleteImage(int index){
	LRESULT count, cursel;
	SendMessage(hListbox, LB_DELETESTRING, index, 0);

	cursel = SendMessage(hListbox, LB_GETCURSEL, 0, 0);
	count = SendMessage(hListbox, LB_GETCOUNT, 0, 0);
	if(cursel == LB_ERR || count > 0){
		int ind = index == 0 ? 0 : (index - 1);
		SendMessage(hListbox, LB_SETCURSEL, ind, 0);
		ShowImage(ind);
	}

	free(path_list[index]);
	arrdel(path_list, index);
}
