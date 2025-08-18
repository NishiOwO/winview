#include <wvcommon.h>

static char** path_list = NULL;

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

DriverProc* drivers[] = {
	TryPNGDriver,
	TryJPEGDriver,
	TryTIFFDriver
};
HWND hImage = NULL;
int ImageWidth, ImageHeight;
static HANDLE image_thread = NULL;
static HANDLE image_mutex = NULL;
static BOOL image_kill = FALSE;
static HBITMAP image_bmp = NULL;
static RGBQUAD* image_quad;
LRESULT CALLBACK ImageWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	if(msg == WM_PAINT){
		PAINTSTRUCT ps;
		RECT r;
		HDC dc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &r);
		SetStretchBltMode(dc, HALFTONE);
		if(image_bmp == NULL){
			ShowBitmapSize(dc, "WVFAILED", 0, 0, r.right - r.left, r.bottom - r.top);
		}else{
			HDC hmdc = CreateCompatibleDC(dc);
			SelectObject(hmdc, image_bmp);
			StretchBlt(dc, 0, 0, r.right - r.left, r.bottom - r.top, hmdc, 0, 0, ImageWidth, ImageHeight, SRCCOPY);
			DeleteDC(hmdc);
		}
		EndPaint(hWnd, &ps);
	}else if(msg == WM_CLOSE){
		hImage = NULL;
		DestroyWindow(hWnd);
	}else if(msg == WM_ERASEBKGND){
	}else if(msg == WM_FINISHED_IMAGE){
		RECT r;
		int style;
		InvalidateRect(hWnd, 0, TRUE);
		if(image_bmp == NULL){
			SetRect(&r, 0, 0, 320, 240);
		}else{
			SetRect(&r, 0, 0, ImageWidth, ImageHeight);
		}
		style = (DWORD)GetWindowLongPtr(hWnd, GWL_STYLE);
		AdjustWindowRect(&r, style, FALSE);
		SetWindowPos(hWnd, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOMOVE);
	}else{
		return DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}

static char txt[256];

DWORD WINAPI ImageThread(LPVOID param){
	wvimage_t* img = NULL;
	int i;

	SetProgress(0);
	SetStatus("Preparing an image");

	for(i = 0; i < sizeof(drivers) / sizeof(drivers[0]); i++){
		img = drivers[i](param);
		if(img != NULL) break;
	}

	if(image_bmp != NULL){
		DeleteObject(image_bmp);
	}
	image_bmp = NULL;
	image_quad = NULL;
	if(img == NULL){
		SetStatus("Failed to prepare an image");
	}else{
		CreateWinViewBitmap(img->width, img->height, &image_bmp, &image_quad);

		SetProgress(1);
		SetStatus("Reading an image");
		for(i = 0; i < img->height; i++){
			unsigned char* data;
			LockWinViewMutex(image_mutex);
			if(image_kill){
				img->close(img);
				UnlockWinViewMutex(image_mutex);
				return 0;
			}
			UnlockWinViewMutex(image_mutex);
			SetProgress(1 + ((double)i / img->height * 99));
			data = img->read(img);
			if(data != NULL){
				int j;
				for(j = 0; j < img->width; j++){
					RGBQUAD* q = &image_quad[i * img->width + j];
					unsigned char* px = &data[j * 4];
					double op = 1.0 - (double)px[3] / 255;
					unsigned char c;
					q->rgbRed = px[0];
					q->rgbGreen = px[1];
					q->rgbBlue = px[2];

					c = ((i / 16 + j / 16) % 2) ? 0x80 : 0x60;

					q->rgbRed += op * c;
					q->rgbGreen += op * c;
					q->rgbBlue += op * c;

					q->rgbReserved = 0;
				}
				free(data);
			}
		}

		SetProgress(100);

		sprintf(txt, "%dx%d, %s image", img->width, img->height, img->name);
		SetStatus(txt);

		ImageWidth = img->width;
		ImageHeight = img->height;

		img->close(img);
	}
	PostMessage(hImage, WM_FINISHED_IMAGE, 0, 0);
	return 0;
}

BOOL InitImageClass(void){
	WNDCLASSEX wc;

	image_mutex = CreateWinViewMutex();

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

void DestoryImageThreadIfPresent(void){
	if(image_thread != NULL){
		LockWinViewMutex(image_mutex);
		image_kill = TRUE;
		UnlockWinViewMutex(image_mutex);
		WaitForSingleObject(image_thread, INFINITE);
		image_kill = FALSE;
		CloseHandle(image_thread);
		image_thread = NULL;
	}
}

void ShowImage(int index){
	const char* path = path_list[index];
	DWORD ident;

	if(hImage == NULL){
		RECT r;

		GetWindowRect(hMain, &r);

		hImage = CreateWindow("winviewimage", "Image", WS_OVERLAPPEDWINDOW, r.right, r.top, 320, 240, NULL, 0, hInst, NULL);

		ShowWindow(hImage, SW_NORMAL);
		UpdateWindow(hImage);

		SetFocus(hMain);
	}

	DestoryImageThreadIfPresent();
	image_thread = CreateThread(NULL, 0, ImageThread, (LPVOID)path, 0, &ident);
}

void DeleteImage(int index){
	LRESULT count, cursel;

	SendMessage(hListbox, LB_DELETESTRING, index, 0);

	count = SendMessage(hListbox, LB_GETCOUNT, 0, 0);
	cursel = SendMessage(hListbox, LB_GETCURSEL, 0, 0);
	if(count == 0){
		DestoryImageThreadIfPresent();
		DestroyWindow(hImage);
		hImage = NULL;
		ReadyStatus();
	}else if(cursel == LB_ERR){
		int ind = index == 0 ? 0 : (index - 1);
		SendMessage(hListbox, LB_SETCURSEL, ind, 0);
		ShowImage(ind);
	}

	free(path_list[index]);
	arrdel(path_list, index);
}
