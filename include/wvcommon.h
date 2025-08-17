#ifndef __WVCOMMON_H__
#define __WVCOMMON_H__

#include <stb_ds.h>

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

extern HINSTANCE hInst;
extern const char* wvversion;

LRESULT CALLBACK VersionDialog(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
HBRUSH GetBrushCached(int r, int g, int b);

#endif
