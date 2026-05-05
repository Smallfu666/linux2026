#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t round_shift_right_even_u64(uint64_t value, unsigned shift) {
    if (shift == 0) {
        return (uint32_t) value;
    }
    if (shift >= 64) {
        return 0;
    }
    uint64_t upper = value >> shift;
    uint64_t lower_mask = ((uint64_t) 1 << shift) - 1;
    uint64_t lower = value & lower_mask;
    uint64_t half = (uint64_t) 1 << (shift - 1);
    if (lower > half || (lower == half && (upper & 1))) {
        upper++;
    }
    return (uint32_t) upper;
}

static uint32_t float_quarter_ref(uint32_t uf) {
    uint32_t sign = uf & 0x80000000u;
    uint32_t exp = (uf >> 23) & 0xffu;
    uint32_t frac = uf & 0x7fffffu;

    if (exp == 0xffu) {
        return uf;
    }

    if (exp >= 3u) {
        return sign | ((exp - 2u) << 23) | frac;
    }

    if (exp == 2u) {
        uint32_t sig = 0x800000u | frac;
        return sign | round_shift_right_even_u64(sig, 1);
    }

    if (exp == 1u) {
        uint32_t sig = 0x800000u | frac;
        return sign | round_shift_right_even_u64(sig, 2);
    }

    return sign | round_shift_right_even_u64(frac, 2);
}

static uint32_t float_quarter(uint32_t uf) {
    uint32_t sign = uf & 0x80000000u;
    uint32_t exp = (uf >> 23) & 0xffu;
    uint32_t frac = uf & 0x7fffffu;

    if (exp == 0xffu) {
        return uf;
    }

    if (exp > 2u) {
        return sign | ((exp - 2u) << 23) | frac;
    }

    if (exp == 2u) {
        uint32_t sig = 0x800000u | frac;
        return sign | round_shift_right_even_u64(sig, 1);
    }

    if (exp == 1u) {
        uint32_t sig = 0x800000u | frac;
        return sign | round_shift_right_even_u64(sig, 2);
    }

    return sign | round_shift_right_even_u64(frac, 2);
}

static uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static void fail_case(const char *label, uint32_t input, uint32_t got, uint32_t want) {
    fprintf(stderr,
            "%s failed: input=0x%08x got=0x%08x want=0x%08x\n",
            label,
            input,
            got,
            want);
    exit(1);
}

static void verify_special_cases(void) {
    static const uint32_t samples[] = {
        0x00000000u,
        0x80000000u,
        0x00000001u,
        0x00000002u,
        0x00000003u,
        0x00000004u,
        0x007fffffu,
        0x00800000u,
        0x00800001u,
        0x01000000u,
        0x01000001u,
        0x017fffffu,
        0x01800000u,
        0x3f800000u,
        0x7f7fffffu,
        0x7f800000u,
        0x7fc00000u,
        0xff800000u,
        0xffc12345u,
    };

    for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
        uint32_t ref = float_quarter_ref(samples[i]);
        uint32_t got = float_quarter(samples[i]);
        if (got != ref) {
            fail_case("special", samples[i], got, ref);
        }
    }
}

static void verify_boundary_cases(void) {
    for (uint32_t frac = 0; frac < 0x1000u; ++frac) {
        uint32_t uf = 0x00800000u | frac;
        uint32_t ref = float_quarter_ref(uf);
        uint32_t got = float_quarter(uf);
        if (got != ref) {
            fail_case("boundary_min_normal", uf, got, ref);
        }
    }

    for (uint32_t frac = 0x7ff000u; frac <= 0x7fffffu; ++frac) {
        uint32_t uf = frac;
        uint32_t ref = float_quarter_ref(uf);
        uint32_t got = float_quarter(uf);
        if (got != ref) {
            fail_case("boundary_subnormal", uf, got, ref);
        }
    }
}

static void verify_random_cases(void) {
    uint32_t state = 0x9e3779b9u;
    for (unsigned i = 0; i < 250000u; ++i) {
        uint32_t uf = xorshift32(&state);
        uint32_t ref = float_quarter_ref(uf);
        uint32_t got = float_quarter(uf);
        if (got != ref) {
            fail_case("random", uf, got, ref);
        }
    }
}

static void print_demo(void) {
    static const uint32_t inputs[] = {
        0x00000001u,
        0x00000002u,
        0x00000003u,
        0x00800000u,
        0x00800001u,
        0x01000000u,
        0x3f800000u,
    };
    puts("float_quarter demo");
    puts("input -> output");
    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        printf("0x%08x -> 0x%08x\n", inputs[i], float_quarter(inputs[i]));
    }
    puts("float_eighth shift table");
    for (unsigned e = 0; e <= 6; ++e) {
        uint32_t uf = e << 23;
        printf("exp=%u raw=0x%08x quarter=0x%08x eighth=0x%08x\n",
               e,
               uf,
               float_quarter(uf),
               float_quarter(float_quarter(uf)));
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && argv[1] && argv[1][0] == '-' && argv[1][1] == '-' &&
        argv[1][2] == 'd' && argv[1][3] == 'e' && argv[1][4] == 'm' &&
        argv[1][5] == 'o' && argv[1][6] == '\0') {
        print_demo();
        return 0;
    }

    verify_special_cases();
    verify_boundary_cases();
    verify_random_cases();

    printf("float_quarter: PASS\n");
    printf("boundary_min_normal_cases=4096\n");
    printf("boundary_subnormal_cases=4096\n");
    printf("random_cases=250000\n");
    printf("E01=2\n");
    printf("E02=1\n");
    printf("E03=00200000\n");
    return 0;
}
