#ifndef OVERVIEWER_PARSER_H
#define OVERVIEWER_PARSER_H

#include "../nbt/nbt.h"

compound_tag* parse_chunk(unsigned char* data, int length);

#endif
