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
	cairo_surface_t* block = cairo_image_surface_create_from_png(path);
	cairo_surface_t* iso = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 32, 32);;
	cairo_t* iso_cr = cairo_create(iso);

	// Transformation matrices that apply a scaling
	// and a shearing. It also applies a rotation for the TOP side.
	// Go look at: http://jeroenhoek.nl/articles/svg-and-isometric-projection.html
	cairo_matrix_t matrix;
	switch (direction) {
	default:
	case TOP:
		matrix.x0 = 15;     matrix.y0 = 0;
		matrix.xx = .866;   matrix.xy = -.933;
		matrix.yx = .5;     matrix.yy = .461;
		break;
	case LEFT:
		matrix.x0 = 0;      matrix.y0 = 7;
		matrix.xx = .866;   matrix.xy = 0;
		matrix.yx = .5;     matrix.yy = 1;
		break;
	case RIGHT:
		matrix.x0 = 14;     matrix.y0 = 15;
		matrix.xx = .866;   matrix.xy = 0;
		matrix.yx = -.5;    matrix.yy = 1;
		break;
	}
	cairo_transform(iso_cr, &matrix);
	cairo_set_source_surface(iso_cr, block, 0, 0);
	cairo_paint(iso_cr);

	free(path);

	return iso;
}

void draw_block(cairo_t* cr, char* name, int x, int y, unsigned char direction) {
	if (direction & 0b100) {
		cairo_set_source_surface(cr, render_block(cr, name, TOP), x, y);
		cairo_paint(cr);
	}
	if (direction & 0b010) {
		cairo_set_source_surface(cr, render_block(cr, name, LEFT), x, y);
		cairo_paint(cr);
	}
	if (direction & 0b001) {
		cairo_set_source_surface(cr, render_block(cr, name, RIGHT), x, y);
		cairo_paint(cr);
	}
}

int render() {
	cairo_surface_t *surface;
	cairo_t *cr;
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 100, 100);
	cr = cairo_create(surface);

	draw_block(cr, "white_wool", 0, 0, LEFT | RIGHT | TOP);

	cairo_surface_write_to_png(surface, "render.png");

	return 0;
}
