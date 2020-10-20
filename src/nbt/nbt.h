#ifndef OVERVIEWER_NBT_H
#define OVERVIEWER_NBT_H

#include "compound.h"

enum nbt_type {
	TAG_End         = 0,
	TAG_Byte        = 1,
	TAG_Short       = 2,
	TAG_Int         = 3,
	TAG_Long        = 4,
	TAG_Float       = 5,
	TAG_Double      = 6,
	TAG_Byte_Array  = 7,
	TAG_String      = 8,
	TAG_List        = 9,
	TAG_Compound    = 10,
	TAG_Int_Array   = 11,
	TAG_Long_Array  = 12
};
typedef enum nbt_type nbt_type;

struct nbt_tree {

};
typedef struct nbt_tree nbt_tree;

struct nbt_value {
	unsigned char* raw_bytes;
	union {
		unsigned char byte_value;
		short short_value;
		int int_value;
		long long_value;
		float float_value;
		double double_value;
		unsigned char* byte_array_value;
		char* string_value;
		struct nbt_tag* list_value;
		struct nbt_tag* compound_value;
		int* int_array_value;
		long* long_array_value;
	};
};
typedef struct nbt_value nbt_value;

struct nbt_tag {
	char* name;             // Name of the tag
	nbt_value* value;       // Value of the tag
	nbt_type type;          // Type of the tag
	struct nbt_tag* parent; // Pointer to the parent tag
};
typedef struct nbt_tag nbt_tag;

long read_big_endian(unsigned char* data, int len);

#endif
