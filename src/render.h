#ifndef OVERVIEWER_RENDER_H
#define OVERVIEWER_RENDER_H

#include <cairo/cairo.h>
#include "parse.h"
#include "models.h"

#define TILE_HEIGHT 32
#define TILE_WIDTH 28
#define TILE_TOP_HEIGHT 16
#define CHUNK_HEIGHT 82
#define IMAGE_HEIGHT (TILE_HEIGHT*CHUNK_HEIGHT + 168)
#define IMAGE_WIDTH (TILE_WIDTH*16)

typedef enum {
	TOP = 1 << 0,
	LEFT = 1 << 1,
	RIGHT = 1 << 2
} direction_t;

int render(chunk_t *chunk);

void map_to_screen(int x, int y, int z, int *screen_x, int *screen_y);

char *get_block_path(char *name);

void draw_block(cairo_t *cr, model_t *model, int x, int y, int z, sides_t sides);

void draw_texture(cairo_t *cr, model_t *model, int x, int y, unsigned char sides, int tint);

cairo_surface_t *render_side(char *name, direction_t direction, int tint);

void render_tint(cairo_t *block_cr, cairo_surface_t *block, int tint);

#endif
