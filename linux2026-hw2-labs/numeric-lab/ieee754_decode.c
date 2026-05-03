#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

union float_bits {
    float value;
    uint32_t bits;
};

union double_bits {
    double value;
    uint64_t bits;
};

static void print_u32_binary(uint32_t value)
{
    for (int i = 31; i >= 0; --i) {
        putchar((value & (UINT32_C(1) << i)) ? '1' : '0');
        if (i == 31 || i == 23) {
            putchar(' ');
        }
    }
}

static void print_u64_binary(uint64_t value)
{
    for (int i = 63; i >= 0; --i) {
        putchar((value & (UINT64_C(1) << i)) ? '1' : '0');
        if (i == 63 || i == 52) {
            putchar(' ');
        }
    }
}

static void decode_float(const char *label, float value)
{
    union float_bits repr = { .value = value };
    uint32_t sign = repr.bits >> 31;
    uint32_t exponent = (repr.bits >> 23) & 0xffU;
    uint32_t fraction = repr.bits & 0x7fffffU;

    printf("float  %-10s value=% .9g\n", label, value);
    printf("  hex bits      : 0x%08" PRIx32 "\n", repr.bits);
    printf("  fields        : sign=%" PRIu32 " exponent=0x%02" PRIx32 " (%" PRIu32 ") fraction=0x%06" PRIx32 "\n",
           sign, exponent, exponent, fraction);
    printf("  bit layout    : ");
    print_u32_binary(repr.bits);
    putchar('\n');
}

static void decode_double(const char *label, double value)
{
    union double_bits repr = { .value = value };
    uint64_t sign = repr.bits >> 63;
    uint64_t exponent = (repr.bits >> 52) & 0x7ffULL;
    uint64_t fraction = repr.bits & 0x000fffffffffffffULL;

    printf("double %-10s value=% .17g\n", label, value);
    printf("  hex bits      : 0x%016" PRIx64 "\n", repr.bits);
    printf("  fields        : sign=%" PRIu64 " exponent=0x%03" PRIx64 " (%" PRIu64 ") fraction=0x%013" PRIx64 "\n",
           sign, exponent, exponent, fraction);
    printf("  bit layout    : ");
    print_u64_binary(repr.bits);
    putchar('\n');
}

static void decode_demo_set(void)
{
    float f_values[] = { 0.1f, 0.2f, 0.3f, 0.1f + 0.2f };
    const char *labels[] = { "0.1f", "0.2f", "0.3f", "0.1f+0.2f" };
    double d_values[] = { 0.1, 0.2, 0.3, 0.1 + 0.2 };

    puts("== Float ==");
    for (size_t i = 0; i < sizeof(f_values) / sizeof(f_values[0]); ++i) {
        decode_float(labels[i], f_values[i]);
    }

    puts("\n== Double ==");
    for (size_t i = 0; i < sizeof(d_values) / sizeof(d_values[0]); ++i) {
        decode_double(labels[i], d_values[i]);
    }

    printf("\n0.1 + 0.2 == 0.3 ? %s\n", ((0.1 + 0.2) == 0.3) ? "true" : "false");
}

int main(void)
{
    decode_demo_set();
    return 0;
}
