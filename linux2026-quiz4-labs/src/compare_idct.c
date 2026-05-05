#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    uint64_t state;
} rng_t;

static void ensure_dir(const char *path) {
    if (mkdir(path, 0777) != 0 && errno != EEXIST) {
        perror(path);
        exit(1);
    }
}

static uint32_t rng_u32(rng_t *rng) {
    uint64_t x = rng->state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    rng->state = x;
    return (uint32_t) ((x * 2685821657736338717ULL) >> 32);
}

static int rand_range(rng_t *rng, int lo, int hi) {
    uint32_t span = (uint32_t) (hi - lo + 1);
    return lo + (int) (rng_u32(rng) % span);
}

static void basis_row(int d, int a, double *row) {
    for (int j = 0; j < d; ++j) {
        double scale = (j == 0) ? 1.0 : sqrt(2.0);
        row[j] = cos(M_PI / d * (a + 0.5) * j) * scale;
    }
}

static void transform_fixed(const int *input, int *tmp, int *output, int d) {
    int g = 0;
    for (int n = d; n > 1; n >>= 1) {
        g++;
    }
    double *basis = (double *) calloc((size_t) d * (size_t) d, sizeof(double));
    if (!basis) {
        perror("calloc");
        exit(1);
    }
    for (int a = 0; a < d; ++a) {
        basis_row(d, a, basis + (size_t) a * d);
    }
    for (int h = 0; h < d; ++h) {
        for (int a = 0; a < d; ++a) {
            long long sum = 0;
            for (int j = 0; j < d; ++j) {
                double weight = basis[(size_t) a * d + j] * 1024.0;
                long long q = llround(weight);
                sum += (long long) input[j * d + h] * q;
            }
            tmp[a * d + h] = (int) ((sum + (1LL << 9)) >> 10);
        }
    }
    for (int h = 0; h < d; ++h) {
        for (int a = 0; a < d; ++a) {
            long long sum = 0;
            for (int j = 0; j < d; ++j) {
                double weight = basis[(size_t) a * d + j] * 1024.0;
                long long q = llround(weight);
                sum += (long long) tmp[j * d + h] * q;
            }
            output[a * d + h] = (int) ((sum + (1LL << (9 + g))) >> (10 + g));
        }
    }
    free(basis);
}

static void transform_reference(const int *input, int *tmp, int *output, int d) {
    int g = 0;
    for (int n = d; n > 1; n >>= 1) {
        g++;
    }
    double *basis = (double *) calloc((size_t) d * (size_t) d, sizeof(double));
    double *tmpf = (double *) calloc((size_t) d * (size_t) d, sizeof(double));
    double *outf = (double *) calloc((size_t) d * (size_t) d, sizeof(double));
    if (!basis || !tmpf || !outf) {
        perror("calloc");
        exit(1);
    }
    for (int a = 0; a < d; ++a) {
        basis_row(d, a, basis + (size_t) a * d);
    }
    for (int h = 0; h < d; ++h) {
        for (int a = 0; a < d; ++a) {
            double sum = 0.0;
            for (int j = 0; j < d; ++j) {
                sum += (double) input[j * d + h] * basis[(size_t) a * d + j];
            }
            tmpf[a * d + h] = llround(sum);
            tmp[a * d + h] = (int) tmpf[a * d + h];
        }
    }
    for (int h = 0; h < d; ++h) {
        for (int a = 0; a < d; ++a) {
            double sum = 0.0;
            for (int j = 0; j < d; ++j) {
                sum += tmpf[j * d + h] * basis[(size_t) a * d + j];
            }
            outf[a * d + h] = llround(sum / (double) (1 << g));
            output[a * d + h] = (int) outf[a * d + h];
        }
    }
    free(basis);
    free(tmpf);
    free(outf);
}

static void evaluate_case(FILE *csv, int d, const char *range_name, int lo, int hi, rng_t *rng) {
    int *input = (int *) calloc((size_t) d * (size_t) d, sizeof(int));
    int *tmp_fixed = (int *) calloc((size_t) d * (size_t) d, sizeof(int));
    int *out_fixed = (int *) calloc((size_t) d * (size_t) d, sizeof(int));
    int *tmp_ref = (int *) calloc((size_t) d * (size_t) d, sizeof(int));
    int *out_ref = (int *) calloc((size_t) d * (size_t) d, sizeof(int));
    if (!input || !tmp_fixed || !out_fixed || !tmp_ref || !out_ref) {
        perror("calloc");
        exit(1);
    }
    for (int trial = 0; trial < 1000; ++trial) {
        for (int i = 0; i < d * d; ++i) {
            input[i] = rand_range(rng, lo, hi);
        }
        transform_fixed(input, tmp_fixed, out_fixed, d);
        transform_reference(input, tmp_ref, out_ref, d);

        long long sq_sum = 0;
        long long abs_sum = 0;
        int max_abs = 0;
        for (int i = 0; i < d * d; ++i) {
            int diff = out_fixed[i] - out_ref[i];
            int ad = diff < 0 ? -diff : diff;
            if (ad > max_abs) {
                max_abs = ad;
            }
            sq_sum += (long long) diff * (long long) diff;
            abs_sum += ad;
        }
        double rmse = sqrt((double) sq_sum / (double) (d * d));
        double mean_abs = (double) abs_sum / (double) (d * d);
        fprintf(csv, "%d,%s,%d,%d,%.6f,%.6f\n", d, range_name, trial, max_abs, rmse, mean_abs);
    }

    free(input);
    free(tmp_fixed);
    free(out_fixed);
    free(tmp_ref);
    free(out_ref);
}

int main(void) {
    ensure_dir("results");
    ensure_dir("results/idct");

    FILE *csv = fopen("results/idct/idct_error.csv", "w");
    if (!csv) {
        perror("results/idct/idct_error.csv");
        return 1;
    }
    fprintf(csv, "block_size,range_name,trial,max_abs_error,rmse,mean_abs_error\n");

    rng_t rng = {0x20260404u};
    const struct {
        const char *name;
        int lo;
        int hi;
    } ranges[] = {
        {"small", -8, 8},
        {"medium", -128, 128},
        {"large", -1024, 1024},
    };
    const int sizes[] = {4, 8, 16, 32};

    for (size_t ri = 0; ri < sizeof(ranges) / sizeof(ranges[0]); ++ri) {
        for (size_t si = 0; si < sizeof(sizes) / sizeof(sizes[0]); ++si) {
            evaluate_case(csv, sizes[si], ranges[ri].name, ranges[ri].lo, ranges[ri].hi, &rng);
        }
    }
    fclose(csv);

    FILE *summary = fopen("results/idct/summary.txt", "w");
    if (!summary) {
        perror("results/idct/summary.txt");
        return 1;
    }
    csv = fopen("results/idct/idct_error.csv", "r");
    if (!csv) {
        perror("results/idct/idct_error.csv");
        return 1;
    }
    char line[256];
    if (!fgets(line, sizeof line, csv)) {
        fclose(csv);
        fclose(summary);
        return 1;
    }
    int global_max = 0;
    double mean_rmse_sum = 0.0;
    int row_count = 0;
    char worst_trial[128] = "UNKNOWN";
    while (fgets(line, sizeof line, csv)) {
        int block_size, trial, max_abs_error;
        char range_name[32];
        double rmse, mean_abs_error;
        if (sscanf(line, "%d,%31[^,],%d,%d,%lf,%lf", &block_size, range_name, &trial, &max_abs_error, &rmse, &mean_abs_error) != 6) {
            continue;
        }
        if (max_abs_error > global_max) {
            global_max = max_abs_error;
            snprintf(worst_trial, sizeof worst_trial, "block_size=%d range_name=%s trial=%d max_abs_error=%d", block_size, range_name, trial, max_abs_error);
        }
        mean_rmse_sum += rmse;
        row_count++;
    }
    fclose(csv);

    fprintf(summary, "global_max_abs_error=%d\n", global_max);
    fprintf(summary, "mean_rmse=%.6f\n", row_count ? mean_rmse_sum / row_count : 0.0);
    fprintf(summary, "worst_trial=%s\n", worst_trial);
    fclose(summary);
    return 0;
}
