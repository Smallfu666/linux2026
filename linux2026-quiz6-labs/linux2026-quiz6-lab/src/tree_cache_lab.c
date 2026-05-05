#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum {
    LOOKUPS = 1000000,
    BLOCK_SIZE = 32,
};

struct node {
    uint64_t key;
    uint64_t value;
    struct node *left;
    struct node *right;
};

struct block_index {
    size_t nblocks;
    uint64_t *max_keys;
    uint64_t *keys;
    uint64_t *values;
};

static volatile uint64_t sink;

static uint64_t xorshift64(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

static int cmp_u64(const void *a, const void *b)
{
    uint64_t x = *(const uint64_t *) a;
    uint64_t y = *(const uint64_t *) b;

    return (x > y) - (x < y);
}

static double now_ns(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (double) ts.tv_sec * 1000000000.0 + (double) ts.tv_nsec;
}

static struct node *node_insert(struct node *root, uint64_t key, uint64_t value)
{
    if (root == NULL) {
        struct node *n = malloc(sizeof(*n));
        if (n == NULL) {
            perror("malloc node");
            exit(1);
        }
        n->key = key;
        n->value = value;
        n->left = NULL;
        n->right = NULL;
        return n;
    }

    if (key < root->key) {
        root->left = node_insert(root->left, key, value);
    } else if (key > root->key) {
        root->right = node_insert(root->right, key, value);
    } else {
        root->value = value;
    }
    return root;
}

static uint64_t node_lookup(const struct node *root, uint64_t key)
{
    while (root != NULL) {
        if (key < root->key) {
            root = root->left;
        } else if (key > root->key) {
            root = root->right;
        } else {
            return root->value;
        }
    }
    return 0;
}

static void node_free(struct node *root)
{
    if (root == NULL) {
        return;
    }
    node_free(root->left);
    node_free(root->right);
    free(root);
}

static uint64_t array_lookup(const uint64_t *keys, const uint64_t *values, size_t n, uint64_t key)
{
    size_t lo = 0;
    size_t hi = n;

    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (keys[mid] < key) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    if (lo < n && keys[lo] == key) {
        return values[lo];
    }
    return 0;
}

static void block_build(struct block_index *idx, const uint64_t *keys, const uint64_t *values, size_t n)
{
    idx->nblocks = (n + BLOCK_SIZE - 1) / BLOCK_SIZE;
    idx->max_keys = malloc(idx->nblocks * sizeof(idx->max_keys[0]));
    idx->keys = malloc(n * sizeof(idx->keys[0]));
    idx->values = malloc(n * sizeof(idx->values[0]));
    if (idx->max_keys == NULL || idx->keys == NULL || idx->values == NULL) {
        perror("malloc block index");
        exit(1);
    }

    memcpy(idx->keys, keys, n * sizeof(idx->keys[0]));
    memcpy(idx->values, values, n * sizeof(idx->values[0]));
    for (size_t b = 0; b < idx->nblocks; ++b) {
        size_t end = (b + 1) * BLOCK_SIZE;
        if (end > n) {
            end = n;
        }
        idx->max_keys[b] = keys[end - 1];
    }
}

static uint64_t block_lookup(const struct block_index *idx, size_t n, uint64_t key)
{
    size_t lo = 0;
    size_t hi = idx->nblocks;

    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (idx->max_keys[mid] < key) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    if (lo == idx->nblocks) {
        return 0;
    }

    size_t start = lo * BLOCK_SIZE;
    size_t end = start + BLOCK_SIZE;
    if (end > n) {
        end = n;
    }
    for (size_t i = start; i < end; ++i) {
        if (idx->keys[i] == key) {
            return idx->values[i];
        }
    }
    return 0;
}

static void block_free(struct block_index *idx)
{
    free(idx->max_keys);
    free(idx->keys);
    free(idx->values);
}

static void fill_data(uint64_t *keys, uint64_t *values, uint64_t *order, uint64_t *queries, size_t n)
{
    uint64_t rng = UINT64_C(0x123456789abcdef0);

    for (size_t i = 0; i < n; ++i) {
        keys[i] = xorshift64(&rng) | UINT64_C(1);
    }
    qsort(keys, n, sizeof(keys[0]), cmp_u64);
    for (size_t i = 0; i < n; ++i) {
        values[i] = keys[i] ^ UINT64_C(0x9e3779b97f4a7c15);
        order[i] = (uint64_t) i;
    }
    for (size_t i = n - 1; i > 0; --i) {
        size_t j = (size_t) (xorshift64(&rng) % (i + 1));
        uint64_t tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
    }
    for (size_t i = 0; i < LOOKUPS; ++i) {
        queries[i] = keys[xorshift64(&rng) % n];
    }
}

static void run_size(size_t n)
{
    uint64_t *keys = malloc(n * sizeof(keys[0]));
    uint64_t *values = malloc(n * sizeof(values[0]));
    uint64_t *order = malloc(n * sizeof(order[0]));
    uint64_t *queries = malloc((size_t) LOOKUPS * sizeof(queries[0]));
    struct node *root = NULL;
    struct block_index block = {0, NULL, NULL, NULL};
    double t0;
    double build_bst;
    double build_array;
    double build_block;
    double lookup_bst;
    double lookup_array;
    double lookup_block;
    uint64_t sum = 0;

    if (keys == NULL || values == NULL || order == NULL || queries == NULL) {
        perror("malloc data");
        exit(1);
    }
    fill_data(keys, values, order, queries, n);

    t0 = now_ns();
    for (size_t i = 0; i < n; ++i) {
        size_t pos = (size_t) order[i];
        root = node_insert(root, keys[pos], values[pos]);
    }
    build_bst = now_ns() - t0;

    t0 = now_ns();
    uint64_t *array_keys = malloc(n * sizeof(array_keys[0]));
    uint64_t *array_values = malloc(n * sizeof(array_values[0]));
    if (array_keys == NULL || array_values == NULL) {
        perror("malloc arrays");
        exit(1);
    }
    memcpy(array_keys, keys, n * sizeof(array_keys[0]));
    memcpy(array_values, values, n * sizeof(array_values[0]));
    build_array = now_ns() - t0;

    t0 = now_ns();
    block_build(&block, keys, values, n);
    build_block = now_ns() - t0;

    t0 = now_ns();
    for (size_t i = 0; i < LOOKUPS; ++i) {
        sum += node_lookup(root, queries[i]);
    }
    lookup_bst = now_ns() - t0;

    t0 = now_ns();
    for (size_t i = 0; i < LOOKUPS; ++i) {
        sum += array_lookup(array_keys, array_values, n, queries[i]);
    }
    lookup_array = now_ns() - t0;

    t0 = now_ns();
    for (size_t i = 0; i < LOOKUPS; ++i) {
        sum += block_lookup(&block, n, queries[i]);
    }
    lookup_block = now_ns() - t0;
    sink = sum;

    printf("N=%zu lookups=%d\n", n, LOOKUPS);
    printf("  %-12s build_ms=%8.3f lookup_ms=%9.3f ns_lookup=%8.2f approx_mem=%zu\n",
           "malloc_bst", build_bst / 1000000.0, lookup_bst / 1000000.0,
           lookup_bst / (double) LOOKUPS, n * sizeof(struct node));
    printf("  %-12s build_ms=%8.3f lookup_ms=%9.3f ns_lookup=%8.2f approx_mem=%zu\n",
           "array", build_array / 1000000.0, lookup_array / 1000000.0,
           lookup_array / (double) LOOKUPS, n * sizeof(uint64_t) * 2);
    printf("  %-12s build_ms=%8.3f lookup_ms=%9.3f ns_lookup=%8.2f approx_mem=%zu\n",
           "block32", build_block / 1000000.0, lookup_block / 1000000.0,
           lookup_block / (double) LOOKUPS,
           n * sizeof(uint64_t) * 2 + block.nblocks * sizeof(uint64_t));

    node_free(root);
    free(array_keys);
    free(array_values);
    block_free(&block);
    free(keys);
    free(values);
    free(order);
    free(queries);
}

int main(void)
{
    static const size_t sizes[] = {1024, 16384, 262144, 1048576};

    puts("Experiment E: pointer tree vs contiguous lookup microbenchmark");
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
        run_size(sizes[i]);
    }
    printf("sink=%" PRIu64 "\n", sink);
    puts("perf note: run `make perf` for hardware counters if perf is installed and permitted.");
    return 0;
}
