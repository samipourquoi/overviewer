#ifndef OVERVIEWER_MODELS_H
#define OVERVIEWER_MODELS_H

#include "../parson/parson.h"

void draw_model(cairo_t* cr, JSON_Object* textures, unsigned char sides, int x, int y, int z, char* parent);

#endif
