#include "nbt.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void read_tags_inside(const unsigned char* data, int* offset, int named, compound_tag* compound);

static long read_big_endian(unsigned char* data, int len) {
	long value = 0;
	for (int i = len; i > 0; i--) {
		value += data[i-1] * (long) pow(16, (len-i)*2);
	}
	free(data);
	return value;
}

static unsigned char* read_bytes(const unsigned char* data, int* offset, int n) {
	unsigned char* bytes = (unsigned char*) malloc(n * sizeof(char));
	for (int i = 0; i < n; i++) {
		bytes[i] = data[(*offset)++];
		// printf("%x ", bytes[i]);
	}
	// printf("\n");
	return bytes;
}

#define READ_NAME ( named? (char*) read_bytes(data, offset, (int) read_big_endian(read_bytes(data, offset, 2), 2)): NULL )
#define READ_BE(SIZE) ( read_big_endian(read_bytes(data, offset, SIZE), SIZE) )
#define CREATE_VALUE ( (nbt_value*) malloc(sizeof(nbt_value)) )
#define CREATE_TAG(NAME, VALUE, TYPE, PARENT)   nbt_tag* tag = malloc(sizeof(nbt_tag)); \
												tag->name = NAME; \
												tag->type = TYPE; \
												tag->parent = PARENT; \
												tag->value = VALUE;

static void read_int(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	nbt_value* value = CREATE_VALUE;
	value->int_value = READ_BE(4);
	CREATE_TAG(name, value, TAG_Int, compound->to_tag);
	cmpd_append_entry(compound, tag);
}

static void read_byte(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	nbt_value* value = CREATE_VALUE;
	value->byte_value = READ_BE(1);
	CREATE_TAG(name, value, TAG_Byte, compound->to_tag);
	cmpd_append_entry(compound, tag);
}

static void read_short(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	nbt_value* value = CREATE_VALUE;
	value->short_value = READ_BE(2);
	CREATE_TAG(name, value, TAG_Short, compound->to_tag);
	cmpd_append_entry(compound, tag);
}

static void read_long(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	nbt_value* value = CREATE_VALUE;
	value->long_value = READ_BE(8);
	CREATE_TAG(name, value, TAG_Long, compound->to_tag);
	cmpd_append_entry(compound, tag);
}

// TODO: float reading, fine for now as we don't need it
static void read_float(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	nbt_value* value = CREATE_VALUE;
	value->float_value = READ_BE(4);
	CREATE_TAG(name, value, TAG_Float, compound->to_tag);
	cmpd_append_entry(compound, tag);
}

// TODO: double reading, fine for now as we don't need it
static void read_double(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	nbt_value* value = CREATE_VALUE;
	value->double_value = READ_BE(8);
	CREATE_TAG(name, value, TAG_Double, compound->to_tag);
	cmpd_append_entry(compound, tag);
}

static void read_byte_array(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	int length = READ_BE(4);
	signed char* payload = malloc(length * sizeof(char)); //read_bytes(data, offset, length*4);
	for (int i = 0; i < length; i++) payload[i] = (char) READ_BE(1);

	nbt_value* value = CREATE_VALUE;
	value->byte_array_value = payload;
	value->array_length = length;
	CREATE_TAG(name, value, TAG_Byte, compound->to_tag);
	cmpd_append_entry(compound, tag);
}

static void read_string(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	int string_length = READ_BE(2);
	char* string = (char*) read_bytes(data, offset, string_length);

	nbt_value* value = CREATE_VALUE;
	value->string_value = string;
	value->array_length = string_length;
	CREATE_TAG(name, value, TAG_String, compound->to_tag);
	cmpd_append_entry(compound, tag);
}

static void read_list(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	nbt_value* value = CREATE_VALUE;
	char* name = READ_NAME;
	compound_tag* new_list = malloc(sizeof(compound_tag)); // Lists are considered as compounds
	CREATE_TAG(name, value, TAG_List, compound->to_tag);

	new_list->size = 0;
	new_list->to_tag = tag;
	value->list_value = new_list;

	cmpd_append_entry(compound, tag);
	read_tags_inside(data, offset, 0, new_list); // Reads the tags inside the compound
}

static void read_int_array(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	int length = READ_BE(4);
	int* payload = malloc(length * sizeof(int)); //read_bytes(data, offset, length*4);
	for (int i = 0; i < length; i++) payload[i] = READ_BE(4);

	nbt_value* value = CREATE_VALUE;
	value->int_array_value = payload;
	value->array_length = length;
	CREATE_TAG(name, value, TAG_Int_Array, compound->to_tag);
	cmpd_append_entry(compound, tag);
}

static void read_long_array(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	int length = READ_BE(4);
	long* payload = malloc(length * sizeof(long)); //read_bytes(data, offset, length*4);
	for (int i = 0; i < length; i++) payload[i] = READ_BE(8);

	nbt_value* value = CREATE_VALUE;
	value->long_array_value = payload;
	value->array_length = length;
	CREATE_TAG(name, value, TAG_Long_Array, compound->to_tag);
	cmpd_append_entry(compound, tag);
}

static void read_compound(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	nbt_value* value = CREATE_VALUE;
	char* name = READ_NAME;
	compound_tag* new_compound = malloc(sizeof(compound_tag));
	CREATE_TAG(name, value, TAG_Compound, compound->to_tag);

	new_compound->size = 0;
	new_compound->to_tag = tag;
	value->compound_value = new_compound;

	cmpd_append_entry(compound, tag);
	read_tags_inside(data, offset, 1, new_compound); // Reads the tags inside the compound
}

/**
 * Reads the next tag according to the offset and whether
 * or not the tags are named.
 *
 * @param data
 * @param offset
 * @param named
 * @return 1 if reaches a TAG_End, otherwise 0.
 */
static int read_following_tag(const unsigned char* data, int* offset, int named, int type, compound_tag* compound) {
	switch (type) {
		default:
		case TAG_End: return 1;
		case TAG_Byte: read_byte(data, offset, named, compound); break;
		case TAG_Short: read_short(data, offset, named, compound); break;
		case TAG_Int: read_int(data, offset, named, compound); break;
		case TAG_Long: read_long(data, offset, named, compound); break;
		case TAG_Float: read_float(data, offset, named, compound); break;
		case TAG_Double: read_double(data, offset, named, compound); break;
		case TAG_Byte_Array: read_byte_array(data, offset, named, compound); break;
		case TAG_String: read_string(data, offset, named, compound); break;
		case TAG_List: read_list(data, offset, named, compound); break;
		case TAG_Compound: read_compound(data, offset, named, compound); break;
		case TAG_Int_Array: read_int_array(data, offset, named, compound); break;
		case TAG_Long_Array: read_long_array(data, offset, named, compound); break;
	}
	return 0;
}

static void read_tags_inside(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	if (named) {
		while (!read_following_tag(data, offset, named, READ_BE(1), compound));
	} else {
		int list_type = READ_BE(1);
		int list_length = READ_BE(4);
		for (int i = 0; i < list_length; i++) {
			read_following_tag(data, offset, named, list_type, compound);
		}
	}
}

/**
 * Parses a nbt tree.
 *
 * @warning
 * THIS FUNCTION ONLY ACCEPTS WELL-FORMED NBT TREES.
 *
 * @param data Uncomrpessed nbt data
 * @param length Length of that data
 * @return
 */
compound_tag* nbt_parse_tree(const unsigned char* data, int length) {
	compound_tag root;
	nbt_tag root_tag;
	char* name = "root";

	root_tag.value = &(nbt_value) {};
	root_tag.value->compound_value = &root;
	root_tag.parent = NULL;
	root_tag.name = name;
	root_tag.type = TAG_Compound;

	root.size = 0;
	root.to_tag = &root_tag;

	// READ_BE doesn't work without having a pointer 'offset'
	int file_offset = 0;
	int *offset = &file_offset;

	read_tags_inside(data, offset, 1, &root);

	return root.values[0]->value->compound_value;
}

/**
 * Frees the memory of an NBT tree.
 *
 * @param compound The NBT tree you want to free
 * @return 0 is the freeing operations succeeded, 1 otherwise.
 */
int nbt_free(struct compound_tag* compound) {
	for (int i = 0; i < compound->size; i++) {
		nbt_tag* tag = compound->values[i];
		switch (tag->type) {
		case TAG_List:
			nbt_free(tag->value->list_value);
			free(tag->value->list_value);
			break;
		case TAG_Compound:
			nbt_free(tag->value->compound_value);
			free(tag->value->compound_value);
			break;
		case TAG_Byte_Array:
			free(tag->value->byte_array_value);
			break;
		case TAG_String:
			free(tag->value->string_value);
			break;
		case TAG_Int_Array:
			free(tag->value->int_array_value);
			break;
		case TAG_Long_Array:
			free(tag->value->long_array_value);
			break;
		}

		free(tag->name);
		free(tag->value);
		free(tag);
	}
}

#undef READ_NAME
#undef READ_BE
#undef CREATE_VALUE
#undef CREATE_TAG