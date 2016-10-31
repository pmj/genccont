/*
Copyright (c) 2011-2013 Phil Jordan <phil@philjordan.eu>

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#include "hash_shared.h"

/* integer hashes described in http://www.cris.com/~Ttwang/tech/inthash.htm */
size_t genc_hash_uint32(uint32_t key)
{
  key = ~key + (key << 15);
  key = key ^ (key >> 12);
  key = key + (key << 2);
  key = key ^ (key >> 4);
  key = key * 2057;
  key = key ^ (key >> 16);
  return key;
}

#ifdef __LP64__
/* 64 bit -> 64 bit hash */
size_t genc_hash_uint64(uint64_t key)
{
  key = (~key) + (key << 21);
  key = key ^ (key >> 24);
  key = (key + (key << 3)) + (key << 8);
  key = key ^ (key >> 14);
  key = (key + (key << 2)) + (key << 4);
  key = key ^ (key >> 28);
  key = key + (key << 31);
  return key;
}
#else
size_t genc_hash_uint64(uint64_t key)
{
  key = (~key) + (key << 18);
  key = key ^ (key >> 31);
  key = key * 21;
  key = key ^ (key >> 11);
  key = key + (key << 6);
  key = key ^ (key >> 22);
  return (size_t)key;
}
#endif


size_t genc_uint32_key_hash(void* item, void* opaque_unused GENC_UNUSED)
{
	return genc_hash_uint32(*(uint32_t*)item);
}
size_t genc_uint64_key_hash(void* item, void* opaque_unused GENC_UNUSED)
{
	return genc_hash_uint64(*(uint64_t*)item);
}
genc_bool_t genc_uint64_keys_equal(void* id1, void* id2, void* opaque_unused GENC_UNUSED)
{
	return *(uint64_t*)id1 == *(uint64_t*)id2;
}
genc_bool_t genc_uint32_keys_equal(void* id1, void* id2, void* opaque_unused GENC_UNUSED)
{
	return *(uint32_t*)id1 == *(uint32_t*)id2;
}

size_t genc_pointer_key_hash(void* key, void* opaque_unused)
{
	return genc_hash_size((uintptr_t)key);
}
genc_bool_t genc_pointer_keys_equal(void* key1, void* key2, void* opaque_unused)
{
	return key1 == key2;
}

size_t genc_hash_combine(size_t seed, size_t hash_value)
{
	seed ^= hash_value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	return seed;
}
