#include "bench_mode_common.h"

#include "bytecode.h"
#include "timing.h"

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile uint64_t sink;

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s predictable|random\n"
            "Env overrides: PROGRAM_LEN STEPS REPS SEED CSV=1\n",
            argv0);
}

static bool parse_size_env(const char *name, size_t default_value, size_t *out)
{
    const char *value = getenv(name);
    char *end = NULL;
    unsigned long long parsed;

    if (value == NULL || *value == '\0') {
        *out = default_value;
        return true;
    }

    errno = 0;
    parsed = strtoull(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0') {
        return false;
    }

    *out = (size_t)parsed;
    return true;
}

static bool parse_u64_env(const char *name, uint64_t default_value, uint64_t *out)
{
    const char *value = getenv(name);
    char *end = NULL;
    unsigned long long parsed;

    if (value == NULL || *value == '\0') {
        *out = default_value;
        return true;
    }

    errno = 0;
    parsed = strtoull(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0') {
        return false;
    }

    *out = (uint64_t)parsed;
    return true;
}

int run_single_dispatch_main(dispatch_mode_t mode, int argc, char **argv)
{
    pattern_t pattern = PATTERN_PREDICTABLE;
    size_t program_len = 32768;
    size_t steps = 1000000;
    size_t reps = 10;
    uint64_t seed = 1;
    bool csv = false;
    int ok = 0;
    program_t program = {0};

    if (argc > 2) {
        usage(argv[0]);
        return 1;
    }

    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            usage(argv[0]);
            return 0;
        }

        pattern = parse_pattern(argv[1], &ok);
        if (!ok) {
            usage(argv[0]);
            return 1;
        }
    }

    if (!parse_size_env("PROGRAM_LEN", program_len, &program_len) ||
        !parse_size_env("STEPS", steps, &steps) ||
        !parse_size_env("REPS", reps, &reps) ||
        !parse_u64_env("SEED", seed, &seed) ||
        reps == 0 || program_len == 0 || steps == 0) {
        usage(argv[0]);
        return 1;
    }

    csv = getenv("CSV") != NULL;

    if (program_init(&program, program_len, pattern, seed) != 0) {
        fprintf(stderr, "failed to initialize program\n");
        return 1;
    }

    if (csv) {
        printf("pattern,dispatch,program_len,steps,rep,elapsed_ns,checksum\n");
    }

    for (size_t rep = 0; rep < reps; ++rep) {
        const uint64_t started = now_ns();
        const uint64_t checksum = run_interpreter(mode, &program, steps, seed + rep);
        const uint64_t elapsed = now_ns() - started;

        sink ^= checksum;
        if (csv) {
            printf("%s,%s,%zu,%zu,%zu,%" PRIu64 ",%" PRIu64 "\n",
                   pattern_name(pattern),
                   dispatch_name(mode),
                   program_len,
                   steps,
                   rep,
                   elapsed,
                   checksum);
        } else {
            printf("pattern=%s dispatch=%s program_len=%zu steps=%zu rep=%zu elapsed_ns=%" PRIu64 " checksum=%" PRIu64 "\n",
                   pattern_name(pattern),
                   dispatch_name(mode),
                   program_len,
                   steps,
                   rep,
                   elapsed,
                   checksum);
        }
    }

    program_destroy(&program);
    (void)sink;
    return 0;
}
