#ifndef G_H
#define G_H
#include "aes.h"

static inline void G2_encrypt(const __m128i *keys, BLOCK P, BLOCK *C, const BLOCK *tkp){

    /* const BLOCK *tkp = (BLOCK *)tk; */
    BLOCK y1, y2, K;
    BLOCK round_keys[NUM_ROUND_KEYS];

    K = XOR(tkp[0], keys[0]);
    AES_encrypt(K, &y1, keys);
    AES_encrypt(tkp[1], &y2, keys);

    P = XOR(P, y1);

    K = XOR(y2, K);
    AES_set_encrypt_key(K, round_keys);
    AES_encrypt(P, C, round_keys);

    *C = XOR(*C, y1);
}

static inline void G3_encrypt(const __m128i *keys, BLOCK P, BLOCK *C, const BLOCK *tkp){
    BLOCK y1, y2, y3, y, K;
    BLOCK round_keys[NUM_ROUND_KEYS];

    AES_encrypt(tkp[0], &y1, keys);
    AES_encrypt(tkp[1], &y2, keys+NUM_ROUND_KEYS);
    AES_encrypt(tkp[2], &y3, keys+(2*NUM_ROUND_KEYS));
    
    K = XOR(keys[0], tkp[0]);
    y = XOR(y2, y3);
    K = XOR(K, y);

    y = XOR(y1, y2);
    P = XOR(P, y);
    y = XOR(y1, y3);
    
    AES_set_encrypt_key(K, round_keys);
    AES_encrypt(P, C, round_keys);
    
    *C = XOR(*C, y);
}

static inline void Gr3_encrypt(const __m128i *keys, BLOCK P, BLOCK *C, const BLOCK *tkp){

    /* const BLOCK *tkp = (BLOCK *)tk; */
    BLOCK y1, y2, y3, s1, s2, K;
    BLOCK round_keys[NUM_ROUND_KEYS];

    K = keys[0];
    AES_encrypt(tkp[0], &y1, keys);
    AES_encrypt(tkp[1], &y2, keys+NUM_ROUND_KEYS);
    AES_encrypt(tkp[2], &y3, keys+(2*NUM_ROUND_KEYS));

    s1 = XOR(y1, y2);
    s1 = XOR(s1, y3);

    Double(y3, &s2);
    s2 = XOR(y2, s2);
    Double(s2, &s2);
    s2 = XOR(y1, s2);
    Double(s2, &s2);
    K = XOR(K, s2);

    P = XOR(P, s1);

    AES_set_encrypt_key(K, round_keys);
    AES_encrypt(P, C, round_keys);

    *C = XOR(*C, s1);
}
static inline void G4_encrypt(const __m128i *keys, BLOCK P, BLOCK *C, const BLOCK *tkp){

    /* const BLOCK *tkp = (BLOCK *)tk; */
    BLOCK y1, y2, y3, y4, s1, s2, K;
    BLOCK round_keys[NUM_ROUND_KEYS];

    K = keys[0];
    AES_encrypt(tkp[0], &y1, keys);
    AES_encrypt(tkp[1], &y2, keys+NUM_ROUND_KEYS);
    AES_encrypt(tkp[2], &y3, keys+(2*NUM_ROUND_KEYS));
    AES_encrypt(tkp[3], &y4, keys+(3*NUM_ROUND_KEYS));

    s1 = XOR(y1, y2);
    s2 = XOR(y3, y4);
    s1 = XOR(s1, s2);

    Double(y4, &s2);
    s2 = XOR(y3, s2);
    Double(s2, &s2);
    s2 = XOR(y2, s2);
    Double(s2, &s2);
    s2 = XOR(y1, s2);
    Double(s2, &s2);
    K = XOR(K, s2);

    P = XOR(P, s1);

    AES_set_encrypt_key(K, round_keys);
    AES_encrypt(P, C, round_keys);

    *C = XOR(*C, s1);
}

#endif
