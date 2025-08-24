#ifndef __WVCOMMON_H__
#define __WVCOMMON_H__

#include <stb_ds.h>

#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <ctype.h>
#include <sys/stat.h>

/* currently */
#define BETA

enum WM_USERS {
	WM_TERMINATE_ME = WM_USER,
	WM_SB_SETTEXT,
	WM_COMPLETED
};

enum WVIMAGE_TYPE {
	WVIMAGE_READ_ROW = 0,
	WVIMAGE_READ_FRAME,
	WVIMAGE_READ_FRAME_T
};

typedef struct wvimage {
	void*	      opaque;
	int	      width;
	int	      height;
	FILE*	      fp;
	const char*   name;
	unsigned char type;
	unsigned char direction; /* 0 for up-to-down, 1 for down-to-up */
	void (*close)(void* ptr);
	unsigned char* (*read)(void* ptr);
} wvimage_t;
typedef wvimage_t*(DriverProc)(const char* path);

typedef struct RGBAPack_ {
	unsigned char red;
	unsigned char blue;
	unsigned char green;
	unsigned char alpha;
} RGBAPack;

#define Allocate(var) \
	var = malloc(sizeof(*var)); \
	memset(var, 0, sizeof(*var));
#define CreateRGBA(r, g, b, a) (((r) << 24) | ((g) << 16) | ((b) << 8) | ((a) << 0))

/* image drivers */
wvimage_t* TryJPEGDriver(const char* path);
wvimage_t* TryPNGDriver(const char* path);
wvimage_t* TryTIFFDriver(const char* path);
wvimage_t* TryXPMDriver(const char* path);
wvimage_t* TryGIFDriver(const char* path);
wvimage_t* TryTGADriver(const char* path);

/* main.c */
extern HINSTANCE hInst;
extern HFONT	 fixedsys, bifixedsys, betafont;
extern HWND	 hMain, hStatus, hProgress, hListbox;

/* version.c */
extern const char* wvversion;
LRESULT CALLBACK   VersionDialog(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

/* credits.c */
LRESULT CALLBACK CreditsDialog(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

/* util.c */
HBRUSH	   GetSolidBrushCached(int r, int g, int b);
HBRUSH	   GetHatchBrushCached(int r, int g, int b);
void	   ShowBitmapSize(HDC hdc, const char* name, int x, int y, int w, int h);
void	   SetProgress(int value);
void	   SetStatus(const char* text);
char*	   DuplicateString(const char* str);
wvimage_t* AllocateImage(void);
HANDLE	   CreateWinViewMutex(void);
void	   DestroyWinViewMutex(HANDLE mutex);
void	   LockWinViewMutex(HANDLE mutex);
void	   UnlockWinViewMutex(HANDLE mutex);
void	   CreateWinViewBitmap(int w, int h, HBITMAP* bmp, RGBQUAD** quad);
void	   ReadyStatus(void);
void	   AdjustImageWindowSize(void);
DWORD	   ParseHex(const char* str, int len);
WORD	   ReadAsWORD(unsigned char* ptr, int start);
DWORD	   ReadAsDWORD(unsigned char* ptr, int start);

/* image.c */
extern HWND hImage;
extern int  ImageWidth;
extern int  ImageHeight;
void	    QueueImage(const char* path, const char* title);
void	    ShowImage(int index);
void	    DeleteImage(int index);
BOOL	    InitImageClass(void);
void	    PreviousImage(void);
void	    NextImage(void);
void	    ScaleImage(double d);

#endif
