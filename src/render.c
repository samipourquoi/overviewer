#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "../parson/parson.h"

char* get_block_path(char* name);
void draw_model_cube_all(cairo_t* cr, JSON_Object* textures);
void draw_block(cairo_t* cr, char* name);
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

void draw_model_cube_all(cairo_t* cr, JSON_Object* textures) {
	char texture_name[50];
	const char* texture = json_object_get_string(textures, "all");
	memcpy(texture_name, &texture[16], 50);
	draw_texture(cr, texture_name, 0, 0, LEFT | RIGHT | TOP);
}

void draw_block(cairo_t* cr, char* name) {
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
			draw_model_cube_all(cr, textures);
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

	draw_block(cr, "diamond_block");

	cairo_surface_write_to_png(surface, "render.png");

	return 0;
}
