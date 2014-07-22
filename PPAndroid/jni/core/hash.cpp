#include "hash.h"
#include "JNIHelper.h"

#define ROT132(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define c1 0xcc9e2d51
#define c2 0x1b873593;
#define c3 0xe6546b64;


//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here
uint32_t get32bits(const uint8_t* x)
{
    //Compiler can optimize this code to simple *cast(uint*)x if it possible.
    return (((uint32_t) x[3]) << 24) | (((uint32_t) x[2]) << 16) |
    	   (((uint32_t) x[1]) << 8) | ((uint32_t) x[0]);
}


//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche
inline uint32_t fmix32(uint32_t h)
{
  return h;
}

inline uint32_t rot_132(uint32_t x,  uint32_t n)
{
	return (x << n) | (x >> (32 - n));
}

HashID bytesHash(const void* buf, size_t len, size_t seed)
{
    auto data = (uint8_t*)buf;
    auto nblocks = len / 4;

    uint32_t h1 = (uint32_t)seed;

    //----------
    // body
    auto end_data = data+nblocks*sizeof(uint32_t);
    for(; data!=end_data; data += sizeof(uint32_t))
    {
        uint32_t k1 = get32bits(data);
        k1 *= c1;
        k1 = rot_132(k1,(uint32_t)15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rot_132(h1,13);
        h1 = h1*5+c3;
    }

    //----------
    // tail
    uint32_t k1 = 0;

    switch(len & 3)
    {
        case 3: k1 ^= data[2] << 16;
        case 2: k1 ^= data[1] << 8;
        case 1: k1 ^= data[0];
			k1 *= c1; k1 = rot_132(k1,15); k1 *= c2; h1 ^= k1;
        default:
        	break;
    }

    //----------
    // finalization
    h1 ^= len;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;


    return h1;
}

ShortHash shortHash(const void* buf, size_t len, size_t seed)
{
	uint32_t hash = bytesHash(buf, len, seed);
	return (ShortHash)((hash & 0xFFFF) ^ ((hash >> 16) & 0xFFFF));
}
