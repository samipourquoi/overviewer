#include "nbt.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void read_tags_inside(const unsigned char* data, int* offset, int named);

static long read_big_endian(unsigned char* data, int len) {
	long value = 0;
	for (int i = len; i > 0; i--) {
		value += data[i-1] * (long) pow(16, (len-i)*2);
	}
	return value;
}

static unsigned char* read_bytes(const unsigned char* data, int* offset, int n) {
	unsigned char* bytes = malloc(n);
	for (int i = 0; i < n; i++) {
		bytes[i] = data[(*offset)++];
		// printf("%x ", bytes[i]);
	}
	// printf("\n");
	return bytes;
}

#define READ_NAME ( named? read_bytes(data, offset, (int) read_big_endian(read_bytes(data, offset, 2), 2)): NULL )
#define READ_BE(SIZE) ( (int) read_big_endian(read_bytes(data, offset, SIZE), SIZE) )

static void read_int(const unsigned char* data, int* offset, int named) {
	unsigned char* name = READ_NAME; int payload = READ_BE(4);
	printf("int name: %s; payload: %d offset: %x\n", name, payload, *offset);
}

static void read_byte(const unsigned char* data, int* offset, int named) {
	unsigned char* name = READ_NAME; int payload = READ_BE(1);
	printf("byte name: %s; payload: %d offset: %x\n", name, payload, *offset);
}

static void read_short(const unsigned char* data, int* offset, int named) {
	unsigned char* name = READ_NAME; int payload = READ_BE(2);
	printf("short name: %s; payload: %d offset: %x\n", name, payload, *offset);
}

static void read_long(const unsigned char* data, int* offset, int named) {
	unsigned char* name = READ_NAME; int payload = READ_BE(8);
	printf("long name: %s; payload: %d offset: %x\n", name, payload, *offset);
}

// TODO: float reading, fine for now as we don't need it
static void read_float(const unsigned char* data, int* offset, int named) {
	unsigned char* name = READ_NAME; int payload = READ_BE(4);
	printf("float name: %s; payload: %d offset: %x\n", name, payload, *offset);
}

// TODO: double reading, fine for now as we don't need it
static void read_double(const unsigned char* data, int* offset, int named) {
	unsigned char* name = READ_NAME; int payload = READ_BE(8);
	printf("double name: %s; payload: %d offset: %x\n", name, payload, *offset);
}

static void read_byte_array(const unsigned char* data, int* offset, int named) {
	unsigned char* name = READ_NAME; int payload_length = READ_BE(4);
	unsigned char* payload = read_bytes(data, offset, payload_length);
	printf("byte array name: %s; payload length: %d offset: %x\n", name, payload_length, *offset);
}

static void read_string(const unsigned char* data, int* offset, int named) {
	unsigned char* name = READ_NAME; int payload_length = READ_BE(2);
	unsigned char* payload = read_bytes(data, offset, payload_length);
	printf("string name: %s; payload: %s offset: %x\n", name, payload, *offset);
}

static void read_list(const unsigned char* data, int* offset, int named) {
	unsigned char* name = READ_NAME;
	printf("list name: %s; offset: %x\n", name, *offset);
	read_tags_inside(data, offset, 0); // Reads the tags inside the list
}

static void read_int_array(const unsigned char* data, int* offset, int named) {
	unsigned char* name = READ_NAME;
	int length = READ_BE(4);
	printf("int array name: %s; length: %d; offset: %x\n", name, length, *offset);
	for (int i = 0; i < length; i++) {
		read_int(data, offset, 0);
	}
}

static void read_long_array(const unsigned char* data, int* offset, int named) {
	unsigned char* name = READ_NAME;
	int length = READ_BE(4);
	printf("long array name: %s; length: %d; offset: %x\n", name, length, *offset);
	for (int i = 0; i < length; i++) {
		read_long(data, offset, 0);
	}
}

static void read_compound(const unsigned char* data, int* offset, int named) {
	unsigned char* name = READ_NAME;
	printf("compound name: %s\n", name);
	read_tags_inside(data, offset, 1); // Reads the tags inside the compound
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
static int read_following_tag(const unsigned char* data, int* offset, int named, int type) {
	switch (type) {
		default:
		case TAG_End: return 1;
		case TAG_Byte: read_byte(data, offset, named); break;
		case TAG_Short: read_short(data, offset, named); break;
		case TAG_Int: read_int(data, offset, named); break;
		case TAG_Long: read_long(data, offset, named); break;
		case TAG_Float: read_float(data, offset, named); break;
		case TAG_Double: read_double(data, offset, named); break;
		case TAG_Byte_Array: read_byte_array(data, offset, named); break;
		case TAG_String: read_string(data, offset, named); break;
		case TAG_List: read_list(data, offset, named); break;
		case TAG_Compound: read_compound(data, offset, named); break;
		case TAG_Int_Array: read_int_array(data, offset, named); break;
		case TAG_Long_Array: read_long_array(data, offset, named); break;
	}
	return 0;
}

static void read_tags_inside(const unsigned char* data, int* offset, int named) {
	if (named) {
		while (!read_following_tag(data, offset, named, READ_BE(1)));
	} else {
		int list_type = READ_BE(1);
		int list_length = READ_BE(4);
		for (int i = 0; i < list_length; i++) {
			read_following_tag(data, offset, named, list_type);
		}
	}
}

#undef READ_NAME
#undef READ_BE

/**
 * Parses a nbt tree.
 *
 * @warning
 * THIS FUNCTION ONLY ACCEPTS WELL-FORMED NBT TREES.
 *
 * @param data
 * @param length
 * @return
 */
compound_tag* parse_tree(const unsigned char* data, int length) {
	compound_tag root;
	root.size = 0;
	int offset = 0;

	read_tags_inside(data, &offset, 1);

	return NULL;
}