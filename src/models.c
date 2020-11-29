#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cairo/cairo.h>
#include "../parson/parson.h"
#include "models.h"
#include "render.h"

#define DRAW_ARGS cairo_t* cr, JSON_Object* textures, unsigned char sides, int screen_x, int screen_y
#define DRAW_GET_NAME(NAME) \
	char texture_name[50]; \
	const char* texture = (const char*)json_object_get_string(textures, NAME); \
	memcpy(texture_name, &texture[texture[9] == ':' /* minecraft: */ ? 16 : 6], 50);
#define DRAW_EASY(NAME, SIDES) { \
    DRAW_GET_NAME(NAME) \
	draw_texture(cr, texture_name, screen_x, screen_y, sides & (SIDES), 0 ); \
}

void draw_tinted_grass(DRAW_ARGS) {
	DRAW_GET_NAME("cross");
	char* path = get_block_path(texture_name);
	cairo_surface_t* block = cairo_image_surface_create_from_png(path);
	cairo_t* block_cr = cairo_create(block);
	render_tint(block_cr, block, 0x91BD59);

	cairo_set_source_surface(cr, block, screen_x + rand()%18, screen_y + TILE_TOP_HEIGHT/2);
	cairo_paint(cr);
	free(path);
	cairo_surface_destroy(block);
}

void draw_grass_block(DRAW_ARGS) {
	DRAW_EASY("side", LEFT | RIGHT);
	{
		DRAW_GET_NAME("top");
		draw_texture(cr, texture_name, screen_x, screen_y, sides & TOP, 0x91BD59);
	}
	{
		DRAW_GET_NAME("overlay");
		draw_texture(cr, texture_name, screen_x, screen_y, sides & (LEFT | RIGHT), 0x91BD59);
	}
}

void draw_model(cairo_t* cr, JSON_Object* textures, unsigned char sides, int x, int y, int z, char* parent) {
	int screen_x, screen_y;
	map_to_screen(x, y, z, &screen_x, &screen_y);

	if (strcmp(parent, "minecraft:block/cube_all") == 0) {

		DRAW_EASY("all", LEFT | RIGHT | TOP);
	} else if (strcmp(parent, "minecraft:block/cube") == 0) {

		DRAW_EASY("up", TOP);
		DRAW_EASY("south", LEFT);
		DRAW_EASY("east", RIGHT);
	} else if (strcmp(parent, "minecraft:block/cube_column") == 0 ||
	           strcmp(parent, "minecraft:block/cube_column_horizontal") == 0) {

		DRAW_EASY("end", TOP);
		DRAW_EASY("side", LEFT | RIGHT);
	} else if (strcmp(parent, "minecraft:block/cube_bottom_top") == 0) {

		DRAW_EASY("top", TOP);
		DRAW_EASY("side", LEFT | RIGHT);
	} else if (strcmp(parent, "block/block") == 0) {

		draw_grass_block(cr, textures, sides, screen_x, screen_y);
	} else if (strcmp(parent, "minecraft:block/tinted_cross") == 0) {

		draw_tinted_grass(cr, textures, sides, screen_x, screen_y);
	}
};
