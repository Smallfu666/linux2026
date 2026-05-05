#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    ROUNDTRIP_TRIALS = 1000,
    ROUNDTRIP_N = 128,
};

static int bytes_for(int n, int bits)
{
    return (n * bits + 7) >> 3;
}

static uint8_t low_mask(int bits)
{
    return (uint8_t) ((UINT32_C(1) << bits) - UINT32_C(1));
}

static void print_binary8(uint8_t x)
{
    for (int i = 7; i >= 0; --i) {
        putchar((x & (uint8_t) (UINT8_C(1) << i)) ? '1' : '0');
    }
}

static void pack_indices(const uint8_t *idx, uint8_t *packed, int n, int bits)
{
    int packed_len = bytes_for(n, bits);
    uint32_t mask = (UINT32_C(1) << bits) - UINT32_C(1);

    memset(packed, 0, (size_t) packed_len);

    for (int i = 0; i < n; ++i) {
        uint32_t value = (uint32_t) idx[i] & mask;
        int bp = i * bits;

        for (int j = 0; j < bits; ++j) {
            int byte = bp >> 3;
            int b = bp & 7;
            uint8_t bit = (uint8_t) (value & UINT32_C(1));

            packed[byte] = (uint8_t) (packed[byte] | (uint8_t) (bit << b));
            value >>= 1;
            ++bp;
        }
    }
}

static void unpack_indices(const uint8_t *packed, uint8_t *idx, int n, int bits)
{
    uint32_t mask = (UINT32_C(1) << bits) - UINT32_C(1);

    for (int i = 0; i < n; ++i) {
        uint32_t value = 0;
        int bp = i * bits;

        for (int j = 0; j < bits; ++j) {
            int byte = bp >> 3;
            int b = bp & 7;
            uint32_t bit = ((uint32_t) packed[byte] >> b) & UINT32_C(1);

            value |= bit << j;
            ++bp;
        }

        idx[i] = (uint8_t) (value & mask);
    }
}

static uint32_t xorshift32(uint32_t *state)
{
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static void test_case_1(void)
{
    const uint8_t idx[3] = {5, 3, 7};
    uint8_t packed[2];
    uint8_t unpacked[3] = {0, 0, 0};

    pack_indices(idx, packed, 3, 3);
    unpack_indices(packed, unpacked, 3, 3);

    puts("case 1: idx={5,3,7}, bits=3, LSB-first");
    printf("  packed[0] binary: ");
    print_binary8(packed[0]);
    printf("\n  packed[0] hex:    0x%02" PRIX8 "\n", packed[0]);
    printf("  packed[1] binary: ");
    print_binary8(packed[1]);
    printf("\n  packed[1] hex:    0x%02" PRIX8 "\n", packed[1]);
    printf("  unpacked:         {%u, %u, %u}\n",
           (unsigned) unpacked[0], (unsigned) unpacked[1], (unsigned) unpacked[2]);
    printf("  expected:         packed={0xDD,0x01}, unpacked={5,3,7}\n");

    if (packed[0] != UINT8_C(0xdd) || packed[1] != UINT8_C(0x01) ||
        unpacked[0] != 5 || unpacked[1] != 3 || unpacked[2] != 7) {
        fputs("case 1 failed\n", stderr);
        exit(1);
    }
}

static void test_case_2(void)
{
    uint8_t idx[ROUNDTRIP_N];
    uint8_t unpacked[ROUNDTRIP_N];
    uint8_t packed[(ROUNDTRIP_N * 7 + 7) / 8];
    uint32_t rng = UINT32_C(0x6d2b79f5);

    puts("case 2: randomized roundtrip, n=128, trials=1000");
    for (int bits = 1; bits <= 7; ++bits) {
        uint8_t mask = low_mask(bits);
        int packed_len = bytes_for(ROUNDTRIP_N, bits);

        for (int trial = 0; trial < ROUNDTRIP_TRIALS; ++trial) {
            for (int i = 0; i < ROUNDTRIP_N; ++i) {
                idx[i] = (uint8_t) (xorshift32(&rng) & mask);
            }
            memset(unpacked, 0, sizeof(unpacked));
            pack_indices(idx, packed, ROUNDTRIP_N, bits);
            unpack_indices(packed, unpacked, ROUNDTRIP_N, bits);
            if (memcmp(idx, unpacked, sizeof(idx)) != 0) {
                fprintf(stderr, "roundtrip failed: bits=%d trial=%d\n", bits, trial);
                exit(1);
            }
        }

        printf("  bits=%d packed_len=%d: pass\n", bits, packed_len);
    }
}

static void test_case_3(void)
{
    puts("case 3: low-bits masks");
    for (int bits = 1; bits <= 7; ++bits) {
        printf("  bits=%d -> 0x%02" PRIX8 "\n", bits, low_mask(bits));
    }
}

int main(void)
{
    test_case_1();
    test_case_2();
    test_case_3();
    puts("bitpack_lab: all checks passed");
    return 0;
}
