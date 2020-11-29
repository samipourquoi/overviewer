#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "models.h"

#define IS_AIR(BLOCK) ((int)strcmp(BLOCK, "air") == 0 || (int)strcmp(BLOCK, "cave_air") == 0 || (int)strcmp(BLOCK, "grass") == 0)

/**
 * Convert a coordinate in the map to screen
 * coordinates. Puts the resulting X and Y coordinates
 * into the pointers `screen_x` and `screen_y`.
 *
 * @details
 * (0; 0; 0) is at the back.
 * (0; 0; 15) is on the left.
 * (15; 0; 0) is on the right.
 * (15; 0; 15) is at the front.
 * The Y value can vary from 0 to `CHUNK_HEIGHT`.
 */
void map_to_screen(int x, int y, int z, int* screen_x, int* screen_y) {
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

/**
 * Returns the path to the given texture name.
 *
 * @warning
 * Don't forget to free() the returned string!
 */
char* get_block_path(char* name) {
	char* dir = "assets/textures/"; // length: 16
	char* extension = ".png"; // length: 4
	char* path = malloc(16 + 4 + strlen(name) + 1);

	strcpy(path, dir);
	strcat(path, name);
	strcat(path, extension);

	return path;
}

/**
 * Draws a given block id to a specific coordinate on
 * the map.
 *
 * 	@see map_to_screen()
 */
void draw_block(cairo_t* cr, char* name, int x, int y, int z, unsigned char sides) {


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
		char* model_id = (char*)json_object_get_string(variant, "model");
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
		char* parent = (char*)json_object_get_string(root, "parent");
		if (parent == NULL) return;

		draw_model(cr, textures, sides, x, y, z, parent);

		free(model_path);
		json_value_free(model);
	}
}

/**
 * Draw a texture on given sides, at a given screen coordinate.
 */
void draw_texture(cairo_t* cr, char* name, int x, int y, unsigned char sides, int tint) {
	cairo_surface_t* surface = NULL;
	if (sides & TOP) {
		surface = render_side(name, TOP, tint);
		cairo_set_source_surface(cr, surface, x, y);
		cairo_paint(cr);
		cairo_surface_destroy(surface);
	}
	if (sides & LEFT) {
		surface = render_side(name, LEFT, tint);
		cairo_set_source_surface(cr, surface, x, y);
		cairo_paint(cr);
		cairo_surface_destroy(surface);
	}
	if (sides & RIGHT) {
		surface = render_side(name, RIGHT, tint);
		cairo_set_source_surface(cr, surface, x, y);
		cairo_paint(cr);
		cairo_surface_destroy(surface);
	}
}

void render_deform(cairo_t* iso_cr, cairo_surface_t* block, direction_t direction) {
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
}

void render_tint(cairo_t* block_cr, cairo_surface_t* block, int tint) {
	cairo_surface_t* tinted = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 16, 16);
	cairo_t* tinted_cr = cairo_create(tinted);

	// Copies the block texture to a new surface
	cairo_set_source_surface(tinted_cr, block, 0, 0);
	cairo_paint(tinted_cr);

	// Tints the surface
	double r = (tint & 0xFF0000) >> 16;
	double g = (tint & 0x00FF00) >> 8;
	double b = (tint & 0x0000FF);
	cairo_set_operator(tinted_cr, CAIRO_OPERATOR_MULTIPLY);
	cairo_rectangle(tinted_cr, 0, 0, 16, 16);
	cairo_set_source_rgba(tinted_cr, r/255, g/255, b/255, 1);
	cairo_fill(tinted_cr);

	// Pastes the tinted surface to the original one,
	// clipped by the texture. This is necessary because
	// otherwise, it would tint the parts of the images
	// that are transparent.
	cairo_set_operator(block_cr, CAIRO_OPERATOR_IN);
	cairo_set_source_surface(block_cr, tinted, 0, 0);
	cairo_paint(block_cr);

	cairo_surface_destroy(tinted);
	cairo_destroy(tinted_cr);
}

/**
 * Generate a Cairo surface of an isometric side
 * of a texture name.
 */
cairo_surface_t* render_side(char* name, direction_t direction, int tint) {
	char* path = get_block_path(name);
	cairo_surface_t* block = cairo_image_surface_create_from_png(path);
	cairo_surface_t* iso = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 32, 32);;
	cairo_t* iso_cr = cairo_create(iso);
	cairo_t* block_cr = cairo_create(block);

	if (tint != 0) render_tint(block_cr, block, tint);
	render_deform(iso_cr, block, direction);

	free(path);
	cairo_surface_destroy(block);
	cairo_destroy(block_cr);
	cairo_destroy(iso_cr);

	return iso;
}

/**
 * Entrypoint of the rendering process.
 * Renders to an isometric view the given chunk content.
 */
int render(chunk_t* chunk) {
	cairo_surface_t *surface;
	cairo_t *cr;
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, IMAGE_WIDTH, IMAGE_HEIGHT);
	cr = cairo_create(surface);
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);

	for (pos_t pos = 0; pos < 0x80FF; pos++) {
		char* block = chunk->blocks[pos];
		if (block == NULL) continue;
		unsigned char sides = 0;

		char* top_block = chunk->blocks[POS_ADD_Y(pos)];
		char* left_block = chunk->blocks[POS_ADD_Z(pos)];
		char* right_block = chunk->blocks[POS_ADD_X(pos)];

		if (POS_GET_Y(pos) == 255 || (top_block   != NULL && IS_AIR(top_block)))   sides |= TOP;
		if (POS_GET_Z(pos) == 15  || (left_block  != NULL && IS_AIR(left_block)))  sides |= LEFT;
		if (POS_GET_X(pos) == 15  || (right_block != NULL && IS_AIR(right_block))) sides |= RIGHT;

		if (sides == 0) continue;

		draw_block(cr, block, POS_GET_X(pos), POS_GET_Y(pos), POS_GET_Z(pos), sides);
	}

	cairo_surface_write_to_png(surface, "render.png");

	return 0;
}
