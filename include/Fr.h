#ifndef F_H
#define F_H
#include "aes.h"

static inline void F1_encrypt(const __m128i *keys, BLOCK P, BLOCK *C, const BLOCK *tk){
    BLOCK Z, K;
    BLOCK round_keys[NUM_ROUND_KEYS];

    gfmul(tk[0], keys[0], &Z);
    
    K = XOR(tk[0], keys[0]);
    AES_set_encrypt_key(K, round_keys);

    P = XOR(P, Z);
    AES_encrypt(P, C, round_keys);
    *C = XOR(*C, Z);
}

static inline void F2_encrypt(const __m128i *keys, BLOCK P, BLOCK *C, const BLOCK *tk){
    BLOCK Z, K;
    BLOCK round_keys[NUM_ROUND_KEYS];
    
    AES_encrypt(tk[0], &Z, keys + 1);

    K = XOR(keys[0], tk[0]);
    AES_set_encrypt_key(K, round_keys);

    P = XOR(P, Z);
    AES_encrypt(P, C, round_keys);
    *C = XOR(*C, Z);
}
#endif // F_H

