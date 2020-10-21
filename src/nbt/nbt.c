#include "nbt.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void read_new_tag(const unsigned char* data, int* offset);

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

static void read_int(const unsigned char* data, int* offset) {
	int name_length = (int) read_big_endian(read_bytes(data, offset, 2), 2);
	unsigned char* name = read_bytes(data, offset, name_length);
	int payload = (int) read_big_endian(read_bytes(data, offset, 4), 4);

	printf("int name: %s; int payload: %d offset: %x\n", name, payload, *offset);
}

static void read_byte(const unsigned char* data, int* offset) {
	int name_length = (int) read_big_endian(read_bytes(data, offset, 2), 2);
	unsigned char* name = read_bytes(data, offset, name_length);
	int payload = (int) read_big_endian(read_bytes(data, offset, 1), 1);

	printf("byte name: %s; int payload: %d offset: %x\n", name, payload, *offset);
}

static void read_short(const unsigned char* data, int* offset) {
	int name_length = (int) read_big_endian(read_bytes(data, offset, 2), 2);
	unsigned char* name = read_bytes(data, offset, name_length);
	int payload = (int) read_big_endian(read_bytes(data, offset, 2), 2);

	printf("short name: %s; int payload: %d offset: %x\n", name, payload, *offset);
}

static void read_long(const unsigned char* data, int* offset) {
	int name_length = (int) read_big_endian(read_bytes(data, offset, 2), 2);
	unsigned char* name = read_bytes(data, offset, name_length);
	int payload = (int) read_big_endian(read_bytes(data, offset, 8), 8);

	printf("short name: %s; int payload: %d offset: %x\n", name, payload, *offset);
}

// TODO: float reading, fine for now as we don't need it
static void read_float(const unsigned char* data, int* offset) {
	int name_length = (int) read_big_endian(read_bytes(data, offset, 2), 2);
	unsigned char* name = read_bytes(data, offset, name_length);
	int payload = (int) read_big_endian(read_bytes(data, offset, 4), 4);

	printf("float name: %s; int payload: %d offset: %x\n", name, payload, *offset);
}

// TODO: double reading, fine for now as we don't need it
static void read_double(const unsigned char* data, int* offset) {
	int name_length = (int) read_big_endian(read_bytes(data, offset, 2), 2);
	unsigned char* name = read_bytes(data, offset, name_length);
	int payload = (int) read_big_endian(read_bytes(data, offset, 8), 8);

	printf("double name: %s; int payload: %d offset: %x\n", name, payload, *offset);
}

static void read_byte_array(const unsigned char* data, int* offset) {
	int name_length = (int) read_big_endian(read_bytes(data, offset, 2), 2);
	unsigned char* name = read_bytes(data, offset, name_length);
	int payload_length = (int) read_big_endian(read_bytes(data, offset, 4), 4);
	unsigned char* payload = read_bytes(data, offset, payload_length);

	printf("byte array name: %s; payload length: %d offset: %x\n", name, payload_length, *offset);
}

static void read_string(const unsigned char* data, int* offset) {
	int name_length = (int) read_big_endian(read_bytes(data, offset, 2), 2);
	unsigned char* name = read_bytes(data, offset, name_length);
	int payload_length = (int) read_big_endian(read_bytes(data, offset, 2), 2);
	unsigned char* payload = read_bytes(data, offset, payload_length);

	printf("byte array name: %s; string: %s offset: %x\n", name, payload, *offset);
}

static void read_compound(const unsigned char* data, int* offset) {
	int name_length = (int) read_big_endian(read_bytes(data, offset, 2), 2);
	unsigned char* name = read_bytes(data, offset, name_length);
	printf("compound name: %s\n", name);

	// Reads the tag inside the compound
	read_new_tag(data, offset);
}

static void read_new_tag(const unsigned char* data, int* offset) {
	int type;
	do {
		type = (int) read_big_endian(read_bytes(data, offset, 1), 1);
		switch (type) {
		default:
		case TAG_End: return;
		case TAG_Byte: read_byte(data, offset); break;
		case TAG_Short: read_short(data, offset); break;
		case TAG_Int: read_int(data, offset); break;
		case TAG_Long: read_long(data, offset); break;
		case TAG_Float: read_float(data, offset); break;
		case TAG_Double: read_double(data, offset); break;
		case TAG_Byte_Array: read_byte_array(data, offset); break;
		case TAG_String: read_string(data, offset); break;
		case TAG_List: break;
		case TAG_Compound: read_compound(data, offset); break;
		case TAG_Int_Array: break;
		case TAG_Long_Array: break;
		}
	} while (1);
}

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

	read_new_tag(data, &offset);

	return NULL;
}