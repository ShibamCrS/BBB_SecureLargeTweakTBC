#ifndef ZMAC_H
#define ZMAC_H
#include "XHX.h"
#include "Gr.h"
#include "Fr.h"
#include "G3_star.h"

// A chunk is a data of size (n+t) bits where n is the block size and t is the tweak size
#if TWEAK_BLOCKS == 4
    #define NUM_BYTES_IN_CHUNK 80   //5*128 bits = 80 bytes
    #define NUM_BYTES_IN_TWEAK 64   //4*128 bits = 64 bytes
    #define NUM_BLOCKS_PROCESSED_IN_EACH_TBC_CALL 5
#elif TWEAK_BLOCKS == 3
    #define NUM_BYTES_IN_CHUNK 64   //4*128 bits = 64 bytes
    #define NUM_BYTES_IN_TWEAK 48   //3*128 bits = 48 bytes
    #define NUM_BLOCKS_PROCESSED_IN_EACH_TBC_CALL 4
#elif TWEAK_BLOCKS == 2
    #define NUM_BYTES_IN_CHUNK 48   //3*128 bits = 48 bytes
    #define NUM_BYTES_IN_TWEAK 32   //2*128 bits = 32 bytes
    #define NUM_BLOCKS_PROCESSED_IN_EACH_TBC_CALL 3
#elif TWEAK_BLOCKS == 1
    #define NUM_BYTES_IN_CHUNK 32   //2*128 bits = 32 bytes
    #define NUM_BYTES_IN_TWEAK 16   //128 bits   = 16 bytes
    #define NUM_BLOCKS_PROCESSED_IN_EACH_TBC_CALL 2
#endif


// Define function pointer type
using EncryptionFunction = void(*)(const BLOCK *, BLOCK, BLOCK*, const BLOCK*);

// Template function that calls XHX_encrypt or G3_encrypt
template <typename EncryptionFunction>
void encrypt(EncryptionFunction enc, const BLOCK *K, BLOCK P, BLOCK *C, const BLOCK *tk) {
    (*enc)(K, P, C, tk);
}

void zhash(const uint8_t *data, const BLOCK *keys, uint64_t len, BLOCK *Y, EncryptionFunction enc, const BLOCK *DsC) {
    const size_t total_num_full_chunks = len / NUM_BYTES_IN_CHUNK;
    const size_t total_num_bytes_in_full_chunks = total_num_full_chunks*NUM_BYTES_IN_CHUNK;
    const size_t remaining_bytes = len % NUM_BYTES_IN_CHUNK;
    const size_t byte_index_of_last_chunk = total_num_bytes_in_full_chunks - NUM_BYTES_IN_CHUNK;
    
    const BLOCK *X = (BLOCK *)data;
    /* printf("%d %d %d %d \n", total_num_full_chunks, total_num_bytes_in_full_chunks, remaining_bytes, byte_index_of_last_chunk); */

    uint64_t index;
    BLOCK Ll = ZERO(), Lr = ZERO(), U = ZERO();
    BLOCK V[TWEAK_BLOCKS];

    for (int i = 0; i < TWEAK_BLOCKS; i++) {
        V[i] = ZERO();
    }
    
    Ll = XOR(Ll, DsC[0]);
    encrypt(enc, keys, Ll, &Ll, V);
    
    Lr = XOR(Lr, DsC[0]);
    V[TWEAK_BLOCKS - 1] = ONE; // 000...001
    encrypt(enc, keys, Lr, &Lr, V);

    V[TWEAK_BLOCKS - 1] = ZERO();
    

    BLOCK Sl, Sr[TWEAK_BLOCKS], Cl;
    index = 0;

    while (len >= NUM_BYTES_IN_CHUNK) {
        Sl    = XOR(Ll, X[index]); 
        Sr[0] = XOR(Lr, X[index + 1]);
        for (int i = 1; i < TWEAK_BLOCKS; i++) {
            Sr[i] = X[index + i + 1];
        }
        
        Double(Ll, &Ll);
        Double(Lr, &Lr);
        
        Sl = XOR(Sl, DsC[1]);
        encrypt(enc, keys, Sl, &Cl, Sr);
        
        U = XOR(U, Cl);
        Double(U, &U);;
        
        Sr[0] = XOR(Cl, X[index + 1]);
        for (int i = 0; i < TWEAK_BLOCKS; i++) {
            V[i] = XOR(V[i], Sr[i]);
        }

        index += NUM_BLOCKS_PROCESSED_IN_EACH_TBC_CALL;
        len   -= NUM_BYTES_IN_CHUNK;
    }

    if(remaining_bytes > 0) {
        uint8_t padded_final_chunk[NUM_BYTES_IN_CHUNK];
        memcpy(padded_final_chunk, data + total_num_bytes_in_full_chunks, remaining_bytes);
        padded_final_chunk[remaining_bytes] = 0x80;
        memset(padded_final_chunk + remaining_bytes + 1, 0x00, NUM_BYTES_IN_CHUNK - remaining_bytes -1);
        X = (BLOCK *)padded_final_chunk;

        Sl    = XOR(Ll, X[0]);
        Sr[0] = XOR(Lr, X[1]);
        for (int i = 1; i < TWEAK_BLOCKS; i++) {
            Sr[i] = X[i + 1];
        }

        Double(Ll, &Ll);
        Double(Lr, &Lr);

        Sl = XOR(Sl, DsC[1]);
        encrypt(enc, keys, Sl, &Cl, Sr);

        U = XOR(U, Cl);
        Double(U, &U);

        Sr[0] = XOR(Cl, X[1]);
        for (int i = 0; i < TWEAK_BLOCKS; i++) {
            V[i] = XOR(V[i], Sr[i]);
        }
        
        //ZFIN
        Sl = XOR(U, DsC[5]);
        encrypt(enc, keys, Sl, Sr, V);
        Sl = XOR(U, DsC[4]);
        encrypt(enc, keys, Sl, Sr+1, V);
        Y[0] = XOR(Sr[0], Sr[1]);
        
        Sl = XOR(U, DsC[3]);
        encrypt(enc, keys, Sl, Sr, V);
        Sl = XOR(U, DsC[2]);
        encrypt(enc, keys, Sl, Sr+1, V);
        Y[1] = XOR(Sr[0], Sr[1]);
    }
    else {
        //ZFIN
        Sl = XOR(U, DsC[9]);
        encrypt(enc, keys, Sl, Sr, V);
        Sl = XOR(U, DsC[8]);
        encrypt(enc, keys, Sl, Sr+1, V);
        Y[0] = XOR(Sr[0], Sr[1]);

        Sl = XOR(U, DsC[7]);
        encrypt(enc, keys, Sl, Sr, V);
        Sl = XOR(U, DsC[6]);
        encrypt(enc, keys, Sl, Sr+1, V);
        Y[1] = XOR(Sr[0], Sr[1]);
    }
}
#endif
