#include "nbt.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Windows lacks that function, used by cmpd_get_from_path(). This is quite literally copied
// from stackoverflow though...
#if !(defined(_WIN32) || defined(_WIN64))

extern char* strsep(char** stringp, const char* delim) {
	if (*stringp == NULL) { return NULL; }
	char* token_start = *stringp;
	*stringp = strpbrk(token_start, delim);
	if (*stringp) {
		**stringp = '\0';
		(*stringp)++;
	}
	return token_start;
}

#endif

nbt_tag* cmpd_get_from_name(compound_tag* compound, char* name) {
	for (int i = 0; i < compound->size; i++) {
		if (strcmp((compound->values[i]->name), name) == 0) {
			return compound->values[i];
		}
	}
	return NULL;
}

nbt_tag* cmpd_get_from_path(compound_tag* compound, char* path) {
	// https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
	char* path_copy = strdup(path);
	char* token;
	compound_tag* current_compound = compound;
	while ((token = strsep(&path_copy, "."))) {
		nbt_tag* tag = cmpd_get_from_name(current_compound, token);
		if (tag == NULL) {
			printf("Not found...\n");
			free(path_copy);
			return NULL;
		}
		if (tag->type == TAG_Compound) {
			current_compound = tag->value->compound_value;
		} else if (tag->type == TAG_List) {
			current_compound = tag->value->list_value;
		} else {
			free(path_copy);
			return tag;
		}
	}
	free(path_copy);
	return current_compound->to_tag;
}

void cmpd_init(compound_tag* compound) {
	compound->_max_size = 20;
	compound->values = malloc(20 * sizeof(struct nbt_tag*));
	compound->size = 0;
	compound->to_tag = NULL;
}

void cmpd_append_entry(compound_tag* compound, nbt_tag* tag) {
	if (compound->size >= compound->_max_size) {
		compound->_max_size *= 2;
		compound->values = realloc(compound->values, compound->_max_size * sizeof(struct nbt_tag*));
	}
	compound->values[compound->size] = tag;
	compound->size++;
}

static void read_tags_inside(const unsigned char* data, int* offset, int named, compound_tag* compound);

static long read_big_endian(unsigned char* data, int len) {
	long value = 0;
	for (int i = len; i > 0; i--) {
		value += data[i - 1] * (long) pow(16, (len - i) * 2);
	}
	free(data);
	return value;
}

static unsigned char* read_bytes(const unsigned char* data, int* offset, int n) {
	unsigned char* bytes = (unsigned char*) calloc(n, sizeof(char));
	for (int i = 0; i < n; i++) {
		bytes[i] = data[(*offset)++];
		// printf("%x ", bytes[i]);
	}
	// printf("\n");
	return bytes;
}

#define READ_NAME ( named? (char*) read_bytes(data, offset, (int) read_big_endian(read_bytes(data, offset, 2), 2)): NULL )
#define READ_BE(SIZE) ( read_big_endian(read_bytes(data, offset, SIZE), SIZE) )
#define CREATE_VALUE ( (nbt_value*) calloc(1, sizeof(nbt_value)) )
#define CREATE_TAG(NAME, VALUE, TYPE, PARENT)   nbt_tag* tag = calloc(1, sizeof(nbt_tag)); \
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
	signed char* payload = calloc(length, sizeof(char)); //read_bytes(data, offset, length*4);
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
	char* string_raw = (char*) read_bytes(data, offset, string_length);
	char* string = malloc(string_length + 1);
	memcpy(string, string_raw, string_length);
	string[string_length] = '\0';
	free(string_raw);

	nbt_value* value = CREATE_VALUE;
	value->string_value = string;
	value->array_length = string_length;
	CREATE_TAG(name, value, TAG_String, compound->to_tag);
	cmpd_append_entry(compound, tag);
}

static void read_list(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	nbt_value* value = CREATE_VALUE;
	char* name = READ_NAME;
	compound_tag* new_list = malloc(1 * sizeof(compound_tag)); // Lists are considered as compounds
	cmpd_init(new_list);
	CREATE_TAG(name, value, TAG_List, compound->to_tag);

	new_list->to_tag = tag;
	value->list_value = new_list;

	cmpd_append_entry(compound, tag);
	read_tags_inside(data, offset, 0, new_list); // Reads the tags inside the compound
}

static void read_int_array(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	int length = READ_BE(4);
	int* payload = calloc(length, sizeof(int)); //read_bytes(data, offset, length*4);
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
	long* payload = calloc(length, sizeof(long)); //read_bytes(data, offset, length*4);
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
	compound_tag* new_compound = malloc(1 * sizeof(compound_tag));
	cmpd_init(new_compound);
	CREATE_TAG(name, value, TAG_Compound, compound->to_tag);

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
		case TAG_End:
			return 1;
		case TAG_Byte:
			read_byte(data, offset, named, compound);
			break;
		case TAG_Short:
			read_short(data, offset, named, compound);
			break;
		case TAG_Int:
			read_int(data, offset, named, compound);
			break;
		case TAG_Long:
			read_long(data, offset, named, compound);
			break;
		case TAG_Float:
			read_float(data, offset, named, compound);
			break;
		case TAG_Double:
			read_double(data, offset, named, compound);
			break;
		case TAG_Byte_Array:
			read_byte_array(data, offset, named, compound);
			break;
		case TAG_String:
			read_string(data, offset, named, compound);
			break;
		case TAG_List:
			read_list(data, offset, named, compound);
			break;
		case TAG_Compound:
			read_compound(data, offset, named, compound);
			break;
		case TAG_Int_Array:
			read_int_array(data, offset, named, compound);
			break;
		case TAG_Long_Array:
			read_long_array(data, offset, named, compound);
			break;
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
	compound_tag* root = calloc(1, sizeof(compound_tag));
	nbt_tag root_tag;
	char* name = "root";

	cmpd_init(root);
	root->to_tag = &root_tag;

	root_tag.value = &(nbt_value) {};
	root_tag.value->compound_value = root;
	root_tag.parent = NULL;
	root_tag.name = name;
	root_tag.type = TAG_Compound;

	// READ_BE doesn't work without having a pointer 'offset'
	int file_offset = 0;
	int* offset = &file_offset;

	read_tags_inside(data, offset, 1, root);

	return root->values[0]->value->compound_value;
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

	return 0;
}

#undef READ_NAME
#undef READ_BE
#undef CREATE_VALUE
#undef CREATE_TAG