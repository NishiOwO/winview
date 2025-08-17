#include <wvcommon.h>

typedef struct brushcache {
	int r;
	int g;
	int b;
	HBRUSH brush;
} brushcache_t;

static brushcache_t* cache = NULL;

HBRUSH GetBrushCached(int r, int g, int b){
	int i;
	brushcache_t c;

	for(i = 0; i < arrlen(cache); i++){
		if(cache[i].r == r && cache[i].g == g && cache[i].b == b){
			return cache[i].brush;
		}
	}

	c.r = r;
	c.g = g;
	c.b = b;
	c.brush = CreateSolidBrush(RGB(r, g, b));

	arrput(cache, c);

	return c.brush;
}
