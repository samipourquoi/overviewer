#ifndef OVERVIEWER_PARSER_H
#define OVERVIEWER_PARSER_H

#include "nbt.h"
#include "list.h"

nbt_node* parse_chunk(unsigned char* data, int length);

#endif
