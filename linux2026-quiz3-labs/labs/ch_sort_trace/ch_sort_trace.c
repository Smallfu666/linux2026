#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void die(const char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

static void swap_int(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

static uint64_t xorshift64star(uint64_t *state) {
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * 2685821657736338717ULL;
}

static size_t random_below(uint64_t *state, size_t bound) {
    if (bound == 0) {
        return 0;
    }
    return (size_t)(xorshift64star(state) % bound);
}

static void fill_permutation(int *a, size_t n, uint64_t seed) {
    for (size_t i = 0; i < n; ++i) {
        a[i] = (int)i;
    }
    for (size_t i = n; i > 1; --i) {
        size_t j = random_below(&seed, i);
        swap_int(&a[i - 1], &a[j]);
    }
}

static int is_sorted(const int *a, size_t n) {
    for (size_t i = 1; i < n; ++i) {
        if (a[i - 1] > a[i]) {
            return 0;
        }
    }
    return 1;
}

static inline int key_less(int a, int b, uint64_t *comparisons) {
    ++*comparisons;
    return a < b;
}

static void sift_down_standard(int *a, size_t start, size_t end, uint64_t *comparisons) {
    size_t root = start;

    for (;;) {
        size_t left = root * 2 + 1;
        if (left >= end) {
            break;
        }

        size_t child = left;
        size_t right = left + 1;
        if (right < end && key_less(a[child], a[right], comparisons)) {
            child = right;
        }

        if (!key_less(a[root], a[child], comparisons)) {
            break;
        }

        swap_int(&a[root], &a[child]);
        root = child;
    }
}

static void sift_down_bottom_up(int *a, size_t start, size_t end, uint64_t *comparisons) {
    size_t root = start;
    for (;;) {
        size_t left = root * 2 + 1;
        if (left >= end) {
            break;
        }

        size_t child = left;
        size_t right = left + 1;
        if (right < end && key_less(a[child], a[right], comparisons)) {
            child = right;
        }

        a[root] = a[child];
        root = child;
    }

    int value = a[start];
    while (root > start) {
        size_t parent = (root - 1) / 2;
        if (!key_less(a[parent], value, comparisons)) {
            break;
        }
        a[root] = a[parent];
        root = parent;
    }
    a[root] = value;
}

static uint64_t heapsort_standard(int *a, size_t n) {
    uint64_t comparisons = 0;
    if (n < 2) {
        return 0;
    }

    for (size_t start = n / 2; start > 0; --start) {
        sift_down_standard(a, start - 1, n, &comparisons);
    }
    for (size_t end = n; end > 1; --end) {
        swap_int(&a[0], &a[end - 1]);
        sift_down_standard(a, 0, end - 1, &comparisons);
    }
    return comparisons;
}

static uint64_t heapsort_bottom_up(int *a, size_t n) {
    uint64_t comparisons = 0;
    if (n < 2) {
        return 0;
    }

    for (size_t start = n / 2; start > 0; --start) {
        sift_down_bottom_up(a, start - 1, n, &comparisons);
    }
    for (size_t end = n; end > 1; --end) {
        swap_int(&a[0], &a[end - 1]);
        sift_down_bottom_up(a, 0, end - 1, &comparisons);
    }
    return comparisons;
}

static size_t count_trailing_ones(size_t value) {
    size_t count = 0;
    while ((value & 1U) != 0U) {
        ++count;
        value >>= 1;
    }
    return count;
}

static size_t highest_power_of_two_le(size_t value) {
    size_t bit = 1;
    if (value == 0) {
        return 0;
    }
    while (bit <= value / 2) {
        bit <<= 1;
    }
    return bit;
}

static size_t runs_from_count(size_t count, size_t *runs, size_t max_runs) {
    size_t nruns = 0;
    size_t bit = highest_power_of_two_le(count);
    while (bit != 0 && nruns < max_runs) {
        if ((count & bit) != 0) {
            runs[nruns++] = bit;
        }
        bit >>= 1;
    }
    return nruns;
}

static void format_binary(size_t value, char *buf, size_t buflen) {
    char tmp[sizeof(size_t) * 8];
    size_t len = 0;

    if (buflen == 0) {
        return;
    }

    if (value == 0) {
        if (buflen > 1) {
            buf[0] = '0';
            buf[1] = '\0';
        } else {
            buf[0] = '\0';
        }
        return;
    }

    while (value != 0 && len < sizeof(tmp)) {
        tmp[len++] = (value & 1U) ? '1' : '0';
        value >>= 1;
    }

    size_t out = 0;
    while (len > 0 && out + 1 < buflen) {
        buf[out++] = tmp[--len];
    }
    buf[out] = '\0';
}

static void format_runs(const size_t *runs, size_t nruns, char *buf, size_t buflen) {
    size_t off = 0;

    if (buflen == 0) {
        return;
    }

    for (size_t i = 0; i < nruns; ++i) {
        int written = snprintf(buf + off, buflen - off, "%s%zu", i == 0 ? "" : "|", runs[i]);
        if (written < 0) {
            buf[0] = '\0';
            return;
        }
        if ((size_t)written >= buflen - off) {
            buf[buflen - 1] = '\0';
            return;
        }
        off += (size_t)written;
    }

    if (nruns == 0) {
        buf[0] = '\0';
    }
}

static void write_trace_csv(FILE *out, size_t n) {
    fputs("count,bits,trailing_ones,merge,pending_runs\n", out);

    for (size_t count = 1; count <= n; ++count) {
        char bits[sizeof(size_t) * 8 + 1];
        char runs_buf[128];
        size_t runs[sizeof(size_t) * 8];
        size_t nruns;
        size_t trailing_ones = count_trailing_ones(count - 1);

        format_binary(count, bits, sizeof(bits));
        nruns = runs_from_count(count, runs, sizeof(runs) / sizeof(runs[0]));
        format_runs(runs, nruns, runs_buf, sizeof(runs_buf));

        fprintf(out, "%zu,%s,%zu,%s,%s\n",
                count,
                bits,
                trailing_ones,
                trailing_ones ? "yes" : "no",
                runs_buf);
    }
}

static uint64_t bench_seed(size_t n) {
    return 0x6a09e667f3bcc909ULL ^ ((uint64_t)n * 0x9e3779b97f4a7c15ULL);
}

static void bench_one(size_t n, uint64_t *standard, uint64_t *bottom_up) {
    int *base = malloc(n * sizeof(*base));
    int *work = malloc(n * sizeof(*work));
    if (base == NULL || work == NULL) {
        free(base);
        free(work);
        die("allocation failed");
    }

    fill_permutation(base, n, bench_seed(n));

    memcpy(work, base, n * sizeof(*work));
    *standard = heapsort_standard(work, n);
    if (!is_sorted(work, n)) {
        free(base);
        free(work);
        die("standard heapsort failed");
    }

    memcpy(work, base, n * sizeof(*work));
    *bottom_up = heapsort_bottom_up(work, n);
    if (!is_sorted(work, n)) {
        free(base);
        free(work);
        die("bottom-up heapsort failed");
    }

    free(base);
    free(work);
}

static void write_bench_csv(FILE *out) {
    static const size_t sizes[] = {10000, 100000, 1000000};

    fputs("n,standard_comparisons,bottom_up_comparisons,delta,ratio\n", out);
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
        uint64_t standard = 0;
        uint64_t bottom_up = 0;
        bench_one(sizes[i], &standard, &bottom_up);
        fprintf(out, "%zu,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%.6f\n",
               sizes[i],
               standard,
               bottom_up,
               standard - bottom_up,
               standard == 0 ? 0.0 : (double)bottom_up / (double)standard);
    }
}

static void usage(const char *argv0) {
    fprintf(stderr,
            "usage: %s trace [--n N] | bench\n"
            "  trace  emit the list_sort pending-run trace as CSV\n"
            "  bench  emit heapsort comparison counts as CSV\n",
            argv0);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "trace") == 0) {
        size_t n = 13;
        for (int i = 2; i < argc; ++i) {
            if ((strcmp(argv[i], "--n") == 0 || strcmp(argv[i], "-n") == 0) && i + 1 < argc) {
                n = (size_t)strtoull(argv[++i], NULL, 10);
            } else {
                usage(argv[0]);
                return 1;
            }
        }
        write_trace_csv(stdout, n);
        return 0;
    }

    if (strcmp(argv[1], "bench") == 0) {
        if (argc != 2) {
            usage(argv[0]);
            return 1;
        }
        write_bench_csv(stdout);
        return 0;
    }

    usage(argv[0]);
    return 1;
}
