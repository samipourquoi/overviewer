#include <string.h>
#include "compound.h"
#include "nbt.h"

nbt_tag* cmpd_get_from_name(compound_tag* compound, char* name) {
	for (int i = 0; i < compound->size; i++) {
		if (strcmp((compound->values[i]->name), name) == 0) {
			return compound->values[i];
		}
	}
	return NULL;
}

void cmpd_append_entry(compound_tag* compound, nbt_tag* tag) {
	compound->values[compound->size] = tag;
	compound->size++;
}