#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

union double_bits {
    double value;
    uint64_t bits;
};

static void print_value_line(const char *label, double value)
{
    union double_bits repr = { .value = value };

    printf("  %-12s = % .17g (hex=%a bits=0x%016" PRIx64 ")\n",
           label, value, value, repr.bits);
}

static void run_case(const char *title, double a, double b, double c)
{
    double left = (a + b) + c;
    double right = a + (b + c);
    double diff = left - right;

    printf("== %s ==\n", title);
    print_value_line("a", a);
    print_value_line("b", b);
    print_value_line("c", c);
    print_value_line("(a+b)+c", left);
    print_value_line("a+(b+c)", right);
    print_value_line("difference", diff);
    printf("  equal?       = %s\n\n", left == right ? "true" : "false");
}

int main(void)
{
    run_case("classic cancellation", 3.14, 1e10, -1e10);
    run_case("wider scale gap", 1.0, 1e20, -1e20);
    run_case("negative pair first", 1e20, -1e20, 3.14);
    run_case("subtle rounding", 0.1, 0.2, 0.3);
    return 0;
}
