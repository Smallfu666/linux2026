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

static void ensure_dir(const char *path) {
    if (mkdir(path, 0777) != 0 && errno != EEXIST) {
        perror(path);
        exit(1);
    }
}

int main(void) {
    ensure_dir("results");
    ensure_dir("results/dct_coef");

    int recursive[128];
    int cosine_scaled[128];
    int reference[128];
    int diff[128];
    const double theta = M_PI / 64.0;
    const double scale = 1024.0 * sqrt(2.0);

    cosine_scaled[0] = 1024;
    cosine_scaled[1] = (int) llround(1024.0 * cos(theta));
    recursive[0] = 1024;
    recursive[1] = (int) llround(scale * cos(theta));
    for (int k = 2; k < 128; ++k) {
        double next = 2.0 * cos(theta) * (double) cosine_scaled[k - 1] - (double) cosine_scaled[k - 2];
        cosine_scaled[k] = (int) llround(next);
        recursive[k] = (int) llround(sqrt(2.0) * (double) cosine_scaled[k]);
    }

    for (int k = 0; k < 128; ++k) {
        if (k == 0) {
            reference[k] = 1024;
        } else {
            reference[k] = (int) llround(scale * cos(M_PI * (double) k / 64.0));
        }
        diff[k] = recursive[k] - reference[k];
    }

    FILE *csv = fopen("results/dct_coef/dct_coef_compare.csv", "w");
    if (!csv) {
        perror("results/dct_coef/dct_coef_compare.csv");
        return 1;
    }
    fprintf(csv, "k,recursive,reference,diff,abs_diff\n");
    long long sq_sum = 0;
    int max_abs = 0;
    int mismatch_count = 0;
    for (int k = 0; k < 128; ++k) {
        int ad = diff[k] < 0 ? -diff[k] : diff[k];
        if (ad > max_abs) {
            max_abs = ad;
        }
        if (diff[k] != 0) {
            mismatch_count++;
        }
        sq_sum += (long long) diff[k] * (long long) diff[k];
        fprintf(csv, "%d,%d,%d,%d,%d\n", k, recursive[k], reference[k], diff[k], ad);
    }
    fclose(csv);

    double rmse = sqrt((double) sq_sum / 128.0);
    FILE *summary = fopen("results/dct_coef/summary.txt", "w");
    if (!summary) {
        perror("results/dct_coef/summary.txt");
        return 1;
    }
    fprintf(summary, "max_abs_error=%d\n", max_abs);
    fprintf(summary, "rmse=%.6f\n", rmse);
    fprintf(summary, "mismatch_count=%d\n", mismatch_count);
    fprintf(summary, "recursive_dct_coef_0=%d\n", recursive[0]);
    fprintf(summary, "recursive_dct_coef_1=%d\n", recursive[1]);
    fprintf(summary, "reference_dct_coef_0=%d\n", reference[0]);
    fprintf(summary, "reference_dct_coef_1=%d\n", reference[1]);
    fclose(summary);
    return 0;
}
