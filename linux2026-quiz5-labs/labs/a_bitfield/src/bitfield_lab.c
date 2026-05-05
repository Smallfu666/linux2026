#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

static uint32_t low_bytes_mask(unsigned nbytes) {
    if (nbytes == 0) {
        return UINT32_C(0);
    }
    if (nbytes >= 4) {
        return UINT32_C(0xffffffff);
    }
    return (UINT32_C(1) << (nbytes * 8)) - UINT32_C(1);
}

static uint32_t genmask_u32(unsigned hi, unsigned lo) {
    if (hi < lo) {
        return UINT32_C(0);
    }
    if (hi >= 31) {
        return UINT32_C(0xffffffff) & ~((UINT32_C(1) << lo) - UINT32_C(1));
    }
    return ((UINT32_C(1) << (hi + 1)) - UINT32_C(1)) & ~((UINT32_C(1) << lo) - UINT32_C(1));
}

static uint32_t replace_low_nbytes(uint32_t dst, uint32_t src, unsigned nbytes) {
    const uint32_t mask = low_bytes_mask(nbytes);
    return (dst & ~mask) | (src & mask);
}

static uint32_t replace_nbytes(uint32_t x, unsigned i, unsigned n, uint32_t y) {
    const unsigned shift = i << 3;
    const uint32_t mask = low_bytes_mask(n);
    const uint32_t field_mask = mask << shift;

    return (x & ~field_mask) | ((y & mask) << shift);
}

static unsigned popcount_u32(uint32_t x) {
    return (unsigned)__builtin_popcount(x);
}

static void check_low_case(unsigned nbytes, uint32_t dst, uint32_t src, uint32_t expected) {
    const uint32_t mask = low_bytes_mask(nbytes);
    const uint32_t field_mask = genmask_u32(nbytes * 8 - 1, 0);
    const uint32_t got = replace_low_nbytes(dst, src, nbytes);

    printf("n=%u mask=0x%08" PRIx32 " popcount=%u result=0x%08" PRIx32 "\n",
           nbytes, mask, popcount_u32(mask), got);
    printf("  GENMASK-like=0x%08" PRIx32 " safe_full_mask=0x%08" PRIx32 "\n",
           field_mask, low_bytes_mask(4));

    assert(mask == field_mask);
    assert(popcount_u32(mask) == nbytes * 8);
    assert(got == expected);
}

static void check_replace_case(uint32_t x, unsigned i, unsigned n, uint32_t y, uint32_t expected) {
    const unsigned shift = i << 3;
    const uint32_t mask = low_bytes_mask(n);
    const uint32_t got = replace_nbytes(x, i, n, y);

    printf("x=0x%08" PRIx32 " i=%u n=%u y=0x%08" PRIx32
           " shift=%u A01=0x%08" PRIx32 " A02=0x%08" PRIx32
           " result=0x%08" PRIx32 "\n",
           x, i, n, y, shift, mask << shift, mask, got);

    assert(got == expected);
}

int main(void) {
    const uint32_t dst = UINT32_C(0x55667788);
    const uint32_t src = UINT32_C(0xAA112233);

    check_low_case(1, dst, src, UINT32_C(0x55667733));
    check_low_case(4, dst, src, UINT32_C(0xAA112233));

    check_replace_case(UINT32_C(0xAABBCCDD), 0, 3, UINT32_C(0x112233), UINT32_C(0xAA112233));
    check_replace_case(UINT32_C(0x12345678), 1, 2, UINT32_C(0xABCD), UINT32_C(0x12ABCD78));
    check_replace_case(UINT32_C(0x12345678), 0, 4, UINT32_C(0xAABBCCDD), UINT32_C(0xAABBCCDD));
    check_replace_case(UINT32_C(0x12345678), 2, 1, UINT32_C(0xAB), UINT32_C(0x12AB5678));

    printf("quiz5 A answer map:\n");
    printf("  A01 = mask << shift\n");
    printf("  A02 = mask\n");
    printf("  A03 = undefined behavior\n");
    printf("  A04 = 0x%08" PRIx32 "\n", UINT32_C(0xAA112233));

    printf("safe mask demo: avoid 1u << 32 UB\n");
    printf("  safe_full_mask(4 bytes) = 0x%08" PRIx32 "\n", low_bytes_mask(4));
    printf("  32-bit mask width       = %u bits\n", (unsigned)(sizeof(uint32_t) * CHAR_BIT));

    return 0;
}
