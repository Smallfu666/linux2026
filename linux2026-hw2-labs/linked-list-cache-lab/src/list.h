#ifndef LINKED_LIST_CACHE_LAB_LIST_H
#define LINKED_LIST_CACHE_LAB_LIST_H

#include <stddef.h>
#include <stdint.h>

struct node {
    struct node *next;
    uint64_t payload;
};

enum alloc_mode {
    ALLOC_CONTIGUOUS = 0,
    ALLOC_MALLOC,
    ALLOC_SHUFFLE,
    ALLOC_PAGE_SPREAD,
};

struct list_handle {
    struct node *head;
    size_t n;
    enum alloc_mode mode;
    size_t spread_bytes;
    void *storage;
    struct node **indirect;
    size_t storage_len;
};

const char *alloc_mode_name(enum alloc_mode mode);
int parse_alloc_mode(const char *text, enum alloc_mode *mode_out);

int list_build(struct list_handle *list,
               size_t n,
               enum alloc_mode mode,
               uint64_t seed,
               size_t spread_bytes,
               char *errbuf,
               size_t errbuf_len);

void list_destroy(struct list_handle *list);

const struct node *middle_fast_slow(const struct node *head);
const struct node *middle_two_pass(const struct node *head);

#endif
