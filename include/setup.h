#ifndef _SETUP_H_
#define _SETUP_H_

#include <stdint.h>
#include <xmmintrin.h>        /* SSE instructions and _mm_malloc */
#include <emmintrin.h>        /* SSE2 instructions               */
#include <wmmintrin.h>

#define ZERO()          _mm_setzero_si128()
#define ONE _mm_set_epi32(0,0,0,1)
#define ADD_ONE(b)      _mm_add_epi32(b, ONE)
#define XOR(x, y)       _mm_xor_si128(x,y)
#define REDUCTION_POLYNOMIAL  _mm_set_epi32(0, 0, 0, 135)


#define TWO     _mm_set_epi32(0,0,0,2)
#define THREE   _mm_set_epi32(0,0,0,3)
#define FOUR    _mm_set_epi32(0,0,0,4)
#define FIVE    _mm_set_epi32(0,0,0,5)
#define SIX     _mm_set_epi32(0,0,0,6)
#define SEVEN   _mm_set_epi32(0,0,0,7)
#define EIGHT   _mm_set_epi32(0,0,0,8)
#define NINE    _mm_set_epi32(0,0,0,9)

/*
    computing:
    y_new = y_old + x[0] + x[1] + x[2] + x[3]
*/
#define accumulate_four_stateful(y, x) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    x[3] = XOR(x[0], x[2]); \
    y    = XOR(y, x[3]); \
}
#define accumulate_eight_stateful(y, x) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    x[4] = XOR(x[4], x[5]); \
    x[6] = XOR(x[6], x[7]); \
    x[0] = XOR(x[0], x[2]); \
    x[4] = XOR(x[4], x[6]); \
    x[3] = XOR(x[0], x[4]); \
    y    = XOR(y, x[3]); \
}

#define accumulate_four(x, y) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    y    = XOR(x[0], x[2]); \
}
#define accumulate_eight(x, y) { \
    x[0] = XOR(x[0], x[1]); \
    x[2] = XOR(x[2], x[3]); \
    x[4] = XOR(x[4], x[5]); \
    x[6] = XOR(x[6], x[7]); \
    x[0] = XOR(x[0], x[2]); \
    x[4] = XOR(x[4], x[6]); \
    y    = XOR(x[0], x[4]); \
}

#define accumulate_16_stateful(y, x) { \
    x[0]  = XOR(x[0], x[1]); \
    x[2]  = XOR(x[2], x[3]); \
    x[4]  = XOR(x[4], x[5]); \
    x[6]  = XOR(x[6], x[7]); \
    x[8]  = XOR(x[8], x[9]); \
    x[10] = XOR(x[10], x[11]); \
    x[12] = XOR(x[12], x[13]); \
    x[14] = XOR(x[14], x[15]); \
    x[0]  = XOR(x[0], x[2]); \
    x[4]  = XOR(x[4], x[6]); \
    x[8]  = XOR(x[8], x[10]); \
    x[12] = XOR(x[12], x[14]); \
    x[0]  = XOR(x[0], x[4]); \
    x[8]  = XOR(x[8], x[12]); \
    x[0]  = XOR(x[0], x[8]); \
    y     = XOR(y, x[0]); \
}

#if __GNUC__
    #define GCC_VERSION (__GNUC__ * 10 + __GNUC_MINOR__)
    #define ALIGN(n) __attribute__ ((aligned(n)))
    #define inline __inline__
    #define restrict __restrict__
#else /* Not GNU/Microsoft/C99: delete alignment/inline/restrict uses.     */
    #define ALIGN(n)
    #define inline
    #define restrict
#endif

typedef ALIGN(16) __m128i BLOCK;

int encrypt(const void *key,
            const void *pt,
            const void *tk,
            uint64_t    tk_len,
            void       *ct);

#endif  // _SETUP_H_
