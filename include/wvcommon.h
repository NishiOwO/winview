#ifndef __WVCOMMON_H__
#define __WVCOMMON_H__

#include <stb_ds.h>

#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

enum WM_USERS {
	WM_FINISHED_IMAGE = WM_USER,
	WM_SB_SETTEXT
};

typedef struct wvimage {
	void* opaque;
	int width;
	int height;
	FILE* fp;
	const char* name;
	void(*close)(void* ptr);
	unsigned char*(*read)(void* ptr);
} wvimage_t;
typedef wvimage_t*(DriverProc)(const char* path);

#define Allocate(var) var = malloc(sizeof(*var));memset(var, 0, sizeof(*var));

/* image drivers */
wvimage_t* TryJPEGDriver(const char* path);
wvimage_t* TryPNGDriver(const char* path);
wvimage_t* TryTIFFDriver(const char* path);

/* main.c */
extern HINSTANCE hInst;
extern HFONT fixedsys;
extern HFONT bifixedsys;
extern HWND hMain, hStatus, hProgress, hListbox;

/* version.c */
extern const char* wvversion;
LRESULT CALLBACK VersionDialog(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

/* util.c */
HBRUSH GetSolidBrushCached(int r, int g, int b);
HBRUSH GetHatchBrushCached(int r, int g, int b);
void ShowBitmapSize(HDC hdc, const char* name, int x, int y, int w, int h);
void SetProgress(int value);
void SetStatus(const char* text);
char* DuplicateString(const char* str);
wvimage_t* AllocateImage(void);
HANDLE CreateWinViewMutex(void);
void DestroyWinViewMutex(HANDLE mutex);
void LockWinViewMutex(HANDLE mutex);
void UnlockWinViewMutex(HANDLE mutex);
void CreateWinViewBitmap(int w, int h, HBITMAP* bmp, RGBQUAD** quad);

/* image.c */
extern HWND hImage;
void QueueImage(const char* path, const char* title);
void ShowImage(int index);
void DeleteImage(int index);
BOOL InitImageClass(void);

#endif
