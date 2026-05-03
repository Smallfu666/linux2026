#include "list.h"

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void set_error(char *errbuf, size_t errbuf_len, const char *fmt, ...)
{
    va_list args;

    if (errbuf == NULL || errbuf_len == 0) {
        return;
    }

    va_start(args, fmt);
    vsnprintf(errbuf, errbuf_len, fmt, args);
    va_end(args);
}

static bool checked_mul_size(size_t a, size_t b, size_t *out)
{
    if (a == 0 || b == 0) {
        *out = 0;
        return true;
    }

    if (a > SIZE_MAX / b) {
        return false;
    }

    *out = a * b;
    return true;
}

static uint64_t splitmix64_next(uint64_t *state)
{
    uint64_t z;

    *state += 0x9e3779b97f4a7c15ULL;
    z = *state;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static void init_payloads(struct node **nodes, size_t n, uint64_t seed)
{
    uint64_t state = seed ? seed : 0x123456789abcdef0ULL;

    for (size_t i = 0; i < n; ++i) {
        nodes[i]->payload = splitmix64_next(&state) ^ (uint64_t)i;
        nodes[i]->next = NULL;
    }
}

static void link_sequential(struct node **nodes, size_t n)
{
    for (size_t i = 0; i + 1 < n; ++i) {
        nodes[i]->next = nodes[i + 1];
    }

    if (n > 0) {
        nodes[n - 1]->next = NULL;
    }
}

static void shuffle_indices(size_t *indices, size_t n, uint64_t seed)
{
    uint64_t state = seed ? seed : 0xfeedfacecafebeefULL;

    for (size_t i = 0; i < n; ++i) {
        indices[i] = i;
    }

    for (size_t i = n; i > 1; --i) {
        size_t j = (size_t)(splitmix64_next(&state) % i);
        size_t tmp = indices[i - 1];
        indices[i - 1] = indices[j];
        indices[j] = tmp;
    }
}

static int build_contiguous(struct list_handle *list,
                            size_t n,
                            uint64_t seed,
                            char *errbuf,
                            size_t errbuf_len)
{
    struct node *nodes;
    struct node **views;

    nodes = calloc(n, sizeof(*nodes));
    views = calloc(n, sizeof(*views));
    if (nodes == NULL || views == NULL) {
        free(nodes);
        free(views);
        set_error(errbuf, errbuf_len, "contiguous allocation failed for %zu nodes", n);
        return -1;
    }

    for (size_t i = 0; i < n; ++i) {
        views[i] = &nodes[i];
    }

    init_payloads(views, n, seed);
    link_sequential(views, n);

    list->head = &nodes[0];
    list->storage = nodes;
    list->indirect = views;
    list->storage_len = n * sizeof(*nodes);
    return 0;
}

static int build_malloc_nodes(struct list_handle *list,
                              size_t n,
                              uint64_t seed,
                              bool shuffle_links,
                              char *errbuf,
                              size_t errbuf_len)
{
    struct node **nodes;
    size_t *indices = NULL;

    nodes = calloc(n, sizeof(*nodes));
    if (nodes == NULL) {
        set_error(errbuf, errbuf_len, "malloc node table failed for %zu nodes", n);
        return -1;
    }

    for (size_t i = 0; i < n; ++i) {
        nodes[i] = malloc(sizeof(*nodes[i]));
        if (nodes[i] == NULL) {
            for (size_t j = 0; j < i; ++j) {
                free(nodes[j]);
            }
            free(nodes);
            set_error(errbuf, errbuf_len, "malloc failed at node %zu of %zu", i, n);
            return -1;
        }
    }

    init_payloads(nodes, n, seed);

    if (!shuffle_links) {
        link_sequential(nodes, n);
        list->head = nodes[0];
    } else {
        indices = calloc(n, sizeof(*indices));
        if (indices == NULL) {
            for (size_t i = 0; i < n; ++i) {
                free(nodes[i]);
            }
            free(nodes);
            set_error(errbuf, errbuf_len, "shuffle index allocation failed for %zu nodes", n);
            return -1;
        }

        shuffle_indices(indices, n, seed ^ 0xa5a5a5a5a5a5a5a5ULL);
        for (size_t i = 0; i + 1 < n; ++i) {
            nodes[indices[i]]->next = nodes[indices[i + 1]];
        }
        nodes[indices[n - 1]]->next = NULL;
        list->head = nodes[indices[0]];
    }

    free(indices);
    list->storage = nodes;
    list->indirect = nodes;
    list->storage_len = n * sizeof(*nodes);
    return 0;
}

static int build_page_spread(struct list_handle *list,
                             size_t n,
                             uint64_t seed,
                             size_t spread_bytes,
                             char *errbuf,
                             size_t errbuf_len)
{
    long page_size_long;
    size_t region_len;
    struct node **views;
    int rc;
    char *region;

    page_size_long = sysconf(_SC_PAGESIZE);
    if (page_size_long <= 0) {
        set_error(errbuf, errbuf_len, "failed to query page size");
        return -1;
    }

    if (spread_bytes == 0) {
        spread_bytes = (size_t)page_size_long;
    }

    if (spread_bytes < sizeof(struct node)) {
        spread_bytes = sizeof(struct node);
    }

    if (!checked_mul_size(n, spread_bytes, &region_len)) {
        set_error(errbuf, errbuf_len, "page-spread region overflow for n=%zu stride=%zu", n, spread_bytes);
        return -1;
    }

    rc = posix_memalign((void **)&region, (size_t)page_size_long, region_len);
    if (rc != 0) {
        set_error(errbuf, errbuf_len,
                  "page-spread allocation failed for %" PRIu64 " bytes: %s",
                  (uint64_t)region_len,
                  strerror(rc));
        return -1;
    }

    memset(region, 0, region_len);

    views = calloc(n, sizeof(*views));
    if (views == NULL) {
        free(region);
        set_error(errbuf, errbuf_len, "page-spread node view allocation failed for %zu nodes", n);
        return -1;
    }

    for (size_t i = 0; i < n; ++i) {
        views[i] = (struct node *)(region + (i * spread_bytes));
    }

    init_payloads(views, n, seed);
    link_sequential(views, n);

    list->head = views[0];
    list->storage = region;
    list->indirect = views;
    list->storage_len = region_len;
    list->spread_bytes = spread_bytes;
    return 0;
}

const char *alloc_mode_name(enum alloc_mode mode)
{
    switch (mode) {
    case ALLOC_CONTIGUOUS:
        return "contiguous";
    case ALLOC_MALLOC:
        return "malloc";
    case ALLOC_SHUFFLE:
        return "shuffle";
    case ALLOC_PAGE_SPREAD:
        return "page_spread";
    default:
        return "unknown";
    }
}

int parse_alloc_mode(const char *text, enum alloc_mode *mode_out)
{
    if (text == NULL || mode_out == NULL) {
        return -1;
    }

    if (strcmp(text, "contiguous") == 0) {
        *mode_out = ALLOC_CONTIGUOUS;
        return 0;
    }

    if (strcmp(text, "malloc") == 0 || strcmp(text, "malloc_per_node") == 0) {
        *mode_out = ALLOC_MALLOC;
        return 0;
    }

    if (strcmp(text, "shuffle") == 0 || strcmp(text, "malloc_shuffle") == 0) {
        *mode_out = ALLOC_SHUFFLE;
        return 0;
    }

    if (strcmp(text, "page_spread") == 0 || strcmp(text, "spread") == 0) {
        *mode_out = ALLOC_PAGE_SPREAD;
        return 0;
    }

    return -1;
}

int list_build(struct list_handle *list,
               size_t n,
               enum alloc_mode mode,
               uint64_t seed,
               size_t spread_bytes,
               char *errbuf,
               size_t errbuf_len)
{
    memset(list, 0, sizeof(*list));

    if (n == 0) {
        set_error(errbuf, errbuf_len, "list length must be > 0");
        return -1;
    }

    list->n = n;
    list->mode = mode;
    list->spread_bytes = spread_bytes;

    switch (mode) {
    case ALLOC_CONTIGUOUS:
        return build_contiguous(list, n, seed, errbuf, errbuf_len);
    case ALLOC_MALLOC:
        return build_malloc_nodes(list, n, seed, false, errbuf, errbuf_len);
    case ALLOC_SHUFFLE:
        return build_malloc_nodes(list, n, seed, true, errbuf, errbuf_len);
    case ALLOC_PAGE_SPREAD:
        return build_page_spread(list, n, seed, spread_bytes, errbuf, errbuf_len);
    default:
        set_error(errbuf, errbuf_len, "unsupported allocation mode");
        return -1;
    }
}

void list_destroy(struct list_handle *list)
{
    if (list == NULL) {
        return;
    }

    if ((list->mode == ALLOC_MALLOC || list->mode == ALLOC_SHUFFLE) && list->indirect != NULL) {
        for (size_t i = 0; i < list->n; ++i) {
            free(list->indirect[i]);
        }
        free(list->indirect);
    } else if (list->mode == ALLOC_CONTIGUOUS) {
        free(list->storage);
        free(list->indirect);
    } else if (list->mode == ALLOC_PAGE_SPREAD) {
        free(list->storage);
        free(list->indirect);
    }

    memset(list, 0, sizeof(*list));
}
