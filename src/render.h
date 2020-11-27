#ifndef OVERVIEWER_RENDER_H
#define OVERVIEWER_RENDER_H

#include <cairo/cairo.h>
#include "parse.h"

typedef enum {
	TOP     = 1<<0,
	LEFT    = 1<<1,
	RIGHT   = 1<<2
} direction_t;

int render(chunk_t* chunk);

#endif
