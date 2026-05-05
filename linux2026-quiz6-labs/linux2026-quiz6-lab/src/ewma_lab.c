#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

enum {
    STEPS = 100,
};

static int weight_shift_from_rcp(int weight_rcp)
{
    int shift = 0;
    int x = weight_rcp;

    while (x > 1) {
        if ((x & 1) != 0) {
            return -1;
        }
        x >>= 1;
        ++shift;
    }
    return shift;
}

static int sample_value(const char *name, int step)
{
    if (name[0] == 'c') {
        return 100;
    }
    if (name[0] == 's' && name[1] == 't') {
        return step < 50 ? 0 : 100;
    }
    if (name[0] == 's' && name[1] == 'm') {
        return 1;
    }
    return (step & 1) ? 100 : 0;
}

static double abs_double(double x)
{
    return x < 0.0 ? -x : x;
}

static void run_one(FILE *csv, const char *sequence, int precision, int weight_rcp)
{
    int weight_shift = weight_shift_from_rcp(weight_rcp);
    double alpha = 1.0 / (double) weight_rcp;
    double f_avg = 0.0;
    uint64_t fixed_internal = 0;
    uint64_t scale = UINT64_C(1) << precision;

    if (weight_shift < 0) {
        fprintf(stderr, "weight_rcp=%d is not a power of two\n", weight_rcp);
        exit(1);
    }

    for (int step = 0; step < STEPS; ++step) {
        int x = sample_value(sequence, step);
        double fixed_avg;
        double err;

        f_avg = f_avg * (1.0 - alpha) + (double) x * alpha;
        fixed_internal = fixed_internal - (fixed_internal >> weight_shift) +
                         (((uint64_t) x << precision) >> weight_shift);
        fixed_avg = (double) fixed_internal / (double) scale;
        err = abs_double(f_avg - fixed_avg);

        fprintf(csv, "%s,%d,%d,%d,%d,%.9f,%.9f,%.9f\n",
                sequence, precision, weight_rcp, step, x, f_avg, fixed_avg, err);
    }

    printf("  %-12s precision=%2d weight_rcp=%2d final_float=%9.4f final_fixed=%9.4f\n",
           sequence, precision, weight_rcp, f_avg, (double) fixed_internal / (double) scale);
}

int main(void)
{
    static const char *sequences[] = {
        "constant100",
        "step0to100",
        "small1",
        "alternating",
    };
    static const int precisions[] = {0, 3, 8, 10};
    static const int weights[] = {8, 16};

    mkdir("results", 0777);
    FILE *csv = fopen("results/ewma_results.csv", "w");
    if (csv == NULL) {
        perror("results/ewma_results.csv");
        return 1;
    }

    puts("Experiment D: fixed-point EWMA lab");
    fputs("sequence_name,precision,weight_rcp,step,input,float_avg,fixed_avg,abs_error\n", csv);

    for (size_t s = 0; s < sizeof(sequences) / sizeof(sequences[0]); ++s) {
        for (size_t p = 0; p < sizeof(precisions) / sizeof(precisions[0]); ++p) {
            for (size_t w = 0; w < sizeof(weights) / sizeof(weights[0]); ++w) {
                run_one(csv, sequences[s], precisions[p], weights[w]);
            }
        }
    }

    fclose(csv);
    puts("wrote results/ewma_results.csv");
    return 0;
}
