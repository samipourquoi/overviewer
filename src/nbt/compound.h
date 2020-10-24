#ifndef OVERVIEWER_COMPOUND_H
#define OVERVIEWER_COMPOUND_H

#define COMPOUND_LENGTH 50
#include "nbt.h"

struct compound_tag {
	int size;
	struct nbt_tag* to_tag;
	struct nbt_tag* values[COMPOUND_LENGTH]; // TODO: dynamic allocation
};
typedef struct compound_tag compound_tag;

struct nbt_tag* cmpd_get_from_name(compound_tag* compound, char* name);
struct nbt_tag* cmpd_get_from_path(compound_tag* compound, char* path);
void cmpd_append_entry(compound_tag* compound, struct nbt_tag* tag);

#endif
