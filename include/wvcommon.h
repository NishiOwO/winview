#ifndef __WVCOMMON_H__
#define __WVCOMMON_H__

#include <stb_ds.h>

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

extern HINSTANCE hInst;
extern HFONT fixedsys;
extern HFONT bifixedsys;
extern const char* wvversion;

LRESULT CALLBACK VersionDialog(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
HBRUSH GetSolidBrushCached(int r, int g, int b);
HBRUSH GetHatchBrushCached(int r, int g, int b);
void ShowBitmapSize(HDC hdc, const char* name, int x, int y, int w, int h);

#endif
