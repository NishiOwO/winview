#include <wvcommon.h>

static char** path_list = NULL;
const char* reading_path = NULL;

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
BOOL failed = FALSE;
int ImageWidth, ImageHeight;
static HANDLE image_thread = NULL;
static HANDLE image_mutex = NULL;
static BOOL image_kill = FALSE;
static HBITMAP image_bmp = NULL;
static RGBQUAD* image_quad;
static char ImageStatus[1024];
LRESULT CALLBACK ImageWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	if(msg == WM_PAINT){
		PAINTSTRUCT ps;
		RECT r;
		HDC dc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &r);
		SetStretchBltMode(dc, HALFTONE);
		if(image_bmp == NULL && failed){
			ShowBitmapSize(dc, "WVFAILED", 0, 0, r.right - r.left, r.bottom - r.top);
		}else if(image_bmp == NULL){
			FillRect(dc, &r, GetSysColorBrush(COLOR_MENU));
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
	}else if(msg == WM_GETMINMAXINFO){
		LPMINMAXINFO mmi = (LPMINMAXINFO)lp;
		mmi->ptMinTrackSize.x = 50;
		mmi->ptMinTrackSize.y = 50;
	}else if(msg == WM_ERASEBKGND){
	}else if(msg == WM_FINISHED_IMAGE){
		RECT r;
		int style;

		InvalidateRect(hWnd, 0, TRUE);
		if(image_bmp == NULL && failed){
			ImageWidth = 320;
			ImageHeight = 240;
		}else if(image_bmp == NULL){
			return 0;
		}else if(GetDIBCache(reading_path, &ImageWidth, &ImageHeight, ImageStatus) == NULL){
			SaveDIBCache(reading_path, image_bmp, ImageWidth, ImageHeight, ImageStatus);
		}

		r.left = 0;
		r.top = 0;
		r.right = ImageWidth;
		r.bottom = ImageHeight;
		style = (DWORD)GetWindowLongPtr(hImage, GWL_STYLE);
		AdjustWindowRect(&r, style, FALSE);
		SetWindowPos(hImage, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOMOVE);

		AdjustImageWindowSize();

		SetStatus(ImageStatus);
	}else{
		return DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}

static char txt[256];

static HANDLE image_event;
DWORD WINAPI ImageThread(LPVOID param){
	wvimage_t* img = NULL;
	int i;
	char* imgpath;
	RGBQUAD* local_quad;
	HBITMAP local_bmp;	

	imgpath = DuplicateString(param);
	SetEvent(image_event);

	SetProgress(0);
	SetStatus("Preparing an image");

	for(i = 0; i < sizeof(drivers) / sizeof(drivers[0]); i++){
		img = drivers[i](imgpath);
		if(img != NULL) break;
	}
	
	if(img == NULL){
		SetStatus("Failed to prepare an image");
		failed = TRUE;
	}else{
		CreateWinViewBitmap(img->width, img->height, &local_bmp, &local_quad);

		SetProgress(1);
		SetStatus("Reading an image");
		for(i = 0; i < img->height; i++){
			unsigned char* data;
			LockWinViewMutex(image_mutex);
			if(image_kill){
				img->close(img);
				UnlockWinViewMutex(image_mutex);
				free(imgpath);
				return 0;
			}
			UnlockWinViewMutex(image_mutex);
			SetProgress(1 + ((double)i / img->height * 99));
			data = img->read(img);
			if(data != NULL){
				int j;
				for(j = 0; j < img->width; j++){
					RGBQUAD* q = &local_quad[i * img->width + j];
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

		strcpy(ImageStatus, txt);
		ImageWidth = img->width;
		ImageHeight = img->height;

		img->close(img);

		image_bmp = local_bmp;
		image_quad = local_quad;
	}
	PostMessage(hImage, WM_FINISHED_IMAGE, 0, 0);
	free(imgpath);
	return 0;
}

BOOL InitImageClass(void){
	WNDCLASSEX wc;

	image_mutex = CreateWinViewMutex();
	image_event = CreateEvent(NULL, FALSE, FALSE, NULL);

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
	BOOL existed = TRUE;

	reading_path = path;

	if(hImage == NULL){
		RECT r;

		GetWindowRect(hMain, &r);

		hImage = CreateWindow("winviewimage", "Image", WS_OVERLAPPEDWINDOW, r.right, r.top, 320, 240, NULL, 0, hInst, NULL);

		ShowWindow(hImage, SW_NORMAL);
		UpdateWindow(hImage);

		SetFocus(hMain);

		existed = FALSE;
	}

	DestoryImageThreadIfPresent();

	image_bmp = NULL;
	image_quad = NULL;
	failed = FALSE;
	if(existed) SendMessage(hImage, WM_FINISHED_IMAGE, 0, 0);

	if((image_bmp = GetDIBCache(path, &ImageWidth, &ImageHeight, ImageStatus)) != NULL){
		SendMessage(hImage, WM_FINISHED_IMAGE, 0, 0);
	}else{
		image_thread = CreateThread(NULL, 0, ImageThread, (LPVOID)path, 0, &ident);
		WaitForSingleObject(image_event, INFINITE);
	}	
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

	DestroyDIBCache(path_list[index]);
	free(path_list[index]);
	arrdel(path_list, index);
}
