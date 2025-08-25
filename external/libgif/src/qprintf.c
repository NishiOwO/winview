/*****************************************************************************

 qprintf.c - module to emulate a printf with a possible quiet (disable mode.)

 A global variable GifNoisyPrint controls the printing of this routine

SPDX-License-Identifier: MIT

*****************************************************************************/

#include <stdarg.h>
#include <stdio.h>

#include "gif_lib.h"

GifBool GifNoisyPrint = GifFalse;

/*****************************************************************************
 Same as fprintf to stderr but with optional print.
******************************************************************************/
void GifQprintf(char *Format, ...) {
	va_list ArgPtr;

	va_start(ArgPtr, Format);

	if (GifNoisyPrint) {
		char Line[128];
		(void)vsprintf(Line, Format, ArgPtr);
		(void)fputs(Line, stderr);
	}

	va_end(ArgPtr);
}

void PrintGifError(int ErrorCode) {
	const char *Err = GifErrorString(ErrorCode);

	if (Err != NULL) {
		fprintf(stderr, "GIF-LIB error: %s.\n", Err);
	} else {
		fprintf(stderr, "GIF-LIB undefined error %d.\n", ErrorCode);
	}
}

/* end */
