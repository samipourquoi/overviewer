#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "render.h"

char* get_block_path(char* name) {
	char* dir = "assets/textures/"; // length: 16
	char* extension = ".png"; // length: 4
	char* path = malloc(16 + 4 + strlen(name) + 1);

	strcpy(path, dir);
	strcat(path, name);
	strcat(path, extension);

	return path;
}

cairo_surface_t* render_block(cairo_t* cr, char* name, direction_t direction) {
	char* path = get_block_path(name);
	cairo_surface_t *block;
	block = cairo_image_surface_create_from_png(path);

	cairo_matrix_t matrix;
	matrix.x0 = 0;  matrix.y0 = 0;

	switch (direction) {
	default:
	case TOP:
		matrix.xx = 1;   matrix.xy = 1;
		matrix.yx = -.5; matrix.yy = .5;
		break;
	case LEFT:
		matrix.xx = 1;   matrix.xy = 0;
		matrix.yx = .577; matrix.yy = 1;
		break;
	case RIGHT:
		matrix.xx = 1;   matrix.xy = 0;
		matrix.yx = -.577; matrix.yy = 1;
		break;
	}

	cairo_transform(cr, &matrix);
	cairo_set_source_surface(cr, block, 0, 0);
	cairo_paint(cr);

	free(path);

	return block;
}

int render() {
	cairo_surface_t *surface;
	cairo_t *cr;
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 100, 100);
	cr = cairo_create(surface);

	render_block(cr, "stone", LEFT);
	render_block(cr, "stone", RIGHT);
	render_block(cr, "stone", TOP);
	cairo_surface_write_to_png(surface, "oak.png");

	return 0;
}
