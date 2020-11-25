#ifndef OVERVIEWER_RENDER_H
#define OVERVIEWER_RENDER_H

#include <cairo/cairo.h>

typedef enum {
	TOP     = 1<<0,
	LEFT    = 1<<1,
	RIGHT   = 1<<2
} direction_t;

int render(char* blocks[16][256][16]);

#endif
