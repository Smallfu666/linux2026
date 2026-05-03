#include "list.h"

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum algorithm_kind {
    ALG_FAST_SLOW = 0,
    ALG_TWO_PASS,
    ALG_ALL,
};

struct bench_options {
    enum algorithm_kind algorithm;
    enum alloc_mode alloc_mode;
    bool all_alloc_modes;
    size_t n;
    size_t reps;
    uint64_t seed;
    size_t spread_bytes;
    bool csv;
    bool header;
};

static volatile uint64_t g_sink;

static void print_usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [options]\n"
            "  --algorithm fast_slow|two_pass|all\n"
            "  --alloc contiguous|malloc|shuffle|page_spread|all\n"
            "  --n <nodes>\n"
            "  --reps <count>\n"
            "  --seed <u64>\n"
            "  --spread-bytes <bytes>\n"
            "  --csv\n"
            "  --header\n",
            argv0);
}

static const char *algorithm_name(enum algorithm_kind algorithm)
{
    switch (algorithm) {
    case ALG_FAST_SLOW:
        return "fast_slow";
    case ALG_TWO_PASS:
        return "two_pass";
    case ALG_ALL:
        return "all";
    default:
        return "unknown";
    }
}

static int parse_algorithm(const char *text, enum algorithm_kind *algorithm_out)
{
    if (text == NULL || algorithm_out == NULL) {
        return -1;
    }

    if (strcmp(text, "fast_slow") == 0) {
        *algorithm_out = ALG_FAST_SLOW;
        return 0;
    }

    if (strcmp(text, "two_pass") == 0) {
        *algorithm_out = ALG_TWO_PASS;
        return 0;
    }

    if (strcmp(text, "all") == 0) {
        *algorithm_out = ALG_ALL;
        return 0;
    }

    return -1;
}

static int parse_size_arg(const char *text, size_t *out)
{
    char *endptr = NULL;
    unsigned long long value;

    errno = 0;
    value = strtoull(text, &endptr, 10);
    if (errno != 0 || endptr == text || *endptr != '\0') {
        return -1;
    }

    *out = (size_t)value;
    return 0;
}

static int parse_u64_arg(const char *text, uint64_t *out)
{
    char *endptr = NULL;
    unsigned long long value;

    errno = 0;
    value = strtoull(text, &endptr, 10);
    if (errno != 0 || endptr == text || *endptr != '\0') {
        return -1;
    }

    *out = (uint64_t)value;
    return 0;
}

static uint64_t timespec_delta_ns(const struct timespec *start, const struct timespec *end)
{
    time_t sec = end->tv_sec - start->tv_sec;
    long nsec = end->tv_nsec - start->tv_nsec;

    if (nsec < 0) {
        --sec;
        nsec += 1000000000L;
    }

    return ((uint64_t)sec * 1000000000ULL) + (uint64_t)nsec;
}

static int benchmark_once(enum algorithm_kind algorithm,
                          enum alloc_mode alloc_mode,
                          size_t n,
                          size_t trial,
                          uint64_t seed,
                          size_t spread_bytes,
                          bool csv,
                          uint64_t *checksum_out)
{
    struct list_handle list;
    const struct node *middle;
    struct timespec start_ts;
    struct timespec end_ts;
    uint64_t elapsed_ns;
    uint64_t checksum;
    char errbuf[256];

    if (list_build(&list, n, alloc_mode, seed, spread_bytes, errbuf, sizeof(errbuf)) != 0) {
        fprintf(stderr, "list_build failed: %s\n", errbuf);
        return -1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start_ts);
    if (algorithm == ALG_FAST_SLOW) {
        middle = middle_fast_slow(list.head);
    } else {
        middle = middle_two_pass(list.head);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_ts);

    elapsed_ns = timespec_delta_ns(&start_ts, &end_ts);
    checksum = middle == NULL ? 0 : (middle->payload ^ (uint64_t)((uintptr_t)middle >> 4) ^ seed);
    g_sink ^= checksum;

    if (csv) {
        printf("%s,%s,%zu,%zu,%" PRIu64 ",%" PRIu64 "\n",
               algorithm_name(algorithm),
               alloc_mode_name(alloc_mode),
               n,
               trial,
               elapsed_ns,
               checksum);
    } else {
        printf("algorithm=%s alloc_mode=%s n=%zu trial=%zu elapsed_ns=%" PRIu64 " checksum=%" PRIu64 "\n",
               algorithm_name(algorithm),
               alloc_mode_name(alloc_mode),
               n,
               trial,
               elapsed_ns,
               checksum);
    }

    list_destroy(&list);
    if (checksum_out != NULL) {
        *checksum_out = checksum;
    }
    return 0;
}

static int run_benchmarks(const struct bench_options *options)
{
    const enum algorithm_kind algorithms[] = { ALG_FAST_SLOW, ALG_TWO_PASS };
    const enum alloc_mode alloc_modes[] = {
        ALLOC_CONTIGUOUS,
        ALLOC_MALLOC,
        ALLOC_SHUFFLE,
        ALLOC_PAGE_SPREAD,
    };
    size_t algorithm_count = options->algorithm == ALG_ALL ? 2U : 1U;
    size_t alloc_count = options->all_alloc_modes ? 4U : 1U;
    uint64_t checksum_acc = 0;

    if (options->csv && options->header) {
        puts("algorithm,alloc_mode,n,trial,elapsed_ns,checksum");
    }

    for (size_t ai = 0; ai < algorithm_count; ++ai) {
        enum algorithm_kind algorithm =
            options->algorithm == ALG_ALL ? algorithms[ai] : options->algorithm;

        for (size_t mi = 0; mi < alloc_count; ++mi) {
            enum alloc_mode alloc_mode =
                options->all_alloc_modes ? alloc_modes[mi] : options->alloc_mode;

            for (size_t trial = 1; trial <= options->reps; ++trial) {
                uint64_t checksum = 0;

                if (benchmark_once(algorithm,
                                   alloc_mode,
                                   options->n,
                                   trial,
                                   options->seed + trial - 1,
                                   options->spread_bytes,
                                   options->csv,
                                   &checksum) != 0) {
                    return 1;
                }
                checksum_acc ^= checksum;
            }
        }
    }

    g_sink ^= checksum_acc;
    return 0;
}

int main(int argc, char **argv)
{
    struct bench_options options = {
        .algorithm = ALG_FAST_SLOW,
        .alloc_mode = ALLOC_CONTIGUOUS,
        .all_alloc_modes = false,
        .n = 10000,
        .reps = 1,
        .seed = 1,
        .spread_bytes = 4096,
        .csv = false,
        .header = false,
    };

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            if (parse_algorithm(argv[++i], &options.algorithm) != 0) {
                fprintf(stderr, "invalid algorithm: %s\n", argv[i]);
                return 1;
            }
            continue;
        }

        if (strcmp(argv[i], "--alloc") == 0 && i + 1 < argc) {
            if (strcmp(argv[i + 1], "all") == 0) {
                options.all_alloc_modes = true;
                ++i;
                continue;
            }

            if (parse_alloc_mode(argv[++i], &options.alloc_mode) != 0) {
                fprintf(stderr, "invalid alloc mode: %s\n", argv[i]);
                return 1;
            }
            continue;
        }

        if (strcmp(argv[i], "--n") == 0 && i + 1 < argc) {
            if (parse_size_arg(argv[++i], &options.n) != 0) {
                fprintf(stderr, "invalid n: %s\n", argv[i]);
                return 1;
            }
            continue;
        }

        if (strcmp(argv[i], "--reps") == 0 && i + 1 < argc) {
            if (parse_size_arg(argv[++i], &options.reps) != 0 || options.reps == 0) {
                fprintf(stderr, "invalid reps: %s\n", argv[i]);
                return 1;
            }
            continue;
        }

        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            if (parse_u64_arg(argv[++i], &options.seed) != 0) {
                fprintf(stderr, "invalid seed: %s\n", argv[i]);
                return 1;
            }
            continue;
        }

        if (strcmp(argv[i], "--spread-bytes") == 0 && i + 1 < argc) {
            if (parse_size_arg(argv[++i], &options.spread_bytes) != 0) {
                fprintf(stderr, "invalid spread-bytes: %s\n", argv[i]);
                return 1;
            }
            continue;
        }

        if (strcmp(argv[i], "--csv") == 0) {
            options.csv = true;
            continue;
        }

        if (strcmp(argv[i], "--header") == 0) {
            options.header = true;
            continue;
        }

        print_usage(argv[0]);
        return 1;
    }

    return run_benchmarks(&options);
}
