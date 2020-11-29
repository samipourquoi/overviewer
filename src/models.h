#ifndef OVERVIEWER_MODELS_H
#define OVERVIEWER_MODELS_H

#include <parson.h>

void models_init();
void draw_model(cairo_t* cr, JSON_Object* textures, unsigned char sides, int x, int y, int z, char* parent);

#endif
