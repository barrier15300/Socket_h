/*
 * AES impl
 * Copyright (C) 2025, Kernel
 */

#ifndef _AES_X86_H_
#define _AES_X86_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _AesContext2 {
	unsigned int key_size;
	unsigned int r;
	unsigned int k;
	unsigned char round_enc[0xF0] __attribute__((aligned(0x10)));
	unsigned char round_dec[0xF0] __attribute__((aligned(0x10)));
} AesContext2;

int AesInit(AesContext2 *ctx, const void *key, unsigned int key_size);

int AesEncrypt(AesContext2 *ctx, const void *src, void *dst);
int AesDecrypt(AesContext2 *ctx, const void *src, void *dst);

int AesEcbEncrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize);
int AesEcbDecrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize);
int AesCbcEncrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize, void *iv);
int AesCbcDecrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize, void *iv);
int AesCtrEncrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize, void *iv);
int AesCtrDecrypt(const void *src, void *dst, size_t length, const void *key, unsigned int keysize, void *iv);


#ifdef __cplusplus
}
#endif

#endif /* _AES_X86_H_ */
