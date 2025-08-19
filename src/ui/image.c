#include <wvcommon.h>

typedef struct image {
	char* path;
	HANDLE thread;
	HANDLE wakeup_event;
	HANDLE ack_event;
	BOOL go_sleep;
	BOOL go_terminate;
	HBITMAP bitmap;
	int width;
	int height;
	char status[1024];
} image_t;

int ImageWidth, ImageHeight;
HWND hImage = NULL;
static image_t* shown = NULL;
static image_t** images = NULL;
static HANDLE mutex = NULL;

void QueueImage(const char* path, const char* title){
	image_t* image;

	Allocate(image);

	image->path = DuplicateString(path);
	image->thread = NULL;
	image->wakeup_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	image->ack_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	image->go_sleep = FALSE;
	image->go_terminate = FALSE;

	arrput(images, image);

	SendMessage(hListbox, LB_ADDSTRING, 0, (LPARAM)title);

	if(arrlen(images) == 1){
		SendMessage(hListbox, LB_SETCURSEL, 0, 0);
		ShowImage(0);
	}
}

DriverProc* drivers[] = {
	TryPNGDriver,
	TryJPEGDriver,
	TryTIFFDriver
};

LRESULT CALLBACK ImageWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	if(msg == WM_CLOSE){
		hImage = NULL;
		DestroyWindow(hWnd);
	}else if(msg == WM_PAINT){
		PAINTSTRUCT ps;
		RECT r;
		HDC hdc = BeginPaint(hWnd, &ps);

		GetClientRect(hWnd, &r);

		if(shown == NULL || (shown->thread != NULL && shown->bitmap != NULL)){
			FillRect(hdc, &r, GetSysColorBrush(COLOR_MENU));
		}else if(shown->thread == NULL && shown->bitmap == NULL){
			ShowBitmapSize(hdc, "WVFAILED", 0, 0, r.right - r.left, r.bottom - r.top);
		}else if(shown->bitmap != NULL){
			HDC hmdc = CreateCompatibleDC(hdc);

			SetStretchBltMode(hdc, HALFTONE);

			SelectObject(hmdc, shown->bitmap);
			StretchBlt(hdc, 0, 0, r.right - r.left, r.bottom - r.top, hmdc, 0, 0, ImageWidth, ImageHeight, SRCCOPY);
			DeleteObject(hmdc);
		}
		EndPaint(hWnd, &ps);
	}else if(msg == WM_TERMINATE_ME || msg == WM_COMPLETED){
		image_t* img = (image_t*)lp;

		WaitForSingleObject(img->thread, INFINITE);
		CloseHandle(img->thread);
		img->thread = NULL;

		if(msg == WM_COMPLETED){
			RECT r;
			int style;

			SaveDIBCache(img->path, img->bitmap, img->width, img->height, img->status);

			ImageWidth = img->width;
			ImageHeight = img->height;

			SetRect(&r, 0, 0, img->width, img->height);
			style = (DWORD)GetWindowLongPtr(hImage, GWL_STYLE);
			AdjustWindowRect(&r, style, FALSE);
			SetWindowPos(hWnd, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOMOVE);

			AdjustImageWindowSize();

			InvalidateRect(hWnd, NULL, FALSE);
		}
	}else if(msg == WM_ERASEBKGND){
	}else{
		return DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}

static char txt[256];
DWORD WINAPI ImageThread(LPVOID param){
	image_t* image = (image_t*)param;
	int i;
	wvimage_t* wvimg;
	HBITMAP bmp;
	RGBQUAD* quad;

	SetEvent(image->ack_event);

	LockWinViewMutex(mutex);
	if(shown == image){
		SetStatus("Reading image");
	}
	UnlockWinViewMutex(mutex);

	for(i = 0; i < sizeof(drivers) / sizeof(drivers[0]); i++){
		wvimg = drivers[i](image->path);

		if(wvimg != NULL) break;
	}

	if(wvimg == NULL){
		LockWinViewMutex(mutex);
		if(shown == image){
			SetStatus("Failed to read image");
		}
		UnlockWinViewMutex(mutex);

		PostMessage(hImage, WM_TERMINATE_ME, 0, (LPARAM)image);
		return 0;
	}

	image->width = wvimg->width;
	image->height = wvimg->height;
	CreateWinViewBitmap(wvimg->width, wvimg->height, &bmp, &quad);
	for(i = 0; i < wvimg->height; i++){
		unsigned char* row;
		int j;
		if(image->go_sleep){
			image->go_sleep = FALSE;
			SetEvent(shown->ack_event);
			WaitForSingleObject(image->wakeup_event, INFINITE);
			SetEvent(shown->ack_event);
		}
		if(image->go_terminate){
			break;
		}

		row = wvimg->read(wvimg);
		for(j = 0; j < wvimg->width; j++){
			RGBQUAD* px = &quad[i * wvimg->width + j];
			px->rgbRed = row[j * 4 + 0];
			px->rgbGreen = row[j * 4 + 1];
			px->rgbBlue = row[j * 4 + 2];
			px->rgbReserved = 0;
		}
	}
	wvimg->close(wvimg);

	if(image->go_terminate){
		image->go_terminate = FALSE;
		DeleteObject(bmp);
	}else{
		image->bitmap = bmp;
		PostMessage(hImage, WM_COMPLETED, 0, (LPARAM)image);
	}

	return 0;
}

BOOL InitImageClass(void){
	WNDCLASSEX wc;

	InitDIBCache();

	mutex = CreateWinViewMutex();

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
	char Status[1024];
	image_t* image;

	if(hImage == NULL){
		hImage = CreateWindow("winviewimage", "Image", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 320, 200, NULL, 0, hInst, NULL);
		ShowWindow(hImage, SW_NORMAL);
		UpdateWindow(hImage);
	}

	image = images[index];

	if(shown != NULL && shown->thread != NULL){
		shown->go_sleep = TRUE;
		WaitForSingleObject(shown->ack_event, INFINITE);
	}

	LockWinViewMutex(mutex);
	shown = image;
	UnlockWinViewMutex(mutex);

	if(GetDIBCache(image->path, &ImageWidth, &ImageHeight, Status) == NULL){
		if(image->thread == NULL){
			DWORD id;
			image->thread = CreateThread(NULL, 0, ImageThread, image, 0, &id);
		}else{
			SetEvent(image->wakeup_event);
		}
		WaitForSingleObject(image->ack_event, INFINITE);
	}else{
		RECT r;
		int style;

		SetRect(&r, 0, 0, ImageWidth, ImageHeight);
		style = (DWORD)GetWindowLongPtr(hImage, GWL_STYLE);
		AdjustWindowRect(&r, style, FALSE);
		SetWindowPos(hImage, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOMOVE);

		AdjustImageWindowSize();

		InvalidateRect(hImage, NULL, FALSE);
	}
}

void DeleteImage(int index){
}
