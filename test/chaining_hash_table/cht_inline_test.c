#include "../../src/chaining_hash_table.c"
#include <assert.h>

int main()
{
	assert(-1 == genc_log2_size(0));
	assert(sizeof(size_t) * 8 - 1 == genc_log2_size(SIZE_MAX));
	assert(0 == genc_log2_size(1));
	assert(1 == genc_log2_size(2));
	assert(1 == genc_log2_size(3));
	assert(2 == genc_log2_size(4));
	return 0;
}
