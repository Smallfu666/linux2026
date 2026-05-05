#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { BITSET_WORDS = 128, BITSET_BITS = BITSET_WORDS * 32, HASH_PROBES = 6 };

uint32_t simple_sum_hash(const char *s);
uint32_t murmurhash3_32(const char *key, uint32_t seed);

static void bitset_clear(uint32_t bits[BITSET_WORDS]) {
    memset(bits, 0, sizeof(uint32_t) * BITSET_WORDS);
}

static void bitset_set(uint32_t bits[BITSET_WORDS], uint32_t index) {
    bits[index / 32u] |= (uint32_t)1u << (index % 32u);
}

static size_t bitset_count(const uint32_t bits[BITSET_WORDS]) {
    size_t total = 0;
    for (size_t i = 0; i < BITSET_WORDS; ++i) {
        total += (size_t)__builtin_popcount(bits[i]);
    }
    return total;
}

static uint32_t derive_index(uint32_t h1, uint32_t h2, size_t probe) {
    uint32_t mix = h1 + (uint32_t)probe * (h2 | 1u) + (uint32_t)(probe * probe + 1u) * 0x9e3779b9u;
    return mix % BITSET_BITS;
}

static uint32_t strong_hash(const char *s) {
    return murmurhash3_32(s, 0x9747b28cu);
}

static void insert_keys(uint32_t bits[BITSET_WORDS], uint32_t (*hash_fn)(const char *), const char *label) {
    char key[64];
    for (int i = 0; i < 500; ++i) {
        snprintf(key, sizeof(key), "%s-key-%03d", label, i);
        uint32_t h1 = hash_fn(key);
        uint32_t h2 = strong_hash(key);
        for (size_t probe = 0; probe < HASH_PROBES; ++probe) {
            bitset_set(bits, derive_index(h1, h2, probe));
        }
    }
}

static void report(const char *name, size_t count) {
    const double expected = 2048.0;
    printf("%s set bits: %zu (expected ~%.0f, delta %+0.1f)\n", name, count, expected, (double)count - expected);
}

int main(void) {
    uint32_t simple_bits[BITSET_WORDS];
    uint32_t strong_bits[BITSET_WORDS];

    bitset_clear(simple_bits);
    bitset_clear(strong_bits);

    insert_keys(simple_bits, simple_sum_hash, "simple");
    insert_keys(strong_bits, strong_hash, "murmur");

    report("Simple Sum Hash", bitset_count(simple_bits));
    report("MurmurHash3", bitset_count(strong_bits));
    return 0;
}
