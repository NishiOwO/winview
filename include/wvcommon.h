#ifndef __WVCOMMON_H__
#define __WVCOMMON_H__

#include <stb_ds.h>

#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* main.c */
extern HINSTANCE hInst;
extern HFONT fixedsys;
extern HFONT bifixedsys;
extern HWND hStatus, hProgress, hListbox;

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

/* image.c */
void QueueImage(const char* path, const char* title);
void ShowImage(int index);
void DeleteImage(int index);

#endif
