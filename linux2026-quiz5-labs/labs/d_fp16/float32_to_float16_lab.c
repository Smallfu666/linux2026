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

static uint32_t half_to_float32_bits(uint16_t h) {
    uint32_t sign = (uint32_t) (h & 0x8000u) << 16;
    uint32_t exp = (h >> 10) & 0x1fu;
    uint32_t frac = h & 0x03ffu;

    if (exp == 0x1fu) {
        return sign | 0x7f800000u | (frac << 13);
    }
    if (exp == 0) {
        if (frac == 0) {
            return sign;
        }
        int e = -14;
        while ((frac & 0x0400u) == 0) {
            frac <<= 1;
            e--;
        }
        frac &= 0x03ffu;
        return sign | ((uint32_t) (e + 127) << 23) | (frac << 13);
    }
    return sign | ((exp + 112u) << 23) | (frac << 13);
}

static uint16_t pack_f16(uint32_t sign, int exp, uint32_t frac) {
    return (uint16_t) ((sign << 15) | ((uint32_t) exp << 10) | (frac & 0x03ffu));
}

static uint16_t float32_to_float16_ref(uint32_t uf) {
    uint32_t sign = uf >> 31;
    uint32_t exp = (uf >> 23) & 0xffu;
    uint32_t frac = uf & 0x7fffffu;

    if (exp == 0xffu) {
        if (frac == 0) {
            return pack_f16(sign, 0x1f, 0);
        }
        uint16_t payload = (uint16_t) (frac >> 13);
        if (payload == 0) {
            payload = 0x0200u;
        }
        return (uint16_t) ((sign << 15) | 0x7c00u | payload);
    }

    if (exp == 0 && frac == 0) {
        return (uint16_t) (sign << 15);
    }

    uint64_t sig;
    int e;
    if (exp == 0) {
        sig = frac;
        e = -126;
        while ((sig & 0x800000u) == 0) {
            sig <<= 1;
            e--;
        }
    } else {
        sig = 0x800000u | frac;
        e = (int) exp - 127;
    }

    int half_exp = e + 15;
    if (half_exp >= 31) {
        return pack_f16(sign, 0x1f, 0);
    }

    if (half_exp > 0) {
        uint32_t rounded = round_shift_right_even_u64(sig, 13);
        if (rounded == 0x800u) {
            half_exp++;
            rounded = 0x400u;
            if (half_exp >= 31) {
                return pack_f16(sign, 0x1f, 0);
            }
        }
        return pack_f16(sign, half_exp, rounded & 0x03ffu);
    }

    int shift = 14 - half_exp;
    uint32_t rounded = round_shift_right_even_u64(sig, (unsigned) shift);
    if (rounded >= 0x400u) {
        return pack_f16(sign, 1, 0);
    }
    return (uint16_t) ((sign << 15) | rounded);
}

static uint16_t float32_to_float16_full(uint32_t uf) {
    uint32_t sign = uf >> 31;
    uint32_t exp = (uf >> 23) & 0xffu;
    uint32_t frac = uf & 0x7fffffu;

    if (exp == 0xffu) {
        if (frac == 0) {
            return (uint16_t) ((sign << 15) | 0x7c00u);
        }
        uint16_t payload = (uint16_t) (frac >> 13);
        if (payload == 0) {
            payload = 0x0200u;
        }
        return (uint16_t) ((sign << 15) | 0x7c00u | payload);
    }

    if (exp == 0 && frac == 0) {
        return (uint16_t) (sign << 15);
    }

    uint32_t sig;
    int e;
    if (exp == 0) {
        sig = frac;
        e = -126;
        while ((sig & 0x800000u) == 0) {
            sig <<= 1;
            e--;
        }
    } else {
        sig = 0x800000u | frac;
        e = (int) exp - 127;
    }

    int half_exp = e + 15;
    if (half_exp >= 31) {
        return (uint16_t) ((sign << 15) | 0x7c00u);
    }

    if (half_exp > 0) {
        uint32_t rounded = round_shift_right_even_u64(sig, 13);
        if (rounded == 0x800u) {
            half_exp++;
            rounded = 0x400u;
            if (half_exp >= 31) {
                return (uint16_t) ((sign << 15) | 0x7c00u);
            }
        }
        return (uint16_t) ((sign << 15) | ((uint32_t) half_exp << 10) | (rounded & 0x03ffu));
    }

    {
        int shift = 14 - half_exp;
        uint32_t rounded = round_shift_right_even_u64(sig, (unsigned) shift);
        if (rounded >= 0x400u) {
            return (uint16_t) ((sign << 15) | 0x0400u);
        }
        return (uint16_t) ((sign << 15) | rounded);
    }
}

static uint16_t float32_to_float16_quiz(uint32_t f) {
    unsigned sign = (f >> 31) & 1;
    unsigned exp = (f >> 23) & 0xff;
    unsigned frac = f & 0x7fffff;
    unsigned hsign = sign << 15;

    if (exp == 0xff) {
        if (frac == 0)
            return (uint16_t)(hsign | 0x7c00);
        return (uint16_t)(hsign | 0x7e00);
    }

    if (exp == 0)
        return (uint16_t)hsign;

    int hexp = (int)exp - 112;

    if (hexp >= 31)
        return (uint16_t)(hsign | 0x7c00);

    if (hexp <= 0) {
        unsigned mant = frac | 0x800000;
        int shift = 14 - hexp;
        if (shift > 24)
            return (uint16_t)hsign;

        unsigned val = mant >> shift;
        unsigned lost = mant & ((1u << shift) - 1);
        unsigned half = 1u << (shift - 1);

        if (lost > half || (lost == half && (val & 1)))
            val++;

        return (uint16_t)(hsign | val);
    }

    unsigned hfrac = frac >> 13;
    unsigned lost = frac & 0x1fff;
    unsigned half = 1u << 12;

    if (lost > half || (lost == half && (hfrac & 1))) {
        hfrac++;
        if (hfrac == 0x400) {
            hfrac = 0;
            hexp++;
            if (hexp >= 31)
                return (uint16_t)(hsign | 0x7c00);
        }
    }

    return (uint16_t)(hsign | ((unsigned)hexp << 10) | hfrac);
}

static uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static void fail_case(const char *label, uint32_t input, uint16_t got, uint16_t want) {
    fprintf(stderr,
            "%s failed: input=0x%08x got=0x%04x want=0x%04x\n",
            label,
            input,
            got,
            want);
    exit(1);
}

static void verify_roundtrip_cases(void) {
    for (uint32_t h = 0; h < 0x10000u; ++h) {
        uint16_t half = (uint16_t) h;
        uint32_t f32 = half_to_float32_bits(half);
        uint16_t ref = float32_to_float16_ref(f32);
        uint16_t got = float32_to_float16_full(f32);
        if (got != ref) {
            fail_case("half_roundtrip", f32, got, ref);
        }
    }
}

static void verify_specials(void) {
    static const uint32_t samples[] = {
        0x00000000u,
        0x80000000u,
        0x00000001u,
        0x00000fffu,
        0x007fffffu,
        0x00800000u,
        0x00800001u,
        0x387fffffu,
        0x38800000u,
        0x38800001u,
        0x477fe000u,
        0x477ff000u,
        0x7f7fffffu,
        0x7f800000u,
        0x7fc00000u,
        0x7fa12345u,
        0xff800000u,
        0xffc00001u,
    };

    for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
        uint16_t ref = float32_to_float16_ref(samples[i]);
        uint16_t got = float32_to_float16_full(samples[i]);
        if (got != ref) {
            fail_case("special", samples[i], got, ref);
        }
    }
}

static void verify_random_cases(void) {
    uint32_t state = 0x6d2b79f5u;
    for (unsigned i = 0; i < 200000u; ++i) {
        uint32_t uf = xorshift32(&state);
        uint16_t ref = float32_to_float16_ref(uf);
        uint16_t got = float32_to_float16_full(uf);
        if (got != ref) {
            fail_case("random", uf, got, ref);
        }
    }
}

int main(void) {
    verify_roundtrip_cases();
    verify_specials();
    verify_random_cases();

    if (float32_to_float16_quiz(0x387fe000u) != 0x0400u)
        fail_case("quiz_denorm_boundary", 0x387fe000u, float32_to_float16_quiz(0x387fe000u), 0x0400u);
    if (float32_to_float16_quiz(0x7f800000u) != 0x7c00u)
        fail_case("quiz_inf", 0x7f800000u, float32_to_float16_quiz(0x7f800000u), 0x7c00u);
    if (float32_to_float16_quiz(0x7fc00000u) != 0x7e00u)
        fail_case("quiz_nan", 0x7fc00000u, float32_to_float16_quiz(0x7fc00000u), 0x7e00u);
    if (float32_to_float16_quiz(0x00000001u) != 0x0000u)
        fail_case("quiz_single_denorm_maps_zero", 0x00000001u, float32_to_float16_quiz(0x00000001u), 0x0000u);

    printf("float32_to_float16_full: PASS\n");
    printf("float32_to_float16_quiz: PASS\n");
    printf("roundtrip_half_cases=65536\n");
    printf("random_cases=200000\n");
    printf("D01=112\n");
    printf("D02=14\n");
    printf("D03=12\n");
    return 0;
}
