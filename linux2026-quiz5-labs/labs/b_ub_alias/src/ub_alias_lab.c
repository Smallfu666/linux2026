#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BIT_BAD(n) (1 << (n))
#define BIT_UL(n)  (1UL << (n))

static uint32_t genmask_u32(unsigned hi, unsigned lo) {
    if (hi < lo) {
        return UINT32_C(0);
    }
    if (hi >= 31) {
        return UINT32_C(0xffffffff) & ~((UINT32_C(1) << lo) - UINT32_C(1));
    }
    return ((UINT32_C(1) << (hi + 1)) - UINT32_C(1)) & ~((UINT32_C(1) << lo) - UINT32_C(1));
}

__attribute__((noinline)) static int signed_shift_bad(int n) {
    return BIT_BAD(n);
}

__attribute__((noinline)) static unsigned long bit_ul(unsigned n) {
    return BIT_UL(n);
}

__attribute__((noinline)) static uint32_t alias_violation(uint32_t *ip, float *fp) {
    *ip = UINT32_C(0x3f800000);
    *fp = 0.0f;
    return *ip;
}

__attribute__((noinline)) static uint32_t bits_from_float_memcpy(float f) {
    uint32_t out = UINT32_C(0);
    memcpy(&out, &f, sizeof out);
    return out;
}

__attribute__((noinline)) static uint32_t bits_from_float_union(float f) {
    union {
        float f;
        uint32_t u;
    } pun = { .f = f };
    return pun.u;
}

static void print_mask_demo(void) {
    const unsigned ul_bits = (unsigned)(sizeof(unsigned long) * CHAR_BIT);
    const unsigned int_bits = (unsigned)(sizeof(int) * CHAR_BIT);
    const uint32_t mask = genmask_u32(15, 8);

    printf("word sizes: int=%u bits, unsigned long=%u bits\n", int_bits, ul_bits);
    printf("GENMASK-like(15,8) = 0x%08" PRIx32 "\n", mask);
    printf("BIT_UL(%u) = 0x%0*lx\n", ul_bits - 1, (int)(sizeof(unsigned long) * 2), bit_ul(ul_bits - 1));
}

static void print_shift_demo(void) {
    const int bad_n = (int)(sizeof(int) * CHAR_BIT - 1);
    const unsigned long good_n = (unsigned long)(sizeof(unsigned long) * CHAR_BIT - 1);

    printf("signed_shift_bad(%d) -> %d\n", bad_n, signed_shift_bad(bad_n));
    printf("BIT_UL(%lu)          -> 0x%0*lx\n",
           good_n,
           (int)(sizeof(unsigned long) * 2),
           bit_ul((unsigned)good_n));
}

static void print_alias_demo(void) {
    union {
        uint32_t u32;
        float f;
    } cell = { .u32 = UINT32_C(0) };

    const uint32_t bad = alias_violation(&cell.u32, &cell.f);
    const uint32_t safe_memcpy = bits_from_float_memcpy(1.0f);
    const uint32_t safe_union = bits_from_float_union(1.0f);

    printf("alias_violation (pointer cast / strict aliasing UB) = 0x%08" PRIx32 "\n", bad);
    printf("memcpy bit-cast                                   = 0x%08" PRIx32 "\n", safe_memcpy);
    printf("union bit-cast                                    = 0x%08" PRIx32 "\n", safe_union);

    assert(safe_memcpy == UINT32_C(0x3f800000));
    assert(safe_union == UINT32_C(0x3f800000));
}

int main(int argc, char **argv) {
    print_mask_demo();
    print_alias_demo();

    if (argc > 1 && strcmp(argv[1], "ub") == 0) {
        print_shift_demo();
    }

    return 0;
}
