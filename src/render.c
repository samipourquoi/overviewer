#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "../parson/parson.h"

#define TILE_HEIGHT 32
#define TILE_WIDTH 28
#define TILE_TOP_HEIGHT 16
#define CHUNK_HEIGHT 128
#define IMAGE_HEIGHT (TILE_HEIGHT*CHUNK_HEIGHT + 168)
#define IMAGE_WIDTH (TILE_WIDTH*16)

char* get_block_path(char* name);
void draw_block(cairo_t* cr, char* name, int x, int y, int z);
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

void pos_to_iso(int x, int y, int z, int* screen_x, int* screen_y) {
	// Makes z=0 start at the top instead of at the bottom.
	z = 15 - z;

	// Don't ask me why 3.48 works, it just does.
	*screen_x = TILE_WIDTH/2 * (x + z);
	*screen_y = TILE_WIDTH/3.48 * (-z + x);

	// Computes the Y offset to make y=0 go at the bottom.
	// The -168 at the end is for letting the whole y=0 layer fit
	// (it would be cropped otherwise).
	*screen_y += IMAGE_HEIGHT - (y-1)*TILE_HEIGHT/2 - 168;
}

void draw_block(cairo_t* cr, char* name, int x, int y, int z) {
	int screen_x, screen_y;
	pos_to_iso(x, y, z, &screen_x, &screen_y);

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
		if (parent == NULL) return;

		if (strcmp(parent, "minecraft:block/cube_all") == 0) {
			draw_model_cube_all(cr, textures, screen_x, screen_y);
		} else if (strcmp(parent, "minecraft:block/cube") == 0) {
			draw_model_cube(cr, textures, screen_x, screen_y);
		} else if (strcmp(parent, "minecraft:block/cube_column") == 0
					|| strcmp(parent, "minecraft:block/cube_column_horizontal") == 0) {
			draw_model_cube_column(cr, textures, screen_x, screen_y);
		} else if (strcmp(parent, "minecraft:block/cube_bottom_top") == 0) {
			draw_model_cube_bottom_top(cr, textures, screen_x, screen_y);
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

	#define COS_30 0.86602540378
	switch (direction) {
	default:
	case TOP:
		matrix.x0 = 14;     matrix.y0 = 0;
		matrix.xx = COS_30;   matrix.xy = -COS_30;
		matrix.yx = 0.5;     matrix.yy = 0.5;
		break;
	case LEFT:
		matrix.x0 = 0;      matrix.y0 = TILE_TOP_HEIGHT/2;
		matrix.xx = COS_30;   matrix.xy = 0;
		matrix.yx = 0.5;     matrix.yy = 1;
		break;
	case RIGHT:
		matrix.x0 = TILE_WIDTH/2; matrix.y0 = TILE_TOP_HEIGHT;
		matrix.xx = COS_30;   matrix.xy = 0;
		matrix.yx = -0.5;    matrix.yy = 1;
		break;
	}
	#undef COS_30
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

int render(char* blocks[16][256][16]) {
	cairo_surface_t *surface;
	cairo_t *cr;
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, IMAGE_WIDTH, IMAGE_HEIGHT);
	cr = cairo_create(surface);
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);

	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < CHUNK_HEIGHT; y++) {
			for (int z = 0; z < 16; z++) {
				char* block = blocks[x][y][z];
				if (block == NULL) continue;
				draw_block(cr, block, x, y, z);
				free(blocks[x][y][z]);
			}
		}
	}

	cairo_surface_write_to_png(surface, "render.png");

	return 0;
}
