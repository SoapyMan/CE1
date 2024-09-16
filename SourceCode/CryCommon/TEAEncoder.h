#pragma once
// Tiny Encryption Algorithm

// src and trg can be the same pointer (in place encryption)
// len must be in bytes and must be multiple of 8 byts (64bits).
// key is 128bit:  int key[4] = {n1,n2,n3,n4};
// void encipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k )
static inline void TEA_ENCODE(unsigned int* src, unsigned int* trg, const int len, unsigned int* key)
{
	constexpr unsigned int delta = 0x9E3779B9;

	unsigned int* v = src, * w = trg, * k = key, nlen = len >> 3;
	unsigned int a = k[0], b = k[1], c = k[2], d = k[3];
	while (nlen--)
	{
		unsigned int y = v[0], z = v[1], n = 32, sum = 0;
		while (n-- > 0)
		{
			sum += delta;
			y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
			z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;
		}
		w[0] = y; 
		w[1] = z;
		v += 2, w += 2;
	}
}

// src and trg can be the same pointer (in place decryption)
// len must be in bytes and must be multiple of 8 byts (64bits).
// key is 128bit: int key[4] = {n1,n2,n3,n4};
// void decipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k)
static inline void TEA_DECODE(unsigned int* src, unsigned int* trg, const int len, unsigned int* key)
{
	constexpr unsigned int delta = 0x9E3779B9;
	constexpr unsigned int init_sum = 0xC6EF3720;

	unsigned int* v = src, * w = trg, * k = key, nlen = len >> 3;
	unsigned int a = k[0], b = k[1], c = k[2], d = k[3];
	while (nlen--) {
		unsigned int y = v[0], z = v[1], sum = init_sum, n = 32;
		while (n-- > 0)
		{
			z -= (y << 4) + c ^ y + sum ^ (y >> 5) + d;
			y -= (z << 4) + a ^ z + sum ^ (z >> 5) + b;
			sum -= delta;
		}
		w[0] = y; 
		w[1] = z; 
		v += 2, w += 2;
	}
}

// encode size ignore last 3 bits of size in bytes. (encode by 8bytes min)
#define TEA_GETSIZE( len ) ((len) & (~7))