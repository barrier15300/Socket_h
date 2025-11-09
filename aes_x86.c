/*
 * AES impl
 * Copyright (C) 2025, Kernel
 */

#include <string.h>
#include <wmmintrin.h>
#include "aes_x86.h"


#define Nb 4

static const uint8_t aes_rcon[11] = {
	0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

static const uint8_t aes_sbox[256] = {
	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
	0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
	0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
	0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
	0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
	0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
	0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
	0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
	0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
	0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
	0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
	0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
	0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
	0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
	0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

#define getSBoxValue(num) (aes_sbox[(num)])

static void aes_key_expansion(AesContext2 *ctx, uint8_t *RoundKey, const uint8_t *Key){

	unsigned i, j, k, n, c;
	uint8_t tempa[4];

	for(i=0;i<ctx->k;++i){
		RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
		RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
		RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
		RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
	}

	n = 0;
	c = 1;

	for(i=ctx->k;i<Nb*(ctx->r+1);++i){
		{
			k = (i - 1) * 4;
			tempa[0]=RoundKey[k + 0];
			tempa[1]=RoundKey[k + 1];
			tempa[2]=RoundKey[k + 2];
			tempa[3]=RoundKey[k + 3];
		}

		// if (i % ctx->Nk == 0)
		if(n == 0){
			n = ctx->k;

			{
				const uint8_t u8tmp = tempa[0];
				tempa[0] = tempa[1];
				tempa[1] = tempa[2];
				tempa[2] = tempa[3];
				tempa[3] = u8tmp;
			}

			{
				tempa[0] = getSBoxValue(tempa[0]);
				tempa[1] = getSBoxValue(tempa[1]);
				tempa[2] = getSBoxValue(tempa[2]);
				tempa[3] = getSBoxValue(tempa[3]);
			}

			// tempa[0] = tempa[0] ^ aes_rcon[i/ctx->Nk];
			tempa[0] = tempa[0] ^ aes_rcon[c++];
		}
		n--;

		// if (ctx->key_size == 256 && i % Nk == 4)
		if (ctx->key_size == 256 && (i & (ctx->k - 1)) == 4)
		{
			{
				tempa[0] = getSBoxValue(tempa[0]);
				tempa[1] = getSBoxValue(tempa[1]);
				tempa[2] = getSBoxValue(tempa[2]);
				tempa[3] = getSBoxValue(tempa[3]);
			}
		}

		j = i * 4; k=(i - ctx->k) * 4;
		RoundKey[j + 0] = RoundKey[k + 0] ^ tempa[0];
		RoundKey[j + 1] = RoundKey[k + 1] ^ tempa[1];
		RoundKey[j + 2] = RoundKey[k + 2] ^ tempa[2];
		RoundKey[j + 3] = RoundKey[k + 3] ^ tempa[3];
	}
}

int AesInit(AesContext2 *ctx, const void *key, unsigned int key_size){

	if(ctx == NULL || key == NULL || (((uintptr_t)ctx | (uintptr_t)key) & 0xF) != 0){
		return -1;
	}

	if((key_size & ~0x1C0) != 0 || (key_size - 1) >= 0x100){
		return -2;
	}

	static int g_rounds[4] = {
		0,  // 0x40 : invalid
		10, // 0x80 : 128 bit
		12, // 0xC0 : 192 bit
		14  // 0x100: 256 bit
	};

	ctx->key_size = key_size;
	ctx->r = g_rounds[(key_size >> 6) - 1];
	ctx->k = key_size >> 5;

	aes_key_expansion(ctx, ctx->round_enc, key);

	__m128i *renc = (__m128i *)(ctx->round_enc);
	__m128i *rdec = (__m128i *)(ctx->round_dec);

	_mm_storeu_si128(&(rdec[0]), _mm_loadu_si128(&(renc[0])));
	_mm_storeu_si128(&(rdec[ctx->r]), _mm_loadu_si128(&(renc[ctx->r])));

	for(int i=ctx->r-1;i>0;i--){
		_mm_storeu_si128(&(rdec[i]), _mm_aesimc_si128(_mm_loadu_si128(&(renc[i]))));
	}

	return 0;
}

__m128i _AesEncrypt(AesContext2 *ctx, __m128i src){

	const __m128i *rkeys = (const __m128i *)(ctx->round_enc);

	__m128i tmp = _mm_xor_si128(src, _mm_loadu_si128(rkeys++));

	for(int r=1;r<ctx->r;++r){
		tmp = _mm_aesenc_si128(tmp, _mm_loadu_si128(rkeys++));
	}

	return _mm_aesenclast_si128(tmp, _mm_loadu_si128(rkeys));
}

__m128i _AesDecrypt(AesContext2 *ctx, __m128i src){

	const __m128i *rkeys = (const __m128i *)(ctx->round_dec);

	__m128i tmp = _mm_xor_si128(src, _mm_loadu_si128(&(rkeys[ctx->r])));

	for(int i=ctx->r-1;i>0;i--){
		// tmp = _mm_aesdec_si128(tmp, _mm_aesimc_si128(_mm_loadu_si128(&(rkeys[i]))));
		tmp = _mm_aesdec_si128(tmp, _mm_loadu_si128(&(rkeys[i])));
	}

	return _mm_aesdeclast_si128(tmp, _mm_load_si128(&(rkeys[0])));
}

int AesEncrypt(AesContext2 *ctx, const void *src, void *dst){
	_mm_store_si128((__m128i *)dst, _AesEncrypt(ctx, _mm_load_si128((const __m128i *)src)));
	return 0;
}

int AesDecrypt(AesContext2 *ctx, const void *src, void *dst){
	_mm_store_si128((__m128i *)dst, _AesDecrypt(ctx, _mm_load_si128((const __m128i *)src)));
	return 0;
}

int AesEcbEncrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize){

	int res;
	AesContext2 ctx;

	if((((uintptr_t)src | (uintptr_t)dst | length) & 0xF) != 0){
		// printf("%s: align error. src=%p, dst=%p, length=0x%lX\n", __FUNCTION__, src, dst, length);
		return -1;
	}

	res = AesInit(&ctx, key, keysize);
	if(res < 0){
		return res;
	}

	for(size_t i=0;i<length;i+=0x10){
		AesEncrypt(&ctx, src + i, dst + i);
	}

	return 0;
}

int AesEcbDecrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize){

	int res;
	AesContext2 ctx;

	if((((uintptr_t)src | (uintptr_t)dst | length) & 0xF) != 0){
		// printf("%s: align error. src=%p, dst=%p, length=0x%lX\n", __FUNCTION__, src, dst, length);
		return -1;
	}

	res = AesInit(&ctx, key, keysize);
	if(res < 0){
		return res;
	}

	for(size_t i=0;i<length;i+=0x10){
		AesDecrypt(&ctx, src + i, dst + i);
	}

	return 0;
}

int AesCbcEncrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize, void *iv){

	int res;
	AesContext2 ctx;

	if((((uintptr_t)src | (uintptr_t)dst | length) & 0xF) != 0){
		// printf("%s: align error. src=%p, dst=%p, length=0x%lX\n", __FUNCTION__, src, dst, length);
		return -1;
	}

	res = AesInit(&ctx, key, keysize);
	if(res < 0){
		return res;
	}

	uint8_t _iv_aligned[0x10] __attribute__((aligned(0x10)));
	memcpy(_iv_aligned, iv, sizeof(_iv_aligned));

	__m128i __iv128 = _mm_load_si128((const __m128i *)_iv_aligned);

	for(int i=0;i<length;i+=0x10){
		__iv128 = _AesEncrypt(&ctx, _mm_xor_si128(_mm_load_si128((const __m128i *)(src + i)), __iv128));
		_mm_store_si128((__m128i *)(dst + i), __iv128);
	}

	_mm_store_si128((__m128i *)_iv_aligned, __iv128);
	memcpy(iv, _iv_aligned, sizeof(_iv_aligned));

	return 0;
}

int AesCbcDecrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize, void *iv){

	int res;
	AesContext2 ctx;

	if((((uintptr_t)src | (uintptr_t)dst | length) & 0xF) != 0){
		// printf("%s: align error. src=%p, dst=%p, length=0x%lX\n", __FUNCTION__, src, dst, length);
		return -1;
	}

	res = AesInit(&ctx, key, keysize);
	if(res < 0){
		return res;
	}

	uint8_t _iv_aligned[0x10] __attribute__((aligned(0x10)));
	memcpy(_iv_aligned, iv, sizeof(_iv_aligned));

	__m128i __iv128 = _mm_load_si128((const __m128i *)_iv_aligned);

	for(int i=0;i<length;i+=0x10){
		__m128i iv_store = _mm_load_si128((const __m128i *)src);

		_mm_store_si128((__m128i *)dst, _mm_xor_si128(_AesDecrypt(&ctx, iv_store), __iv128));

		__iv128 = iv_store;

		src = (const void *)((uintptr_t)src + 0x10);
		dst = (void *)((uintptr_t)dst + 0x10);
	}

	_mm_store_si128((__m128i *)_iv_aligned, __iv128);
	memcpy(iv, _iv_aligned, sizeof(_iv_aligned));

	return 0;
}

#include <stdio.h>

void hex_dump(const void *addr, int len){

	if(addr == NULL)
		return;

	if(len == 0)
		return;

	while(len >= 0x10){
		printf(
			"%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
			((unsigned char *)addr)[0x0], ((unsigned char *)addr)[0x1], ((unsigned char *)addr)[0x2], ((unsigned char *)addr)[0x3],
			((unsigned char *)addr)[0x4], ((unsigned char *)addr)[0x5], ((unsigned char *)addr)[0x6], ((unsigned char *)addr)[0x7],
			((unsigned char *)addr)[0x8], ((unsigned char *)addr)[0x9], ((unsigned char *)addr)[0xA], ((unsigned char *)addr)[0xB],
			((unsigned char *)addr)[0xC], ((unsigned char *)addr)[0xD], ((unsigned char *)addr)[0xE], ((unsigned char *)addr)[0xF]
		);
		addr = (void *)((uintptr_t)addr + 0x10);
		len -= 0x10;
	}

	if(len != 0){
		while(len >= 1){
			printf("%02X ", ((unsigned char *)addr)[0x0]);
			addr = (void *)((uintptr_t)addr + 1);
			len -= 1;
		}

		printf("\n");
	}
}

static __m128i add_iv(__m128i iv){

	uint64_t lower = __builtin_bswap64(_mm_cvtsi128_si64(_mm_srli_si128(iv, 8)));
	uint64_t upper = __builtin_bswap64(_mm_cvtsi128_si64(iv));

	// printf("old iv: %016llX%016llX\n", upper, lower);

	uint64_t s1 = lower;
	uint64_t s2 = 1ull;
	uint64_t val = s1 + s2;
	uint64_t carry = ((((s2 | s1) & ~val) | (s1 & s2)) >> 0x3F) & 1;

	// printf("new iv: %016llX%016llX\n", upper + carry, val);

	return _mm_set_epi64x(__builtin_bswap64(val), __builtin_bswap64(upper + carry));
}

int _AesCtrCrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize, void *iv){

	int res;
	AesContext2 ctx;

	if((((uintptr_t)src | (uintptr_t)dst) & 0xF) != 0){
		return -1;
	}

	res = AesInit(&ctx, key, keysize);
	if(res < 0){
		return res;
	}

	uint8_t _iv_aligned[0x10] __attribute__((aligned(0x10)));
	for(int i=0;i<0x10;i++){
		_iv_aligned[i] = ((unsigned char *)iv)[0xF - i];
	}

	__m128i __iv128 = _mm_load_si128((const __m128i *)_iv_aligned);

	size_t length_aligned = length & ~0xF;

	for(size_t i=0;i<length_aligned;i+=0x10){
		_mm_store_si128((__m128i *)dst, _mm_xor_si128(_AesEncrypt(&ctx, __iv128), _mm_load_si128((const __m128i *)src)));

		__iv128 = add_iv(__iv128);

		src = (const void *)((uintptr_t)src + 0x10);
		dst = (void *)((uintptr_t)dst + 0x10);
	}

	if((length & 0xF) != 0){

		size_t tail_len = length & 0xF;

		unsigned char src_last[0x10] __attribute__((aligned(0x10)));
		memset(src_last, 0, sizeof(src_last));
		memcpy(src_last, src, length & 0xF);

		_mm_store_si128((__m128i *)src_last, _mm_xor_si128(_AesEncrypt(&ctx, __iv128), _mm_load_si128((const __m128i *)src_last)));

		memcpy(dst, src_last, length & 0xF);

		__iv128 = add_iv(__iv128);
	}

	_mm_store_si128((__m128i *)_iv_aligned, __iv128);
	for(int i=0;i<0x10;i++){
		((unsigned char *)iv)[0xF - i] = _iv_aligned[i];
	}

	return 0;
}

int AesCtrEncrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize, void *iv){
	return _AesCtrCrypt(src, dst, length, key, keysize, iv);
}

int AesCtrDecrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize, void *iv){
	return _AesCtrCrypt(src, dst, length, key, keysize, iv);
}
