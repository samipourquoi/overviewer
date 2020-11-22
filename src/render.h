#ifndef OVERVIEWER_RENDER_H
#define OVERVIEWER_RENDER_H

#include <cairo/cairo.h>

typedef enum {
	TOP,
	LEFT,
	RIGHT,
	SIDE
} direction_t;

int render();
char* get_block_path(char* name);
cairo_surface_t* render_block(cairo_t* cr, char* name, direction_t direction);

#endif
