#ifndef OVERVIEWER_MODELS_H
#define OVERVIEWER_MODELS_H

#include <parson.h>
#include <stdbool.h>
#include <cairo/cairo.h>

typedef unsigned char sides_t;

typedef struct {
	char* key;
	char* value;
} blockstate_t;

typedef struct {
	char* texture;
	int resize[4];
	double scale_x;
	double scale_y;
	char* overlay;
	int tint_index;
} model_side_t;

typedef struct {
	double from[3];
	double to[3];

	char* up_name;
	char* south_name;
	char* east_name;
	model_side_t* up;
	model_side_t* south;
	model_side_t* east;
} model_element_t;

typedef struct {
	model_element_t** elements;
	int elements_amount;
	char** faces_name;
	int faces_amount;
	unsigned char culling;
	blockstate_t** blockstates;
	bool is_cube;
} model_t;

void draw_model(cairo_t* cr, model_t* model, unsigned char sides, int x, int y, int z);

void model_init(model_t* model);

void model_free(model_t* model);

bool model_culls_from(model_t* model, unsigned char side);

model_t* models_parse(model_t* model, JSON_Object* root);


#endif
