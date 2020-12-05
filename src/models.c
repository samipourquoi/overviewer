#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cairo/cairo.h>
#include <parson.h>
#include <stdbool.h>
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

void model_init(model_t* model) {
	model->elements = calloc(20, sizeof(model_element_t*));
	model->faces_name = calloc(20, sizeof(char*));
	model->elements_amount = 0;
	model->faces_amount = 0;
	model->culling = 0;
	model->blockstates = NULL;
}

void model_free(model_t* model) {
	free(model->elements);
	free(model);
}

bool models_insert_face_name(model_t* model, char* name) {
	bool present = false;
	for (int i = 0; i < model->faces_amount; i++) {
		char* face_name = model->faces_name[i];
		if (face_name == NULL) break;
		if (strcmp(face_name, name) == 0) {
			present = true;
			break;
		}
	}

	if (!present) {
		// Gets free()'d otherwise.
		char* persistent_name = malloc(strlen(name)+1);
		strcpy(persistent_name, name);
		model->faces_name[model->faces_amount++] = persistent_name;
	}

	return present;
}

void models_parse_texture(model_side_t* side, JSON_Object* textures, char** side_name) {
	char* side_id = (char*)json_object_get_string(textures, *side_name);
	if (side_id != NULL) {
		if (side_id[0] == '#') {
			free(*side_name);
			*side_name = malloc(strlen(side_id) + 1 /* \0 */ - 1 /* 1 */);
			strcpy(*side_name, &side_id[1]);
		} else {
			char* persistent = malloc(strlen(side_id) + 1);
			strcpy(persistent, side_id);
			side->texture = persistent;
		}
	}
}

void models_parse_textures(model_t* model, JSON_Object* textures) {
	for (int i = 0; i < model->elements_amount; i++) {
		model_element_t* element = model->elements[i];
		if (element == NULL) break;
		if (element->up_name != NULL) {
			models_parse_texture(element->up, textures, &element->up_name);
		}
		if (element->south_name != NULL) {
			models_parse_texture(element->south, textures, &element->south_name);
		}
		if (element->east_name != NULL) {
			models_parse_texture(element->east, textures, &element->east_name);
		}
	}
}

void models_parse_side(model_t* model, model_element_t* element, JSON_Object* face, char* face_name) {
	model_side_t* side = malloc(sizeof(model_side_t));
	char* direction_tag = (char*)json_object_get_string(face, "texture");

	// Gets free()'d otherwise.
	char* dir_tag = malloc(strlen(direction_tag) + 1 /* \0 */ - 1 /* # */);
	strcpy(dir_tag, &direction_tag[1]);
	if (strcmp(face_name, "up") == 0) {
		element->up = side;
		element->up_name = dir_tag;
	} else if (strcmp(face_name, "south") == 0) {
		element->south = side;
		element->south_name = dir_tag;
	} else if (strcmp(face_name, "east") == 0) {
		element->east = side;
		element->east_name = dir_tag;
	} else {
		return;
	}

	models_insert_face_name(model, dir_tag);
}

model_element_t* models_parse_element(model_t* model, JSON_Object* element_json) {
	model_element_t* element = calloc(1, sizeof(model_element_t));
	model->elements_amount++;
	JSON_Array* from = json_object_get_array(element_json, "from");
	JSON_Array* to = json_object_get_array(element_json, "to");

	#define X 0
	#define Y 1
	#define Z 2

	element->from[X] = json_array_get_number(from, X);
	element->from[Y] = json_array_get_number(from, Y);
	element->from[Z] = json_array_get_number(from, Z);
	element->to[X] = json_array_get_number(to, X);
	element->to[Y] = json_array_get_number(to, Y);
	element->to[Z] = json_array_get_number(to, Z);

	JSON_Object* faces = json_object_get_object(element_json, "faces");
	size_t faces_length = json_object_get_count(faces);
	for (int j = 0; j < faces_length; j++) {
		JSON_Object* face = json_value_get_object(json_object_get_value_at(faces, j));
		char* face_name = (char*)json_object_get_name(faces, j);
		models_parse_side(model, element, face, face_name);
	}

	#undef X
	#undef Y
	#undef Z

	return element;
}

model_t* models_parse(model_t* model, JSON_Object* root) {
	char* parent = (char*)json_object_get_string(root, "parent");
	if (parent != NULL && strcmp(parent, "block/block") != 0) {
		char* parent_path = malloc(26 + strlen(parent));
		strcpy(parent_path, "assets/models/");
		strcat(parent_path, &parent[parent[9] == ':' ? 10 : 0]);
		strcat(parent_path, ".json");
		JSON_Value* parent_value = json_parse_file(parent_path);
		JSON_Object* parent_model = json_value_get_object(parent_value);

		free(parent_path);
		models_parse(model, parent_model);
		json_value_free(parent_value);
	}

	JSON_Array* elements_array = json_object_get_array(root, "elements");
	if (elements_array != NULL) {
		size_t length = json_array_get_count(elements_array);
		for (int i = 0; i < length; i++) {
			JSON_Object* element_json = json_array_get_object(elements_array, i);
			model_element_t* element = models_parse_element(model, element_json);
			model->elements[i] = element;
		}
	}

	JSON_Object* textures = json_object_get_object(root, "textures");
	if (textures != NULL) {
		models_parse_textures(model, textures);
	}

	return model;
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
