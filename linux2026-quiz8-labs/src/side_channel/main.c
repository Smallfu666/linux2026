#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

uint32_t crc32_table(const uint8_t *data, size_t len);
uint32_t crc32_branchless(const uint8_t *data, size_t len);
void crc_init_table(void);
void crc_touch_table(void);
uint32_t *crc_table_base(void);

enum { DATA_LEN = 64, SAMPLES = 2000 };

static volatile uint32_t g_sink;

static uint64_t measure(uint32_t (*fn)(const uint8_t *, size_t), const uint8_t *data, size_t len) {
    _mm_mfence();
    uint64_t start = __rdtsc();
    g_sink ^= fn(data, len);
    _mm_mfence();
    uint64_t end = __rdtsc();
    return end - start;
}

static void flush_crc_table(void) {
    uint32_t *table = crc_table_base();
    for (size_t i = 0; i < 256; ++i) {
        _mm_clflush(&table[i]);
    }
    _mm_mfence();
}

static double sample_average(uint32_t (*fn)(const uint8_t *, size_t), const uint8_t *data, size_t len, int flush_table) {
    uint64_t total = 0;
    for (int i = 0; i < SAMPLES; ++i) {
        if (flush_table) {
            flush_crc_table();
        } else {
            crc_touch_table();
        }
        total += measure(fn, data, len);
    }
    return (double)total / (double)SAMPLES;
}

int main(void) {
    uint8_t data[DATA_LEN];
    for (size_t i = 0; i < sizeof(data); ++i) {
        data[i] = (uint8_t)(i * 3u + 7u);
    }

    crc_init_table();
    crc_touch_table();

    double table_hot = sample_average(crc32_table, data, sizeof(data), 0);
    double table_cold = sample_average(crc32_table, data, sizeof(data), 1);
    double branchless_hot = sample_average(crc32_branchless, data, sizeof(data), 0);
    double branchless_cold = sample_average(crc32_branchless, data, sizeof(data), 1);

    printf("table_lookup_hot_cycles=%.2f\n", table_hot);
    printf("table_lookup_cold_cycles=%.2f\n", table_cold);
    printf("branchless_hot_cycles=%.2f\n", branchless_hot);
    printf("branchless_cold_cycles=%.2f\n", branchless_cold);
    printf("sink=%u\n", g_sink);
    return 0;
}
