#include "bytecode.h"
#include "interpreter.h"
#include "timing.h"

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile uint64_t sink;

typedef struct {
    pattern_t *items;
    size_t count;
} pattern_list_t;

typedef struct {
    dispatch_mode_t *items;
    size_t count;
} dispatch_list_t;

static void usage(const char *argv0) {
    fprintf(stderr,
            "Usage: %s [--pattern predictable|random|all] [--dispatch switch|computed|all]\n"
            "          [--program-len N] [--steps N] [--reps N] [--seed N] [--csv]\n",
            argv0);
}

static bool parse_size(const char *value, size_t *out) {
    char *end = NULL;
    errno = 0;
    unsigned long long parsed = strtoull(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0') {
        return false;
    }
    *out = (size_t)parsed;
    return true;
}

static bool parse_u64(const char *value, uint64_t *out) {
    char *end = NULL;
    errno = 0;
    unsigned long long parsed = strtoull(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0') {
        return false;
    }
    *out = (uint64_t)parsed;
    return true;
}

static int append_pattern(pattern_list_t *list, pattern_t pattern) {
    pattern_t *items = realloc(list->items, (list->count + 1) * sizeof(*items));
    if (items == NULL) {
        return -1;
    }
    list->items = items;
    list->items[list->count++] = pattern;
    return 0;
}

static int append_dispatch(dispatch_list_t *list, dispatch_mode_t mode) {
    dispatch_mode_t *items = realloc(list->items, (list->count + 1) * sizeof(*items));
    if (items == NULL) {
        return -1;
    }
    list->items = items;
    list->items[list->count++] = mode;
    return 0;
}

static int parse_pattern_list(const char *value, pattern_list_t *list) {
    int ok = 0;
    if (strcmp(value, "all") == 0) {
        if (append_pattern(list, PATTERN_PREDICTABLE) != 0) {
            return -1;
        }
        return append_pattern(list, PATTERN_RANDOM);
    }
    pattern_t pattern = parse_pattern(value, &ok);
    if (!ok) {
        return -1;
    }
    return append_pattern(list, pattern);
}

static int parse_dispatch_list(const char *value, dispatch_list_t *list) {
    int ok = 0;
    if (strcmp(value, "all") == 0) {
        if (append_dispatch(list, DISPATCH_SWITCH) != 0) {
            return -1;
        }
        return append_dispatch(list, DISPATCH_COMPUTED);
    }
    dispatch_mode_t mode = parse_dispatch(value, &ok);
    if (!ok) {
        return -1;
    }
    return append_dispatch(list, mode);
}

int main(int argc, char **argv) {
    size_t program_len = 32768;
    size_t steps = 262144;
    size_t reps = 3;
    uint64_t seed = 1;
    bool csv = false;
    pattern_list_t patterns = {0};
    dispatch_list_t dispatches = {0};
    program_t program = {0};
    bool program_ready = false;

    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (strcmp(arg, "--pattern") == 0 && i + 1 < argc) {
            if (parse_pattern_list(argv[++i], &patterns) != 0) {
                usage(argv[0]);
                goto fail;
            }
        } else if (strcmp(arg, "--dispatch") == 0 && i + 1 < argc) {
            if (parse_dispatch_list(argv[++i], &dispatches) != 0) {
                usage(argv[0]);
                goto fail;
            }
        } else if (strcmp(arg, "--program-len") == 0 && i + 1 < argc) {
            if (!parse_size(argv[++i], &program_len)) {
                usage(argv[0]);
                goto fail;
            }
        } else if (strcmp(arg, "--steps") == 0 && i + 1 < argc) {
            if (!parse_size(argv[++i], &steps)) {
                usage(argv[0]);
                goto fail;
            }
        } else if (strcmp(arg, "--reps") == 0 && i + 1 < argc) {
            if (!parse_size(argv[++i], &reps) || reps == 0) {
                usage(argv[0]);
                goto fail;
            }
        } else if (strcmp(arg, "--seed") == 0 && i + 1 < argc) {
            if (!parse_u64(argv[++i], &seed)) {
                usage(argv[0]);
                goto fail;
            }
        } else if (strcmp(arg, "--csv") == 0) {
            csv = true;
        } else if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            usage(argv[0]);
            free(patterns.items);
            free(dispatches.items);
            return 0;
        } else {
            usage(argv[0]);
            goto fail;
        }
    }

    if (patterns.count == 0) {
        if (append_pattern(&patterns, PATTERN_PREDICTABLE) != 0) {
            goto fail;
        }
    }
    if (dispatches.count == 0) {
        if (append_dispatch(&dispatches, DISPATCH_SWITCH) != 0) {
            goto fail;
        }
    }

    if (csv) {
        printf("pattern,dispatch,program_len,steps,rep,elapsed_ns,checksum\n");
    }

    for (size_t pi = 0; pi < patterns.count; ++pi) {
        if (program_init(&program, program_len, patterns.items[pi], seed + pi) != 0) {
            fprintf(stderr, "failed to initialize program\n");
            goto fail;
        }
        program_ready = true;

        for (size_t di = 0; di < dispatches.count; ++di) {
            for (size_t rep = 0; rep < reps; ++rep) {
                const uint64_t started = now_ns();
                const uint64_t checksum = run_interpreter(dispatches.items[di], &program, steps, seed + rep);
                const uint64_t elapsed = now_ns() - started;
                sink ^= checksum;

                if (csv) {
                    printf("%s,%s,%zu,%zu,%zu,%" PRIu64 ",%" PRIu64 "\n",
                           pattern_name(patterns.items[pi]),
                           dispatch_name(dispatches.items[di]),
                           program_len,
                           steps,
                           rep,
                           elapsed,
                           checksum);
                } else {
                    printf("pattern=%s dispatch=%s len=%zu steps=%zu rep=%zu elapsed_ns=%" PRIu64 " checksum=%" PRIu64 "\n",
                           pattern_name(patterns.items[pi]),
                           dispatch_name(dispatches.items[di]),
                           program_len,
                           steps,
                           rep,
                           elapsed,
                           checksum);
                }
            }
        }

        program_destroy(&program);
        program_ready = false;
    }

    free(patterns.items);
    free(dispatches.items);
    (void)sink;
    return 0;

fail:
    if (program_ready) {
        program_destroy(&program);
    }
    free(patterns.items);
    free(dispatches.items);
    return 1;
}
