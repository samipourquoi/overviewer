#ifndef OVERVIEWER_NBT_H
#define OVERVIEWER_NBT_H

#define COMPOUND_LENGTH 50

typedef enum nbt_type {
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
} nbt_type;

typedef struct nbt_value {
	unsigned char* raw_bytes;
	union {
		signed char byte_value;
		short short_value;
		int int_value;
		long long_value;
		float float_value;
		double double_value;
		signed char* byte_array_value;
		char* string_value;
		struct compound_tag* list_value; // Lists are considered as compounds
		struct compound_tag* compound_value;
		int* int_array_value;
		long* long_array_value;
	};
	int array_length;
} nbt_value;

typedef struct nbt_tag {
	char* name;             // Name of the tag
	nbt_value* value;       // Value of the tag
	nbt_type type;          // Type of the tag
	struct nbt_tag* parent; // Pointer to the parent tag
} nbt_tag;

struct compound_tag {
	int size;
	int _max_size;
	struct nbt_tag* to_tag;
	struct nbt_tag** values;
};
typedef struct compound_tag compound_tag;

struct nbt_tag* cmpd_get_from_name(compound_tag* compound, char* name);
struct nbt_tag* cmpd_get_from_path(compound_tag* compound, char* path);
void cmpd_append_entry(compound_tag* compound, struct nbt_tag* tag);
struct compound_tag* nbt_parse_tree(const unsigned char* data, int length);
int nbt_free(struct compound_tag* compound);

#endif
