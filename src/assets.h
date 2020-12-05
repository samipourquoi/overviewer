#ifndef OVERVIEWER_ASSETS_H
#define OVERVIEWER_ASSETS_H

#include <hashtable.h>
#include "models.h"

HashTable* blockstates_list;

void assets_init();
model_t* assets_get_model(char* blockstate_name, blockstate_t** states);

#endif
