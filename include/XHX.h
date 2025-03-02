#ifndef XHX_H
#define XHX_H
#include "aes.h"

static inline void polyHash1(const BLOCK *tkp, BLOCK K, BLOCK *res){
    BLOCK H1;

    gfmul(tkp[0], K, &H1);
    gfmul(H1, K, &H1);
    H1 = XOR(H1, K);

    *res = H1;
}
static inline void polyHash2(const BLOCK *tkp, BLOCK K, BLOCK *res){
    BLOCK H1;

    gfmul(tkp[0], K, &H1);

    H1 = XOR(H1, tkp[1]);
    gfmul(H1, K, &H1);

    gfmul(H1, K, &H1);
    H1 = XOR(H1, K);

    *res = H1;
}

static inline void polyHash3(const BLOCK *tkp, BLOCK K, BLOCK *res){
    BLOCK H1;

    gfmul(tkp[0], K, &H1);

    H1 = XOR(H1, tkp[1]);
    gfmul(H1, K, &H1);
        
    H1 = XOR(H1, tkp[2]);
    gfmul(H1, K, &H1);

    gfmul(H1, K, &H1);
    H1 = XOR(H1, K);
    
    *res = H1;
}
static inline void polyHash4(const BLOCK *tkp, BLOCK K, BLOCK *res){
    BLOCK H1;

    gfmul(tkp[0], K, &H1);

    H1 = XOR(H1, tkp[1]);
    gfmul(H1, K, &H1);

    H1 = XOR(H1, tkp[2]);
    gfmul(H1, K, &H1);

    H1 = XOR(H1, tkp[3]);
    gfmul(H1, K, &H1);

    gfmul(H1, K, &H1);
    H1 = XOR(H1, K);

    *res = H1;
}

static inline void XHX1_encrypt(const __m128i *keys, BLOCK P, BLOCK *C, const BLOCK *tk){
    BLOCK H1, H2;
    BLOCK round_keys[NUM_ROUND_KEYS];

    polyHash1(tk, keys[0], &H1);
    polyHash1(tk, keys[1], &H2);
 
    AES_set_encrypt_key(H2, round_keys);
    
    P = XOR(P, H1);
    AES_encrypt(P, C, round_keys);
    *C = XOR(*C, H1);
}

static inline void XHX2_encrypt(const __m128i *keys, BLOCK P, BLOCK *C, const BLOCK *tk){
    BLOCK H1, H2;
    BLOCK round_keys[NUM_ROUND_KEYS];

    polyHash2(tk, keys[0], &H1);
    polyHash2(tk, keys[1], &H2);
 
    AES_set_encrypt_key(H2, round_keys);
    
    P = XOR(P, H1);
    AES_encrypt(P, C, round_keys);
    *C = XOR(*C, H1);
}


static inline void XHX3_encrypt(const __m128i *keys, BLOCK P, BLOCK *C, const BLOCK *tk){
    BLOCK H1, H2;
    BLOCK round_keys[NUM_ROUND_KEYS];

    polyHash3(tk, keys[0], &H1);
    polyHash3(tk, keys[1], &H2);
 
    AES_set_encrypt_key(H2, round_keys);
    
    P = XOR(P, H1);
    AES_encrypt(P, C, round_keys);
    *C = XOR(*C, H1);
}

static inline void XHX4_encrypt(const __m128i *keys, BLOCK P, BLOCK *C, const BLOCK *tk){
    BLOCK H1, H2;
    BLOCK round_keys[NUM_ROUND_KEYS];

    polyHash4(tk, keys[0], &H1);
    polyHash4(tk, keys[1], &H2);
 
    AES_set_encrypt_key(H2, round_keys);
    
    P = XOR(P, H1);
    AES_encrypt(P, C, round_keys);
    *C = XOR(*C, H1);
}
#endif // XHX_H

