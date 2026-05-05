#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

uint32_t bit_bad(int n)
{
    return (uint32_t) (1 << n);
}

uint32_t bit_good(int n)
{
    return UINT32_C(1) << n;
}

static void print_i32(const char *label, int32_t value)
{
    printf("  %-24s decimal=%12" PRId32 " hex=0x%08" PRIX32 "\n",
           label, value, (uint32_t) value);
}

static void print_u32(const char *label, uint32_t value)
{
    printf("  %-24s decimal=%12" PRIu32 " hex=0x%08" PRIX32 "\n",
           label, value, value);
}

static void expression_tests(void)
{
    volatile int one = 1;
    volatile unsigned int one_u = 1u;
    volatile uint32_t one_u32 = UINT32_C(1);

    int a = 1 << 30;
    int b = one << 31;
    unsigned int c = one_u << 31;
    uint32_t d = one_u32 << 31;

    puts("expression tests");
    print_i32("int a = 1 << 30", a);
    print_i32("int b = 1 << 31", b);
    print_u32("unsigned c = 1u << 31", c);
    print_u32("uint32_t d = UINT32_C(1) << 31", d);
}

static void function_tests(void)
{
    static const int ns[] = {0, 1, 30, 31};

    puts("function tests");
    for (size_t i = 0; i < sizeof(ns) / sizeof(ns[0]); ++i) {
        int n = ns[i];
        uint32_t bad = bit_bad(n);
        uint32_t good = bit_good(n);

        printf("  n=%2d bit_bad=0x%08" PRIX32 " bit_good=0x%08" PRIX32 "\n",
               n, bad, good);
    }
}

int main(void)
{
    puts("Experiment C: signed shift UB lab");
    puts("UBSan builds should report signed left shift UB for 1 << 31.");
    expression_tests();
    function_tests();
    return 0;
}
