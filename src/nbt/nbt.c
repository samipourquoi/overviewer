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
#define READ_BE(SIZE) ( (int) read_big_endian(read_bytes(data, offset, SIZE), SIZE) )
#define CREATE_TAG(NAME, VALUE, TYPE, PARENT)   nbt_tag* tag = malloc(sizeof(nbt_tag)); \
												tag->name = NAME; \
												tag->type = TYPE; \
												tag->parent = PARENT; \
												tag->value = VALUE;

static void read_int(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	nbt_value* value = malloc(sizeof(nbt_value));
	char* name = READ_NAME;
	value->int_value = READ_BE(4);

	CREATE_TAG(name, value, TAG_Int, compound->to_tag);
	append_tag(compound, tag);

	printf("int name: %s; payload: %d offset: %x\n", tag->name, tag->value->int_value, *offset);
}

static void read_byte(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME; int payload = READ_BE(1);
	printf("byte name: %s; payload: %d offset: %x\n", name, payload, *offset);
}

static void read_short(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME; int payload = READ_BE(2);
	printf("short name: %s; payload: %d offset: %x\n", name, payload, *offset);
}

static void read_long(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME; int payload = READ_BE(8);
	printf("long name: %s; payload: %d offset: %x\n", name, payload, *offset);
}

// TODO: float reading, fine for now as we don't need it
static void read_float(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME; int payload = READ_BE(4);
	printf("float name: %s; payload: %d offset: %x\n", name, payload, *offset);
}

// TODO: double reading, fine for now as we don't need it
static void read_double(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME; int payload = READ_BE(8);
	printf("double name: %s; payload: %d offset: %x\n", name, payload, *offset);
}

static void read_byte_array(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME; int payload_length = READ_BE(4);
	unsigned char* payload = read_bytes(data, offset, payload_length);
	printf("byte array name: %s; payload length: %d offset: %x\n", name, payload_length, *offset);
}

static void read_string(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME; int payload_length = READ_BE(2);
	unsigned char* payload = read_bytes(data, offset, payload_length);
	printf("string name: %s; payload: %s offset: %x\n", name, payload, *offset);
}

static void read_list(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	printf("list name: %s; offset: %x\n", name, *offset);
	read_tags_inside(data, offset, 0, compound); // Reads the tags inside the list
}

static void read_int_array(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	int length = READ_BE(4);
	printf("int array name: %s; length: %d; offset: %x\n", name, length, *offset);
	for (int i = 0; i < length; i++) {
		read_int(data, offset, 0, compound);
	}
}

static void read_long_array(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	int length = READ_BE(4);
	printf("long array name: %s; length: %d; offset: %x\n", name, length, *offset);
	for (int i = 0; i < length; i++) {
		read_long(data, offset, 0, compound);
	}
}

static void read_compound(const unsigned char* data, int* offset, int named, compound_tag* compound) {
	char* name = READ_NAME;
	nbt_tag* int_tag = malloc(sizeof(nbt_tag));
	nbt_value* compound_value = malloc(sizeof(nbt_value));
	compound_value->compound_value = compound_value;
	int_tag->name = name;
	int_tag->type = TAG_Int;
	int_tag->parent = compound->to_tag;
	int_tag->value = compound_value;
	append_tag(compound, int_tag);
	printf("compound name: %s\n", name);
	read_tags_inside(data, offset, 1, compound); // Reads the tags inside the compound
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
compound_tag* parse_tree(const unsigned char* data, int length) {
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

	// Reads the root compound, to avoid having
	// two nested root compounds.
	read_following_tag(data, offset, 1, READ_BE(1), &root);

	read_tags_inside(data, offset, 1, &root);

	printf("pointer to tag: %s\n", get_tag_from_name(&root, "A")->name);

	return NULL;
}

#undef READ_NAME
#undef READ_BE