#include "compound.h"
#include "nbt.h"
#include <stdio.h>
#include <string.h>

nbt_tag* get_tag_from_name(compound_tag* compound, char* name) {
	for (int i = 0; i < compound->size; i++) {
		if (strcmp((compound->values[i]->name), name) == 0) {
			return compound->values[i];
		}
	}
	return NULL;
}

void append_tag(compound_tag* compound, nbt_tag* tag) {
	compound->values[compound->size] = tag;
	compound->size++;
}