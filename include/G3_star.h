#ifndef G3_STAR_H
#define G3_STAR_H
#include "aes.h"

static inline void G3_STAR_encrypt(const __m128i *keys, BLOCK P, BLOCK *C, const BLOCK *tkp){
    
    /* const BLOCK *tkp = (BLOCK *)tk; */
    BLOCK y1, y2, y, K;
    BLOCK round_keys[NUM_ROUND_KEYS];

    K = XOR(keys[0], tkp[1]);
    AES_set_encrypt_key(K, round_keys);
    AES_encrypt(tkp[0], &y1, round_keys);

    K = XOR(y1, tkp[1]);
    AES_set_encrypt_key(K, round_keys);
    AES_encrypt(tkp[2], &y2, round_keys);
    
    y = XOR(y2, tkp[0]);
    K = XOR(keys[1], y);

    P = XOR(P, y1);
    AES_set_encrypt_key(K, round_keys);
    AES_encrypt(P, C, round_keys);
    *C = XOR(*C, y1);
}

#endif
