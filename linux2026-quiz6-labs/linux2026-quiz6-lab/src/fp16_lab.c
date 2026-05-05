#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

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

    if (lower > half || (lower == half && (upper & 1u))) {
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

static uint32_t float_to_bits(float x) {
    uint32_t bits;
    memcpy(&bits, &x, sizeof(bits));
    return bits;
}

static float bits_to_float(uint32_t bits) {
    float x;
    memcpy(&x, &bits, sizeof(x));
    return x;
}

static float half_to_float(uint16_t h) {
    return bits_to_float(half_to_float32_bits(h));
}

static double abs_double(double x) {
    return x < 0.0 ? -x : x;
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
        return (uint16_t) ((sign << 15) | 0x7e00u);
    }

    if (exp == 0 && frac == 0) {
        return (uint16_t) (sign << 15);
    }

    if (exp == 0) {
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

    {
        int shift = 14 - half_exp;
        uint32_t rounded = round_shift_right_even_u64(sig, (unsigned) shift);
        if (rounded >= 0x400u) {
            return (uint16_t) ((sign << 15) | 0x0400u);
        }
        return (uint16_t) ((sign << 15) | rounded);
    }
}

static uint16_t float32_to_float16_experiment_a(uint32_t uf) {
    uint32_t sign = uf >> 31;
    uint32_t exp = (uf >> 23) & 0xffu;
    uint32_t frac = uf & 0x7fffffu;
    uint32_t hsign = sign << 15;

    if (exp == 0xffu) {
        if (frac == 0) {
            return (uint16_t) (hsign | 0x7c00u);
        }
        return (uint16_t) (hsign | 0x7e00u);
    }

    if (exp == 0) {
        return (uint16_t) hsign;
    }

    int hexp = (int) exp - 112;
    if (hexp >= 31) {
        return (uint16_t) (hsign | 0x7c00u);
    }

    if (hexp <= 0) {
        uint32_t mant = frac | 0x800000u;
        int shift = 14 - hexp;
        if (shift > 24) {
            return (uint16_t) hsign;
        }

        uint32_t val = mant >> shift;
        uint32_t lost = mant & ((UINT32_C(1) << shift) - 1u);
        uint32_t half = UINT32_C(1) << (shift - 1);

        if (lost > half || (lost == half && (val & 1u))) {
            val++;
        }
        return (uint16_t) (hsign | val);
    }

    uint32_t hfrac = frac >> 13;
    uint32_t lost = frac & 0x1fffu;
    uint32_t half = UINT32_C(1) << 12;

    if (lost > half || (lost == half && (hfrac & 1u))) {
        hfrac++;
        if (hfrac == 0x400u) {
            hfrac = 0;
            hexp++;
            if (hexp >= 31) {
                return (uint16_t) (hsign | 0x7c00u);
            }
        }
    }

    return (uint16_t) (hsign | ((uint32_t) hexp << 10) | hfrac);
}

static void fail_case(const char *label, uint32_t input, uint16_t got, uint16_t want) {
    fprintf(stderr,
            "%s failed: input=0x%08" PRIx32 " got=0x%04" PRIx16 " want=0x%04" PRIx16 "\n",
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
        uint16_t got = float32_to_float16_experiment_a(f32);
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
        uint16_t got = float32_to_float16_experiment_a(samples[i]);
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
        uint16_t got = float32_to_float16_experiment_a(uf);
        if (got != ref) {
            fail_case("random", uf, got, ref);
        }
    }
}

static void print_fp32_dump(const char *label, float x) {
    uint32_t bits = float_to_bits(x);
    uint32_t sign = bits >> 31;
    uint32_t exponent = (bits >> 23) & 0xffu;
    uint32_t fraction = bits & 0x7fffffu;

    printf("  %-8s value=% .9g raw=0x%08" PRIX32
           " sign=%" PRIu32 " exponent=0x%02" PRIX32 " fraction=0x%06" PRIX32,
           label, (double) x, bits, sign, exponent, fraction);
    if (exponent == 0) {
        puts(" unbiased=subnormal/zero");
    } else if (exponent == 0xffu) {
        puts(" unbiased=special");
    } else {
        printf(" unbiased=%d\n", (int) exponent - 127);
    }
}

static void print_sample_table(void) {
    struct sample {
        const char *label;
        float value;
    };
    static const struct sample samples[] = {
        {"0.0", 0.0f},
        {"1.0", 1.0f},
        {"-1.0", -1.0f},
        {"0.5", 0.5f},
        {"2.0", 2.0f},
        {"2048.0", 2048.0f},
        {"2049.0", 2049.0f},
        {"65504.0", 65504.0f},
        {"65536.0", 65536.0f},
        {"1e-8", 1.0e-8f},
    };

    puts("Experiment A: IEEE754 / FP16 representation lab");
    puts("FP32 field dump");
    for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
        print_fp32_dump(samples[i].label, samples[i].value);
    }

    puts("");
    puts("FP32 -> FP16 conversion table");
    puts("  value        fp32_raw    fp16_raw  decoded         abs_error       exact");
    for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
        float x = samples[i].value;
        uint32_t raw = float_to_bits(x);
        uint16_t h = float32_to_float16_experiment_a(raw);
        float decoded = half_to_float(h);
        double err = abs_double((double) decoded - (double) x);
        int exact = float_to_bits(decoded) == raw;

        printf("  %-10s 0x%08" PRIX32 " 0x%04" PRIX16 "  % .9g  % .9g  %s\n",
               samples[i].label, raw, h, (double) decoded, err, exact ? "yes" : "no");
    }

    puts("");
    puts("special checks");
    printf("  2048 exact:  %s\n",
           float_to_bits(half_to_float(float32_to_float16_experiment_a(float_to_bits(2048.0f)))) ==
                   float_to_bits(2048.0f)
               ? "yes"
               : "no");
    printf("  2049 exact:  %s\n",
           float_to_bits(half_to_float(float32_to_float16_experiment_a(float_to_bits(2049.0f)))) ==
                   float_to_bits(2049.0f)
               ? "yes"
               : "no");
    printf("  65504 maps to max finite fp16 0x7BFF: %s\n",
           float32_to_float16_experiment_a(float_to_bits(65504.0f)) == 0x7bffu ? "yes" : "no");
    printf("  65536 overflows to +inf 0x7C00:      %s\n",
           float32_to_float16_experiment_a(float_to_bits(65536.0f)) == 0x7c00u ? "yes" : "no");
}

int main(void) {
    verify_roundtrip_cases();
    verify_specials();
    verify_random_cases();
    print_sample_table();

    if (float32_to_float16_experiment_a(0x387fe000u) != 0x0400u) {
        fail_case("boundary_min_normal", 0x387fe000u,
                  float32_to_float16_experiment_a(0x387fe000u), 0x0400u);
    }
    if (float32_to_float16_experiment_a(0x7f800000u) != 0x7c00u) {
        fail_case("boundary_inf", 0x7f800000u, float32_to_float16_experiment_a(0x7f800000u), 0x7c00u);
    }
    if (float32_to_float16_experiment_a(0x7fc00000u) != 0x7e00u) {
        fail_case("boundary_nan", 0x7fc00000u, float32_to_float16_experiment_a(0x7fc00000u), 0x7e00u);
    }
    if (float32_to_float16_experiment_a(0x00000001u) != 0x0000u) {
        fail_case("boundary_denorm_to_zero", 0x00000001u,
                  float32_to_float16_experiment_a(0x00000001u), 0x0000u);
    }

    puts("  roundtrip cases: 65536/65536");
    puts("  random cases:    200000/200000");
    puts("  boundary checks: pass");
    return 0;
}
