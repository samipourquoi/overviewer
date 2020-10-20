#include "nbt.h"
#include "math.h"
#include <stdio.h>

long read_big_endian(unsigned char* data, int len) {
	long value = 0;
	for (int i = len; i > 0; i--) {
		// value += data[i] * (int) pow(16, len-i*2);
		value += data[i-1] * (long) pow(16, (len-i)*2);
		printf("%ld\n", value);
	}
	return value;
}