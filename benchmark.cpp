#include <x86intrin.h>
#include <immintrin.h>
#include <smmintrin.h>
#include <iostream>
#include <algorithm>
#include <random>
#include <bitset>
#include <cstring>
#include <iomanip>
#ifndef __PCLMUL__
#error "This code requires PCLMULQDQ support. Compile with -mpclmul."
#endif

#ifndef TWEAK_BLOCKS
#define TWEAK_BLOCKS 3
#endif

#include "./include/setup.h"
#include "./include/utility.h"
#include "./include/aes.h"
#include "./include/ZMAC.h"

// ---------------------------------------------------------------------

const size_t NUM_TIMER_SAMPLES(1000000);
const size_t PRE_WARM_UP_RUNS(128);
const size_t NUM_CONTINUOUS_RUNS(128);
const size_t NUM_REPETITIONS(128);
const size_t MEDIAN_REPETITIONS(NUM_REPETITIONS >> 1);

// ---------------------------------------------------------------------

void print_progress_bar(std::ostream &out, uint8_t progress) {
    std::ios_base::fmtflags fmt(out.flags());
    out << "\u2563";

    for (size_t i(0); i < 20; ++i) {
        if (progress / 5 > i) {
            out << "\u2588";
        } else {
            out << "\u2591";
        }
    }

    out << "\u2560";

    out << ' ' << std::setw(3) << (int) progress << " %";
    out.flags(fmt);
}

// ---------------------------------------------------------------------



static uint64_t get_timer_overhead() {
    uint64_t min_overhead = 0xffffffffffffffff;
    uint64_t start_time, end_time;

    for (size_t i = 0; i < NUM_TIMER_SAMPLES; ++i) {
#ifdef USE_RDTSC
        start_time = __rdtsc();
#else
        start_time = __builtin_ia32_rdpmc(0);
        _mm_lfence();
#endif

#ifdef USE_RDTSC
        end_time = __rdtsc();
#else
        _mm_lfence();
        end_time = __builtin_ia32_rdpmc(0);
#endif


        if (min_overhead > (end_time - start_time)) {
            min_overhead = end_time - start_time;
        }
    }

    return min_overhead;
}

// ---------------------------------------------------------------------

static double benchmark(const BLOCK *key, const char* label, uint64_t LEN, EncryptionFunction enc) {
    // ---------------------------------------------------------------------
    // Initialize progress bar
    // ---------------------------------------------------------------------

    uint8_t num_percent_done = 0;
    print_progress_bar(std::cout, num_percent_done);
    std::cout.flush();

    // ---------------------------------------------------------------------
    // Initialize timer
    // ---------------------------------------------------------------------

    uint64_t start_time;
    uint64_t end_time;
    const uint64_t timer_overhead = get_timer_overhead();
    // ---------------------------------------------------------------------
    // Initialize randomization
    // ---------------------------------------------------------------------

    std::random_device device;
    std::default_random_engine generator(device());
    std::uniform_int_distribution<uint8_t> distribution(0, UINT8_MAX);

    // ---------------------------------------------------------------------
    // Initialize buffers and results
    // Checksum prevents compiler optimizations
    // ---------------------------------------------------------------------
    double results[NUM_REPETITIONS];
    uint8_t check_sum = 0;
    uint8_t* plaintext = (uint8_t*)malloc(LEN * NUM_CONTINUOUS_RUNS);
    uint8_t* mac = (uint8_t*)malloc(32 * NUM_CONTINUOUS_RUNS);
    
    BLOCK *M = (BLOCK*)mac;
    
    //prepare domain seperation constants for ZMAC by encrypting 0 to 9 using 0 as tweak
    BLOCK DsC[10];
    const uint8_t tweakBytesToProcessDsC[NUM_BYTES_IN_TWEAK] = {0};
    const BLOCK *tweakToProcessDsC = (BLOCK*)tweakBytesToProcessDsC;
    encrypt(enc, key, NINE,    DsC + 0, tweakToProcessDsC);
    encrypt(enc, key, EIGHT,   DsC + 1, tweakToProcessDsC);
    encrypt(enc, key, SEVEN,   DsC + 2, tweakToProcessDsC);
    encrypt(enc, key, SIX,     DsC + 3, tweakToProcessDsC);
    encrypt(enc, key, FIVE,    DsC + 4, tweakToProcessDsC);
    encrypt(enc, key, FOUR,    DsC + 5, tweakToProcessDsC);
    encrypt(enc, key, THREE,   DsC + 6, tweakToProcessDsC);
    encrypt(enc, key, TWO,     DsC + 7, tweakToProcessDsC);
    encrypt(enc, key, ONE,     DsC + 8, tweakToProcessDsC);
    encrypt(enc, key, ZERO(),  DsC + 9, tweakToProcessDsC);

    for (size_t num_repetitions = 0; num_repetitions < NUM_REPETITIONS; ++num_repetitions) {
        size_t i;
        // ---------------------------------------------------------------------
        // Randomize buffer to prevent compiler optimizations
        // ---------------------------------------------------------------------

        for (i = 0; i < LEN * NUM_CONTINUOUS_RUNS; ++i) {
            plaintext[i] = distribution(generator);
        }

        // ---------------------------------------------------------------------
        // Load instructions into cache
        // ---------------------------------------------------------------------

#ifdef USE_RDTSC
        start_time = __rdtsc();
#else
        start_time = __builtin_ia32_rdpmc(0);
        _mm_lfence();
#endif

        for (i = 0; i < PRE_WARM_UP_RUNS; ++i) {
            zhash(plaintext, key, LEN, M, enc, DsC);
        }

#ifdef USE_RDTSC
        end_time = __rdtsc();
#else
        _mm_lfence();
        end_time = __builtin_ia32_rdpmc(0);
#endif
        // ---------------------------------------------------------------------
        // Benchmark
        // ---------------------------------------------------------------------

#ifdef USE_RDTSC
        start_time = __rdtsc();
#else
        start_time = __builtin_ia32_rdpmc(0);
        _mm_lfence();
#endif

        for (i = 0; i < NUM_CONTINUOUS_RUNS; ++i) {
            zhash(plaintext, key, LEN, M, enc, DsC);
        }

#ifdef USE_RDTSC
        end_time = __rdtsc();
#else
        _mm_lfence();
        end_time = __builtin_ia32_rdpmc(0);
#endif
        /* printf("%lu %lu\n", start_time, end_time); */
        results[num_repetitions] =
            ( 1.0 * ((double) (end_time - start_time - timer_overhead)) ) / NUM_CONTINUOUS_RUNS;
        
        for (i = 0; i < NUM_CONTINUOUS_RUNS; ++i) {
            check_sum ^= mac[i * 32];
        }

        // ---------------------------------------------------------------------
        // Update progress bar
        // ---------------------------------------------------------------------

        const size_t num_repetitions_percent =
            ((num_repetitions + 1) * 100) / NUM_REPETITIONS;

        if (num_repetitions_percent > num_percent_done) {
            num_percent_done = num_repetitions_percent;
            std::cout << "\r"
                << label
                << " "
                << std::setw(5)
                << LEN
                << " ";
            print_progress_bar(std::cout, num_percent_done);
            std::cout.flush();
        }
    }
    // ---------------------------------------------------------------------
    // Find median
    // ---------------------------------------------------------------------

    std::sort(results, results + NUM_REPETITIONS);
    const double media_num_cycles = results[MEDIAN_REPETITIONS];
    const double median_cpb = media_num_cycles / LEN;

    // ---------------------------------------------------------------------
    // Print results and the checksum to prevent the compiler from removing
    // the encryptions for optimization
    // ---------------------------------------------------------------------

    std::cout << " c "
              << std::setw(5)
              << median_cpb
              << " cpb ["
              << (int) check_sum << "]"
              << std::endl;

    // ---------------------------------------------------------------------
    // Clean memory
    // ---------------------------------------------------------------------

    free(plaintext);
    free(mac);
    return median_cpb;
}

//---------------------------------------------------------------------
void write_to_file(FILE* file, const char* dict_name,
                   uint64_t *keys, double *values, int num_keys) {
    fprintf(file, "%s = {\n", dict_name);   // Write dictionary name
    for (int i = 0; i < num_keys; i++) {
        int key = keys[i];
        double value = values[i];
        fprintf(file, "    %d: %.2f,\n", key/1024, value);  // Write key-value pair
    }
    fprintf(file, "}\n\n");   // Close dictionary
    
    //print data for latex table
    for (int i = 0; i < num_keys; i++) {
        double value = values[i];
        printf("%.2f & ", value);
    }
    printf("\n");
}

void getKeysXHX(const uint8_t *key, BLOCK *KL) {
    BLOCK K, L;
    BLOCK temp_index = ONE;
    BLOCK keysXHX[NUM_ROUND_KEYS];
    K = _mm_load_si128((__m128i*)key);
    AES_set_encrypt_key(K, keysXHX);
    AES_encrypt(temp_index, &L, keysXHX);
    KL[0] = K;
    KL[1] = L; 
}

void getKeysGr(const uint8_t *key, BLOCK *keys) {
    BLOCK KK[TWEAK_BLOCKS]; // K, K2, K4, K8
    KK[0] = _mm_load_si128((__m128i*)key);
    for(int i = 1; i < TWEAK_BLOCKS; i++) {
        Double(KK[i-1], KK + i);
    }

    for(int i = 0; i < TWEAK_BLOCKS; i++) {
        AES_set_encrypt_key(KK[i], keys + (i*NUM_ROUND_KEYS));
    }
}

void getKeysG3SorG2(const uint8_t *key, BLOCK *KK) {
    KK[0] = _mm_load_si128((__m128i*)key);
    Double(KK[0], KK + 1);
}
void getKeysF2(const uint8_t *key, BLOCK *keys) {
    keys[0] = _mm_load_si128((__m128i*)key);
    BLOCK L;
    Double(keys[0], &L);
    AES_set_encrypt_key(L, keys + 1);
}

// ---------------------------------------------------------------------
int main(int argc, char **argv) {
    // ---------------------------------------------------------------------
    // Set up key
    // ---------------------------------------------------------------------

    const uint8_t key[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };
    uint64_t index = 0, len;
    uint64_t LEN[8] = {1024, 2048, 4096, 8192, 16384, 32768, 65536, 0};

    printf("Benchmarking ZMAC where the Tweak Size of TBC is %d*n\n", TWEAK_BLOCKS);


    char filename[256];
    sprintf(filename, "./data/benchmark_TBC%d.py", TWEAK_BLOCKS);
    FILE* file = fopen(filename, "w"); // Open in append mode
    if (!file) {
        perror("Error opening file");
        return 1;
    }
    double cpbdata1[8], cpbdata2[8], cpbdata3[8], cpbdata4[8];

#if TWEAK_BLOCKS == 4
    BLOCK KL[2];
    BLOCK keys[TWEAK_BLOCKS*NUM_ROUND_KEYS];
    getKeysXHX(key, KL);
    getKeysGr(key, keys);

    len = LEN[index];
    while (len > 0) {
        cpbdata1[index] = benchmark(keys, "G4_encrypt  ", len, G4_encrypt);
        cpbdata2[index] = benchmark(KL,   "XHX4_encrypt", len, XHX4_encrypt);
        printf("\n");
        index++;
        len = LEN[index];
    }
    write_to_file(file, "G4_encrypt  ", LEN, cpbdata1, index );
    write_to_file(file, "XHX4_encrypt", LEN, cpbdata2, index );

#elif TWEAK_BLOCKS == 3
    BLOCK KL[2];
    getKeysXHX(key, KL);
    BLOCK K2K[2];
    getKeysG3SorG2(key, K2K);
    BLOCK keys[TWEAK_BLOCKS*NUM_ROUND_KEYS];
    getKeysGr(key, keys);

    len = LEN[index];
    while (len > 0) {
        cpbdata1[index] = benchmark(keys, "G3_encrypt  ", len, G3_encrypt);
        cpbdata2[index] = benchmark(keys, "Gr3_encrypt  ", len, Gr3_encrypt);
        cpbdata3[index] = benchmark(KL,   "XHX3_encrypt", len, XHX3_encrypt);
        cpbdata4[index] = benchmark(K2K,   "G3S_encrypt ", len, G3_STAR_encrypt);
        printf("\n");
        index++;
        len = LEN[index];
    }
    write_to_file(file, "G3_encrypt  ", LEN, cpbdata1, index );
    write_to_file(file, "Gr3_encrypt  ", LEN, cpbdata2, index );
    write_to_file(file, "XHX3_encrypt", LEN, cpbdata3, index );
    write_to_file(file, "G3S_encrypt ", LEN, cpbdata4, index );
#elif TWEAK_BLOCKS == 2
    BLOCK KL[2];
    getKeysXHX(key, KL);
    BLOCK K2K[2];
    getKeysG3SorG2(key, K2K);
    
    len = LEN[index];
    while (len > 0) {
        cpbdata1[index] = benchmark(K2K,  "G2_encrypt  ", len, G2_encrypt);
        cpbdata2[index] = benchmark(KL,   "XHX2_encrypt", len, XHX2_encrypt);
        printf("\n");
        index++;
        len = LEN[index];
    }
    write_to_file(file, "G2_encrypt  ", LEN, cpbdata1, index );
    write_to_file(file, "XHX2_encrypt", LEN, cpbdata2, index );

#elif TWEAK_BLOCKS == 1
    BLOCK KL[2];
    getKeysXHX(key, KL);

    BLOCK keys[NUM_ROUND_KEYS + 1];
    getKeysF2(key, keys);
    
    len = LEN[index];
    while (len > 0) {
        
        cpbdata1[index] =  benchmark(keys, "F1_encrypt", len, F1_encrypt);
        cpbdata2[index] =  benchmark(keys, "F2_encrypt",   len, F2_encrypt);
        cpbdata3[index] =  benchmark(KL,   "XHX1_encrypt",   len, XHX1_encrypt);
        printf("\n");
        index++;
        len = LEN[index];
    }
    write_to_file(file, "F1_encrypt", LEN, cpbdata1, index );
    write_to_file(file, "F2_encrypt", LEN, cpbdata2, index );
    write_to_file(file, "XHX1_encrypt", LEN, cpbdata3, index );
#endif

    // ---------------------------------------------------------------------
    // Clean memory
    // ---------------------------------------------------------------------

    fclose(file);
    (void)argc;
    (void)argv;
    return EXIT_SUCCESS;
}

