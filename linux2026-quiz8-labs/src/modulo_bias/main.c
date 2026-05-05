#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint64_t state;
} rng_t;

static uint32_t xorshift32(rng_t *rng) {
    uint32_t x = (uint32_t)rng->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng->state = (uint64_t)x;
    return x;
}

static uint32_t lemire_rejection(rng_t *rng, uint32_t n) {
    uint32_t threshold = (uint32_t)(-n) % n;
    for (;;) {
        uint32_t x = xorshift32(rng);
        uint64_t m = (uint64_t)x * (uint64_t)n;
        uint32_t l = (uint32_t)m;
        if (l >= threshold) {
            return (uint32_t)(m >> 32);
        }
    }
}

static void zero_counts(uint64_t *counts, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        counts[i] = 0;
    }
}

int main(int argc, char **argv) {
    const char *out_path = argc > 1 ? argv[1] : "results/modulo_bias.csv";
    uint64_t samples = argc > 2 ? strtoull(argv[2], NULL, 10) : 100000000ULL;
    uint32_t n = argc > 3 ? (uint32_t)strtoul(argv[3], NULL, 10) : 1009u;

    uint64_t *raw_counts = calloc(n, sizeof(uint64_t));
    uint64_t *lemire_counts = calloc(n, sizeof(uint64_t));
    if (!raw_counts || !lemire_counts) {
        fprintf(stderr, "allocation failure\n");
        free(raw_counts);
        free(lemire_counts);
        return 1;
    }

    rng_t rng = { .state = 0x12345678u };
    zero_counts(raw_counts, n);
    zero_counts(lemire_counts, n);

    for (uint64_t i = 0; i < samples; ++i) {
        uint32_t x = xorshift32(&rng);
        raw_counts[x % n]++;
        lemire_counts[lemire_rejection(&rng, n)]++;
    }

    FILE *fp = fopen(out_path, "w");
    if (!fp) {
        perror("fopen");
        free(raw_counts);
        free(lemire_counts);
        return 1;
    }

    fprintf(fp, "method,value,count\n");
    for (uint32_t i = 0; i < n; ++i) {
        fprintf(fp, "raw,%u,%llu\n", i, (unsigned long long)raw_counts[i]);
        fprintf(fp, "lemire,%u,%llu\n", i, (unsigned long long)lemire_counts[i]);
    }
    fclose(fp);

    printf("wrote %s\n", out_path);
    printf("samples=%llu n=%u\n", (unsigned long long)samples, n);
    free(raw_counts);
    free(lemire_counts);
    return 0;
}
