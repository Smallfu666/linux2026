#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct high_prod_case {
    uint64_t x;
    uint64_t y;
    const char *label;
};

struct high_prod32_case {
    uint32_t x;
    uint32_t y;
    const char *label;
};

struct shift_case {
    uint64_t a;
    uint32_t b;
    unsigned shift;
    const char *label;
};

static uint64_t rng_state = 0x6a09e667f3bcc909ULL;

static uint64_t next_u64(void)
{
    uint64_t x = rng_state;

    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    rng_state = x;

    return x;
}

static int64_t signed_high_prod(int64_t x, int64_t y)
{
    __int128 product = (__int128)x * (__int128)y;

    return (int64_t)(product >> 64);
}

static int32_t signed_high_prod32(int32_t x, int32_t y)
{
    int64_t product = (int64_t)x * (int64_t)y;

    return (int32_t)(product >> 32);
}

static uint32_t unsigned_high_prod32_formula(uint32_t x, uint32_t y)
{
    int32_t sx = (int32_t)x >> 31;
    int32_t sy = (int32_t)y >> 31;
    int32_t signed_hi = signed_high_prod32((int32_t)x, (int32_t)y);

    return (uint32_t)signed_hi + ((uint32_t)sx & y) + ((uint32_t)sy & x);
}

static uint32_t unsigned_high_prod32_oracle(uint32_t x, uint32_t y)
{
    uint64_t product = (uint64_t)x * (uint64_t)y;

    return (uint32_t)(product >> 32);
}

static uint64_t unsigned_high_prod_formula(uint64_t x, uint64_t y)
{
    int64_t sx = (int64_t)x;
    int64_t sy = (int64_t)y;
    uint64_t sign_mask_x = 0u - (uint64_t)((x >> 63) & 1u);
    uint64_t sign_mask_y = 0u - (uint64_t)((y >> 63) & 1u);
    uint64_t correction = (sign_mask_x & y) + (sign_mask_y & x);

    return (uint64_t)signed_high_prod(sx, sy) + correction;
}

static uint64_t unsigned_high_prod_oracle(uint64_t x, uint64_t y)
{
    __uint128_t product = (__uint128_t)x * (__uint128_t)y;

    return (uint64_t)(product >> 64);
}

static bool verify_unsigned_high_prod32(void)
{
    static const struct high_prod32_case fixed_cases[] = {
        {0x00000000u, 0x00000000u, "zero * zero"},
        {0x00000001u, 0x00000001u, "one * one"},
        {0xffffffffu, 0xffffffffu, "quiz C02"},
        {0x80000000u, 0x00000002u, "sign bit carry"},
        {0x89abcdefu, 0x76543210u, "mixed bits"},
    };
    size_t i;

    for (i = 0; i < sizeof(fixed_cases) / sizeof(fixed_cases[0]); ++i) {
        uint32_t got = unsigned_high_prod32_formula(fixed_cases[i].x, fixed_cases[i].y);
        uint32_t want = unsigned_high_prod32_oracle(fixed_cases[i].x, fixed_cases[i].y);

        if (got != want) {
            printf("[FAIL] unsigned_high_prod32 fixed case %s\n", fixed_cases[i].label);
            printf("       x=%08" PRIx32 " y=%08" PRIx32 "\n", fixed_cases[i].x, fixed_cases[i].y);
            printf("       got = %08" PRIx32 "\n", got);
            printf("       want= %08" PRIx32 "\n", want);
            return false;
        }
    }

    for (i = 0; i < 100000; ++i) {
        uint32_t x = (uint32_t)next_u64();
        uint32_t y = (uint32_t)next_u64();
        uint32_t got = unsigned_high_prod32_formula(x, y);
        uint32_t want = unsigned_high_prod32_oracle(x, y);

        if (got != want) {
            printf("[FAIL] unsigned_high_prod32 pseudo-random case %zu\n", i);
            printf("       x=%08" PRIx32 " y=%08" PRIx32 "\n", x, y);
            printf("       got = %08" PRIx32 "\n", got);
            printf("       want= %08" PRIx32 "\n", want);
            return false;
        }
    }

    printf("[OK] unsigned_high_prod32 formula matches uint64 oracle on %zu fixed + 100000 random cases\n",
           sizeof(fixed_cases) / sizeof(fixed_cases[0]));
    printf("[OK] C02 anchor: unsigned_high_prod32(FFFFFFFF, FFFFFFFF) = %08" PRIx32 "\n",
           unsigned_high_prod32_formula(0xffffffffu, 0xffffffffu));
    return true;
}

static bool verify_unsigned_high_prod(void)
{
    static const struct high_prod_case fixed_cases[] = {
        {0x0000000000000000ULL, 0x0000000000000000ULL, "zero * zero"},
        {0x0000000000000001ULL, 0x0000000000000001ULL, "one * one"},
        {0xffffffffffffffffULL, 0xfffffffffffffffeULL, "max * max-1"},
        {0x8000000000000000ULL, 0x0000000000000002ULL, "sign bit carry"},
        {0x0123456789abcdefULL, 0xfedcba9876543210ULL, "mixed bits"},
        {0xffffffff00000000ULL, 0x00000000fffffffeULL, "lower-32 stress"},
    };

    size_t i;

    for (i = 0; i < sizeof(fixed_cases) / sizeof(fixed_cases[0]); ++i) {
        uint64_t got = unsigned_high_prod_formula(fixed_cases[i].x, fixed_cases[i].y);
        uint64_t want = unsigned_high_prod_oracle(fixed_cases[i].x, fixed_cases[i].y);

        if (got != want) {
            printf("[FAIL] unsigned_high_prod fixed case %s\n", fixed_cases[i].label);
            printf("       x=%016" PRIx64 " y=%016" PRIx64 "\n", fixed_cases[i].x, fixed_cases[i].y);
            printf("       got = %016" PRIx64 "\n", got);
            printf("       want= %016" PRIx64 "\n", want);
            return false;
        }
    }

    for (i = 0; i < 100000; ++i) {
        uint64_t x = next_u64();
        uint64_t y = next_u64();
        uint64_t got = unsigned_high_prod_formula(x, y);
        uint64_t want = unsigned_high_prod_oracle(x, y);

        if (got != want) {
            printf("[FAIL] unsigned_high_prod pseudo-random case %zu\n", i);
            printf("       x=%016" PRIx64 " y=%016" PRIx64 "\n", x, y);
            printf("       got = %016" PRIx64 "\n", got);
            printf("       want= %016" PRIx64 "\n", want);
            return false;
        }
    }

    printf("[OK] unsigned_high_prod formula matches __int128 oracle on %zu fixed + 100000 random cases\n",
           sizeof(fixed_cases) / sizeof(fixed_cases[0]));
    return true;
}

static uint64_t mul_u64_u32_shr_oracle(uint64_t a, uint32_t b, unsigned shift)
{
    __uint128_t product = (__uint128_t)a * (__uint128_t)b;

    return (uint64_t)(product >> shift);
}

static uint64_t mul_u64_u32_shr_fallback(uint64_t a, uint32_t b, unsigned shift,
                                         uint64_t *p0_out, uint64_t *p1_out,
                                         uint64_t *carry_out, uint64_t *mid_out)
{
    uint64_t a_lo = (uint32_t)a;
    uint64_t a_hi = a >> 32;
    uint64_t p0 = a_lo * (uint64_t)b;
    uint64_t p1 = a_hi * (uint64_t)b;
    uint64_t carry = p0 >> 32;
    uint64_t lo32 = p0 & UINT32_MAX;
    uint64_t mid = p1 + carry;
    uint64_t result;

    if (shift < 32) {
        result = (mid << (32 - shift)) | (lo32 >> shift);
    } else if (shift == 32) {
        result = mid;
    } else {
        result = mid >> (shift - 32);
    }

    if (p0_out != NULL) {
        *p0_out = p0;
    }
    if (p1_out != NULL) {
        *p1_out = p1;
    }
    if (carry_out != NULL) {
        *carry_out = carry;
    }
    if (mid_out != NULL) {
        *mid_out = mid;
    }
    return result;
}

static bool verify_mul_u64_u32_shr(void)
{
    static const struct shift_case cases[] = {
        {0x0123456789abcdefULL, 0x89abcdefu, 16, "mid shift with carry"},
        {0xffffffff00000000ULL, 0xfffffffeu, 32, "exact 32-bit cut"},
        {0xfeedfacecafebeefULL, 0x7fffffffu, 47, "large shift"},
        {0x00000001ffffffffULL, 0xffffffffu, 31, "carry crosses into high word"},
        {0xffffffffffffffffULL, 0x00000002u, 1, "max times two"},
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        uint64_t p0 = 0;
        uint64_t p1 = 0;
        uint64_t carry = 0;
        uint64_t mid = 0;
        uint64_t got = mul_u64_u32_shr_fallback(cases[i].a, cases[i].b, cases[i].shift,
                                                &p0, &p1, &carry, &mid);
        uint64_t want = mul_u64_u32_shr_oracle(cases[i].a, cases[i].b, cases[i].shift);

        printf("[demo] %s\n", cases[i].label);
        printf("       a=%016" PRIx64 " b=%08" PRIx32 " shift=%u\n",
               cases[i].a, cases[i].b, cases[i].shift);
        printf("       p0=lo*mul=%016" PRIx64 " p1=hi*mul=%016" PRIx64 "\n", p0, p1);
        printf("       carry=p0>>32=%016" PRIx64 " mid=p1+carry=%016" PRIx64 "\n", carry, mid);
        printf("       fallback=%016" PRIx64 " oracle=%016" PRIx64 "\n", got, want);

        if (got != want) {
            printf("[FAIL] mul_u64_u32_shr fallback mismatch\n");
            return false;
        }
    }

    for (i = 0; i < 50000; ++i) {
        uint64_t a = next_u64();
        uint32_t b = (uint32_t)next_u64();
        unsigned shift = (unsigned)(next_u64() % 64u);
        uint64_t got = mul_u64_u32_shr_fallback(a, b, shift, NULL, NULL, NULL, NULL);
        uint64_t want = mul_u64_u32_shr_oracle(a, b, shift);

        if (got != want) {
            printf("[FAIL] mul_u64_u32_shr random case %zu\n", i);
            printf("       a=%016" PRIx64 " b=%08" PRIx32 " shift=%u\n", a, b, shift);
            printf("       got = %016" PRIx64 "\n", got);
            printf("       want= %016" PRIx64 "\n", want);
            return false;
        }
    }

    printf("[OK] mul_u64_u32_shr fallback matches __int128 oracle on %zu fixed + 50000 random cases\n",
           sizeof(cases) / sizeof(cases[0]));
    return true;
}

static size_t size_mul_like(size_t a, size_t b)
{
    if (a == 0 || b == 0) {
        return 0;
    }
    if (a > SIZE_MAX / b) {
        return SIZE_MAX;
    }
    return a * b;
}

static bool verify_size_mul_like(void)
{
    struct {
        size_t a;
        size_t b;
        size_t want;
        const char *label;
    } cases[] = {
        {3, 5, 15, "small product"},
        {1024, 4096, 4194304, "power-of-two product"},
        {SIZE_MAX, 2, SIZE_MAX, "overflow sentinel"},
        {((size_t)1 << (sizeof(size_t) * CHAR_BIT - 2)), 8, SIZE_MAX, "large overflow"},
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        size_t got = size_mul_like(cases[i].a, cases[i].b);

        printf("[demo] %s: a=%zu b=%zu => %zu\n", cases[i].label, cases[i].a, cases[i].b, got);
        if (got != cases[i].want) {
            printf("[FAIL] size_mul_like mismatch\n");
            return false;
        }
    }

    printf("[OK] size_mul_like returns SIZE_MAX sentinel on overflow\n");
    return true;
}

int main(void)
{
    bool ok = true;

    ok = verify_unsigned_high_prod32() && ok;
    ok = verify_unsigned_high_prod() && ok;
    ok = verify_mul_u64_u32_shr() && ok;
    ok = verify_size_mul_like() && ok;

    if (!ok) {
        return EXIT_FAILURE;
    }

    printf("[OK] all c_mul_high checks passed\n");
    return EXIT_SUCCESS;
}
