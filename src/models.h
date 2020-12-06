#ifndef OVERVIEWER_MODELS_H
#define OVERVIEWER_MODELS_H

#include <parson.h>
#include "parse.h"

typedef unsigned char sides_t;

typedef struct {
	char *texture;
	int resize[4];
	double scale_x;
	double scale_y;
} model_side_t;

typedef struct {
	double from[3];
	double to[3];

	char *up_name;
	char *south_name;
	char *east_name;
	model_side_t *up;
	model_side_t *south;
	model_side_t *east;
} model_element_t;

typedef struct {
	model_element_t **elements;
	int elements_amount;
	char **faces_name;
	int faces_amount;
	unsigned char culling;
	blockstate_t **blockstates;
} model_t;

void draw_model(cairo_t *cr, model_t *model, unsigned char sides, int x, int y, int z);

void model_init(model_t *model);

void model_free(model_t *model);

model_t *models_parse(model_t *model, JSON_Object *root);


#endif
