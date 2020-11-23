#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "../parson/parson.h"

char* get_block_path(char* name);
void draw_block(cairo_t* cr, char* name, int x, int y);
cairo_surface_t* render_block(char* name, direction_t direction);
void draw_texture(cairo_t* cr, char* name, int x, int y, unsigned char direction);

char* get_block_path(char* name) {
	char* dir = "assets/textures/"; // length: 16
	char* extension = ".png"; // length: 4
	char* path = malloc(16 + 4 + strlen(name) + 1);

	strcpy(path, dir);
	strcat(path, name);
	strcat(path, extension);

	return path;
}

#define DRAW_ARGS cairo_t* cr, JSON_Object* textures, int x, int y

void draw_model_cube_all(DRAW_ARGS) {
	char texture_name[50];
	const char* texture = json_object_get_string(textures, "all");

	memcpy(texture_name, &texture[16], 50);
	draw_texture(cr, texture_name, x, y, LEFT | RIGHT | TOP);
}

void draw_model_cube(DRAW_ARGS) {
	char texture_name[50];
	char* top = json_object_get_string(textures, "up");
	const char* south = json_object_get_string(textures, "south");
	const char* east = json_object_get_string(textures, "east");

	memcpy(texture_name, &top[16], 50);
	draw_texture(cr, texture_name, x, y, TOP);
	memcpy(texture_name, &south[16], 50);
	draw_texture(cr, texture_name, x, y, LEFT);
	memcpy(texture_name, &east[16], 50);
	draw_texture(cr, texture_name, x, y, RIGHT);
}

void draw_model_cube_bottom_top(DRAW_ARGS) {
	char texture_name[50];
	char* top = json_object_get_string(textures, "top");
	const char* side = json_object_get_string(textures, "side");

	memcpy(texture_name, &top[16], 50);
	draw_texture(cr, texture_name, x, y, TOP);
	memcpy(texture_name, &side[16], 50);
	draw_texture(cr, texture_name, x, y, LEFT | RIGHT);
}

void draw_model_cube_column(DRAW_ARGS) {
	char texture_name[50];
	char* end = json_object_get_string(textures, "end");
	const char* side = json_object_get_string(textures, "side");

	memcpy(texture_name, &end[16], 50);
	draw_texture(cr, texture_name, x, y, TOP);
	memcpy(texture_name, &side[16], 50);
	draw_texture(cr, texture_name, x, y, LEFT | RIGHT);
}

void pos_to_iso(int* x, int* y) {
	*x *= 14;
	*y *= 14;
	double new_x = *x + *y;
	double new_y = -.5 * (double)(*x) + .5 * (double)(*y);
	*x = (int)(new_x);
	*y = (int)(new_y);
}

void draw_block(cairo_t* cr, char* name, int x, int y) {
	pos_to_iso(&x, &y);

	char model_name[50];
	{
		// "assets/blockstates/": 19
		// ".json": 5
		// NULL terminating character: 1
		char* blockstate_path = malloc(19 + strlen(name) + 5 + 1);
		strcpy(blockstate_path, "assets/blockstates/");
		strcat(blockstate_path, name);
		strcat(blockstate_path, ".json");

		JSON_Value* blockstate = json_parse_file(blockstate_path);
		JSON_Object* root = json_value_get_object(blockstate);
		JSON_Object* variants = json_object_get_object(root, "variants");
		JSON_Value* variant_value = json_object_get_value_at(variants, 0);

		// It can either be an object (=one variant), or an
		// array of variants.
		JSON_Object* variant;
		if (json_type(variant_value) == JSONObject) {
			variant = json_value_get_object(variant_value);
		} else {
			variant = json_array_get_object(json_value_get_array(variant_value), 0);
		}
		char* model_id = json_object_get_string(variant, "model");
		memcpy(model_name, &model_id[16], 50);

		json_value_free(blockstate);
		free(blockstate_path);
	}

	{
		// "assets/models/block/": 21
		// ".json": 5
		// NULL terminating character: 1
		char* model_path = malloc(20 + strlen(name) + 5 + 1);
		strcpy(model_path, "assets/models/block/");
		strcat(model_path, model_name);
		strcat(model_path, ".json");

		JSON_Value* model = json_parse_file(model_path);
		JSON_Object* root = json_value_get_object(model);
		JSON_Object* textures = json_object_get_object(root, "textures");
		char* parent = json_object_get_string(root, "parent");
		if (strcmp(parent, "minecraft:block/cube_all") == 0) {
			draw_model_cube_all(cr, textures, x, y);
		} else if (strcmp(parent, "minecraft:block/cube") == 0) {
			draw_model_cube(cr, textures, x, y);
		} else if (strcmp(parent, "minecraft:block/cube_column") == 0
					|| strcmp(parent, "minecraft:block/cube_column_horizontal") == 0) {
			draw_model_cube_column(cr, textures, x, y);
		} else if (strcmp(parent, "minecraft:block/cube_bottom_top") == 0) {
			draw_model_cube_bottom_top(cr, textures, x, y);
		}

		free(model_path);
		json_value_free(model);
	}
}

cairo_surface_t* render_block(char* name, direction_t direction) {
	char* path = get_block_path(name);
	cairo_surface_t* block = cairo_image_surface_create_from_png(path);
	cairo_surface_t* iso = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 32, 32);;
	cairo_t* iso_cr = cairo_create(iso);

	#define COS_30 0.86602540378

	// Transformation matrices that apply a scaling
	// and a shearing. It also applies a rotation for the TOP side.
	// Go look at: http://jeroenhoek.nl/articles/svg-and-isometric-projection.html
	cairo_matrix_t matrix;
	switch (direction) {
	default:
	case TOP:
		matrix.x0 = 15;     matrix.y0 = 0;
		matrix.xx = COS_30;   matrix.xy = -.933;
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
	cairo_surface_destroy(block);
	cairo_destroy(iso_cr);

	return iso;
}

void draw_texture(cairo_t* cr, char* name, int x, int y, unsigned char direction) {
	cairo_surface_t* surface = NULL;
	if (direction & 0b100) {
		surface = render_block(name, TOP);
		cairo_set_source_surface(cr, surface, x, y);
		cairo_paint(cr);
	}
	if (direction & 0b010) {
		surface = render_block(name, LEFT);
		cairo_set_source_surface(cr, surface, x, y);
		cairo_paint(cr);
	}
	if (direction & 0b001) {
		surface = render_block(name, RIGHT);
		cairo_set_source_surface(cr, surface, x, y);
		cairo_paint(cr);
	}
	cairo_surface_destroy(surface);
}

int render() {
	cairo_surface_t *surface;
	cairo_t *cr;
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 100, 100);
	cr = cairo_create(surface);
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);


	draw_block(cr, "crafting_table", 0, 0);
	// draw_block(cr, "red_wool", -1, 1);
	// draw_block(cr, "stone", 1, 2);
	// draw_block(cr, "stone", 0, 2);
	// draw_block(cr, "gold_ore", 1, 3);
	// draw_block(cr, "stone", 0, 3);

	// draw_block(cr, "lime_wool", 0, 1);
	// draw_block(cr, "netherrack", 28, 0);
	// draw_block(cr, "crafting_table", 13, 23);
	// draw_block(cr, "target", 24, 24);

	cairo_surface_write_to_png(surface, "render.png");

	return 0;
}
