#ifndef OVERVIEWER_RENDER_H
#define OVERVIEWER_RENDER_H

#include <cairo/cairo.h>

typedef enum {
	TOP     = 0b100,
	LEFT    = 0b010,
	RIGHT   = 0b001
} direction_t;

int render();

#endif
