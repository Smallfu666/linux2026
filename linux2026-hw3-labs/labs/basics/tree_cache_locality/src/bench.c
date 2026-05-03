#include "tree.h"
#include "timing.h"

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile uint64_t sink;

static const char *match_arg_value(const char *arg, const char *name)
{
    size_t len = strlen(name);

    if (strcmp(arg, name) == 0) {
        return "";
    }

    if (strncmp(arg, name, len) == 0 && arg[len] == '=') {
        return arg + len + 1;
    }

    return NULL;
}

static void usage(const char *argv0) {
    fprintf(stderr,
            "Usage: %s [--mode bst|btree16|all] [--type bst|btree16|all] [--key-count N] [--lookups N]\n"
            "          [--reps N] [--seed N] [--csv]\n",
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

static uint64_t next_rng(uint64_t *state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

static int append_mode(mode_list_t *list, tree_mode_t mode) {
    tree_mode_t *items = realloc(list->items, (list->count + 1) * sizeof(*items));
    if (items == NULL) {
        return -1;
    }
    list->items = items;
    list->items[list->count++] = mode;
    return 0;
}

static int parse_mode_list(const char *value, mode_list_t *list) {
    int ok = 0;
    if (strcmp(value, "all") == 0) {
        if (append_mode(list, TREE_MODE_BST) != 0) {
            return -1;
        }
        return append_mode(list, TREE_MODE_BTREE16);
    }
    tree_mode_t mode = parse_mode(value, &ok);
    if (!ok) {
        return -1;
    }
    return append_mode(list, mode);
}

static int *make_keys(size_t count) {
    int *keys = malloc(count * sizeof(*keys));
    if (keys == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < count; ++i) {
        keys[i] = (int)(i * 2u + 1u);
    }
    return keys;
}

static int *make_queries(const int *keys, size_t key_count, size_t lookup_count, uint64_t seed) {
    int *queries = malloc(lookup_count * sizeof(*queries));
    if (queries == NULL) {
        return NULL;
    }

    uint64_t rng = seed ? seed : 0x0123456789abcdefULL;
    for (size_t i = 0; i < lookup_count; ++i) {
        queries[i] = keys[next_rng(&rng) % key_count];
    }
    return queries;
}

static uint64_t run_bst(const bst_tree_t *tree, const int *queries, size_t lookup_count) {
    uint64_t checksum = 0;
    for (size_t i = 0; i < lookup_count; ++i) {
        checksum += (uint64_t)bst_tree_lookup(tree, queries[i]);
    }
    return checksum;
}

static uint64_t run_btree16(const btree_tree_t *tree, const int *queries, size_t lookup_count) {
    uint64_t checksum = 0;
    for (size_t i = 0; i < lookup_count; ++i) {
        checksum += (uint64_t)btree_tree_lookup(tree, queries[i]);
    }
    return checksum;
}

int main(int argc, char **argv) {
    size_t key_count = 65536;
    size_t lookup_count = 262144;
    size_t reps = 3;
    uint64_t seed = 1;
    bool csv = false;
    mode_list_t modes = {0};

    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        const char *value = NULL;

        value = match_arg_value(arg, "--mode");
        if (value == NULL) {
            value = match_arg_value(arg, "--type");
        }
        if (value != NULL) {
            if (*value == '\0') {
                if (i + 1 >= argc) {
                    usage(argv[0]);
                    goto fail;
                }
                value = argv[++i];
            }
            if (parse_mode_list(value, &modes) != 0) {
                usage(argv[0]);
                goto fail;
            }
        } else if ((value = match_arg_value(arg, "--key-count")) != NULL) {
            if (*value == '\0') {
                if (i + 1 >= argc) {
                    usage(argv[0]);
                    goto fail;
                }
                value = argv[++i];
            }
            if (!parse_size(value, &key_count)) {
                usage(argv[0]);
                goto fail;
            }
        } else if ((value = match_arg_value(arg, "--lookups")) != NULL) {
            if (*value == '\0') {
                if (i + 1 >= argc) {
                    usage(argv[0]);
                    goto fail;
                }
                value = argv[++i];
            }
            if (!parse_size(value, &lookup_count)) {
                usage(argv[0]);
                goto fail;
            }
        } else if ((value = match_arg_value(arg, "--reps")) != NULL) {
            if (*value == '\0') {
                if (i + 1 >= argc) {
                    usage(argv[0]);
                    goto fail;
                }
                value = argv[++i];
            }
            if (!parse_size(value, &reps) || reps == 0) {
                usage(argv[0]);
                goto fail;
            }
        } else if ((value = match_arg_value(arg, "--seed")) != NULL) {
            if (*value == '\0') {
                if (i + 1 >= argc) {
                    usage(argv[0]);
                    goto fail;
                }
                value = argv[++i];
            }
            if (!parse_u64(value, &seed)) {
                usage(argv[0]);
                goto fail;
            }
        } else if (strcmp(arg, "--csv") == 0) {
            csv = true;
        } else if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            usage(argv[0]);
            free(modes.items);
            return 0;
        } else {
            usage(argv[0]);
            goto fail;
        }
    }

    if (modes.count == 0) {
        if (append_mode(&modes, TREE_MODE_BST) != 0) {
            goto fail;
        }
    }

    if (key_count == 0 || lookup_count == 0) {
        usage(argv[0]);
        goto fail;
    }

    int *keys = make_keys(key_count);
    if (keys == NULL) {
        goto fail;
    }
    int *queries = make_queries(keys, key_count, lookup_count, seed + 17);
    if (queries == NULL) {
        free(keys);
        goto fail;
    }

    if (csv) {
        printf("mode,key_count,lookups,rep,elapsed_ns,checksum\n");
    }

    for (size_t mi = 0; mi < modes.count; ++mi) {
        const tree_mode_t mode = modes.items[mi];
        if (mode == TREE_MODE_BST) {
            bst_tree_t tree = {0};
            if (bst_tree_init(&tree, keys, key_count, seed + mi) != 0) {
                fprintf(stderr, "failed to build bst\n");
                goto fail_modes;
            }

            for (size_t rep = 0; rep < reps; ++rep) {
                const uint64_t started = now_ns();
                const uint64_t checksum = run_bst(&tree, queries, lookup_count);
                const uint64_t elapsed = now_ns() - started;
                sink ^= checksum;

                if (csv) {
                    printf("%s,%zu,%zu,%zu,%" PRIu64 ",%" PRIu64 "\n",
                           mode_name(mode),
                           key_count,
                           lookup_count,
                           rep,
                           elapsed,
                           checksum);
                } else {
                    printf("mode=%s keys=%zu lookups=%zu rep=%zu elapsed_ns=%" PRIu64 " checksum=%" PRIu64 "\n",
                           mode_name(mode),
                           key_count,
                           lookup_count,
                           rep,
                           elapsed,
                           checksum);
                }
            }

            bst_tree_destroy(&tree);
        } else if (mode == TREE_MODE_BTREE16) {
            btree_tree_t tree = {0};
            if (btree_tree_init(&tree, keys, key_count) != 0) {
                fprintf(stderr, "failed to build btree16\n");
                goto fail_modes;
            }

            for (size_t rep = 0; rep < reps; ++rep) {
                const uint64_t started = now_ns();
                const uint64_t checksum = run_btree16(&tree, queries, lookup_count);
                const uint64_t elapsed = now_ns() - started;
                sink ^= checksum;

                if (csv) {
                    printf("%s,%zu,%zu,%zu,%" PRIu64 ",%" PRIu64 "\n",
                           mode_name(mode),
                           key_count,
                           lookup_count,
                           rep,
                           elapsed,
                           checksum);
                } else {
                    printf("mode=%s keys=%zu lookups=%zu rep=%zu elapsed_ns=%" PRIu64 " checksum=%" PRIu64 "\n",
                           mode_name(mode),
                           key_count,
                           lookup_count,
                           rep,
                           elapsed,
                           checksum);
                }
            }

            btree_tree_destroy(&tree);
        }
    }

    free(keys);
    free(queries);
    free(modes.items);
    (void)sink;
    return 0;

fail_modes:
    free(keys);
    free(queries);
fail:
    free(modes.items);
    return 1;
}
