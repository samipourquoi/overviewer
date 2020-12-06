#ifndef OVERVIEWER_ASSETS_H
#define OVERVIEWER_ASSETS_H

#include <hashtable.h>
#include <cairo/cairo.h>
#include "parse.h"
#include "models.h"
#include "render.h"

HashTable *blockstates_list;

void assets_init();

model_t *assets_get_model(char *blockstate_name, blockstate_t **states);

#endif
