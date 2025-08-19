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

		LockWinViewMutex(mutex);
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
		UnlockWinViewMutex(mutex);

		EndPaint(hWnd, &ps);
	}else if(msg == WM_TERMINATE_ME || msg == WM_COMPLETED){
		image_t* img = (image_t*)lp;

		WaitForSingleObject(img->thread, INFINITE);
		CloseHandle(img->thread);
		img->thread = NULL;

		LockWinViewMutex(mutex);
		if(msg == WM_COMPLETED && shown == img && img->bitmap == NULL){
			RECT r;
			int style;

			ImageWidth = 320;
			ImageHeight = 240;

			SetRect(&r, 0, 0, img->width, img->height);
			style = (DWORD)GetWindowLongPtr(hImage, GWL_STYLE);
			AdjustWindowRect(&r, style, FALSE);
			SetWindowPos(hWnd, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOMOVE);

			AdjustImageWindowSize();

			InvalidateRect(hWnd, NULL, FALSE);
		}else if(msg == WM_COMPLETED && shown == img){
			RECT r;
			int style;

			ImageWidth = img->width;
			ImageHeight = img->height;

			SetRect(&r, 0, 0, img->width, img->height);
			style = (DWORD)GetWindowLongPtr(hImage, GWL_STYLE);
			AdjustWindowRect(&r, style, FALSE);
			SetWindowPos(hWnd, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOMOVE);

			AdjustImageWindowSize();

			InvalidateRect(hWnd, NULL, FALSE);

			SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)img->status);
		}
		UnlockWinViewMutex(mutex);
	}else if(msg == WM_GETMINMAXINFO){
		LPMINMAXINFO mmi = (LPMINMAXINFO)lp;
		mmi->ptMinTrackSize.x= 200;
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

		PostMessage(hImage, WM_COMPLETED, 0, (LPARAM)image);
		return 0;
	}

	sprintf(image->status, "%dx%d, %s image", wvimg->width, wvimg->height, wvimg->name);

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

		LockWinViewMutex(mutex);
		if(shown == image){
			SetProgress((double)i / wvimg->height * 100);
		}
		UnlockWinViewMutex(mutex);

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

	if(image->bitmap == NULL){
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

		ImageWidth = image->width;
		ImageHeight = image->height;

		SetRect(&r, 0, 0, ImageWidth, ImageHeight);
		style = (DWORD)GetWindowLongPtr(hImage, GWL_STYLE);
		AdjustWindowRect(&r, style, FALSE);
		SetWindowPos(hImage, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOMOVE);

		AdjustImageWindowSize();

		InvalidateRect(hImage, NULL, FALSE);

		SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)shown->status);
	}
}

void DeleteImage(int index){
	LRESULT count, cursel;

	if(images[index]->thread != NULL){
		images[index]->go_terminate = TRUE;
		WaitForSingleObject(images[index]->thread, INFINITE);
		CloseHandle(images[index]->thread);
		images[index]->thread = NULL;
		CloseHandle(images[index]->wakeup_event);
		CloseHandle(images[index]->ack_event);
		free(images[index]->path);
	}
	if(images[index]->bitmap != NULL) DeleteObject(images[index]->bitmap);

	SendMessage(hListbox, LB_DELETESTRING, index, 0);
	arrdel(images, index);

	count = SendMessage(hListbox, LB_GETCOUNT, 0, 0);
	cursel = SendMessage(hListbox, LB_GETCURSEL, 0, 0);
	if(count == 0){
		DestroyWindow(hImage);
		hImage = NULL;
		ReadyStatus();
	}else if(cursel == LB_ERR){
		int ind = index == 0 ? 0 : (index - 1);
		SendMessage(hListbox, LB_SETCURSEL, ind, 0);
		ShowImage(ind);
	}
}
