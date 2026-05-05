#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>

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

static int sample_bit(rng_t *rng, double p1) {
    return ((double) rng_u32(rng) / (double) UINT32_MAX) < p1;
}

typedef struct {
    int c0;
    int c1;
    int observations;
    int halving_count;
} ctx_t;

static void write_step(FILE *csv, const char *case_name, int step, int bit, const ctx_t *ctx, int halved) {
    double p0 = (ctx->c0 + 1.0) / (ctx->observations);
    double p1 = (ctx->c1 + 1.0) / (ctx->observations);
    fprintf(csv, "%s,%d,%d,%d,%d,%d,%.6f,%.6f,%d\n",
            case_name, step, bit, ctx->c0, ctx->c1, ctx->observations, p0, p1, halved);
}

static void step_context(ctx_t *ctx, int bit) {
    int observations_before = ctx->c0 + ctx->c1 + 2;
    if (bit) {
        ctx->c1++;
    } else {
        ctx->c0++;
    }
    if (observations_before > 63) {
        ctx->c0 /= 2;
        ctx->c1 /= 2;
        ctx->halving_count++;
    }
    ctx->observations = ctx->c0 + ctx->c1 + 2;
}

static void run_case(FILE *csv, FILE *summary, const char *case_name, const int *bits, size_t nbits) {
    ctx_t ctx = {0, 0, 2, 0};
    int first_step = -1;
    for (size_t i = 0; i < nbits; ++i) {
        int bit = bits[i];
        int observations_before = ctx.c0 + ctx.c1 + 2;
        step_context(&ctx, bit);
        double p1 = (ctx.c1 + 1.0) / (ctx.observations);
        if (first_step < 0 && p1 > 0.5) {
            first_step = (int) i + 1;
        }
        int halved = observations_before > 63 ? 1 : 0;
        write_step(csv, case_name, (int) i + 1, bit, &ctx, halved);
    }
    fprintf(summary, "%s,%d,%d,%d,%.6f,%.6f,%d\n",
            case_name, first_step, ctx.c0, ctx.c1,
            (ctx.c0 + 1.0) / (ctx.observations), (ctx.c1 + 1.0) / (ctx.observations), ctx.halving_count);
}

static void make_bits_from_probs(int *bits, size_t nbits, double p1, rng_t *rng) {
    for (size_t i = 0; i < nbits; ++i) {
        bits[i] = sample_bit(rng, p1);
    }
}

int main(void) {
    ensure_dir("results");
    ensure_dir("results/context_model");

    FILE *csv = fopen("results/context_model/context_adaptation.csv", "w");
    FILE *summary = fopen("results/context_model/summary.csv", "w");
    if (!csv || !summary) {
        perror("results/context_model");
        return 1;
    }
    fprintf(csv, "case_name,step,bit,c0,c1,observations,p0,p1,halved\n");
    fprintf(summary, "case_name,first_step_p1_gt_0_5,final_c0,final_c1,final_p0,final_p1,halving_count\n");

    int case_a[128];
    int case_b[256];
    int case_c[128];
    int case_d_01[256];
    int case_d_05[256];
    int case_d_09[256];
    for (int i = 0; i < 64; ++i) {
        case_a[i] = 0;
        case_a[64 + i] = 1;
    }
    for (int i = 0; i < 128; ++i) {
        case_b[i] = 0;
        case_b[128 + i] = 1;
        case_c[i] = i & 1;
    }

    rng_t rng = {0xC0FFEEu};
    make_bits_from_probs(case_d_01, 256, 0.1, &rng);
    make_bits_from_probs(case_d_05, 256, 0.5, &rng);
    make_bits_from_probs(case_d_09, 256, 0.9, &rng);

    run_case(csv, summary, "case_a", case_a, 128);
    run_case(csv, summary, "case_b", case_b, 256);
    run_case(csv, summary, "case_c", case_c, 128);
    run_case(csv, summary, "case_d_p010", case_d_01, 256);
    run_case(csv, summary, "case_d_p050", case_d_05, 256);
    run_case(csv, summary, "case_d_p090", case_d_09, 256);

    fclose(csv);
    fclose(summary);
    return 0;
}
