#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    NODE_RED = 0u,
    NODE_BLACK = 1u,
};

struct tagged_node {
    uintptr_t parent_color;
    struct tagged_node *left;
    struct tagged_node *right;
    int key;
};

struct packed_wrapper {
    char prefix;
    struct tagged_node node;
} __attribute__((packed));

static uintptr_t pack_parent_color(const void *parent, unsigned color)
{
    return ((uintptr_t)parent) | (uintptr_t)(color & 1u);
}

static const void *unpack_parent(uintptr_t tagged)
{
    return (const void *)(tagged & ~(uintptr_t)1u);
}

static unsigned unpack_color(uintptr_t tagged)
{
    return (unsigned)(tagged & 1u);
}

int main(void)
{
    struct tagged_node *parent = malloc(sizeof(*parent));
    struct tagged_node *child = malloc(sizeof(*child));
    struct packed_wrapper *packed_parent_box = malloc(sizeof(*packed_parent_box));
    struct packed_wrapper *packed_child_box = malloc(sizeof(*packed_child_box));
    uintptr_t packed_parent_addr;
    uintptr_t packed_child_addr;
    uintptr_t normal_tagged;
    uintptr_t packed_tagged;

    if (parent == NULL || child == NULL ||
        packed_parent_box == NULL || packed_child_box == NULL) {
        fputs("allocation failed\n", stderr);
        free(parent);
        free(child);
        free(packed_parent_box);
        free(packed_child_box);
        return 1;
    }

    packed_parent_addr = (uintptr_t)((char *)packed_parent_box +
                                     offsetof(struct packed_wrapper, node));
    packed_child_addr = (uintptr_t)((char *)packed_child_box +
                                    offsetof(struct packed_wrapper, node));

    normal_tagged = pack_parent_color(parent, NODE_BLACK);
    packed_tagged = pack_parent_color((const void *)packed_parent_addr, NODE_BLACK);

    printf("aligned node sizeof=%zu alignof=%zu\n",
           sizeof(struct tagged_node), _Alignof(struct tagged_node));
    printf("packed wrapper sizeof=%zu alignof=%zu offset(node)=%zu\n",
           sizeof(struct packed_wrapper), _Alignof(struct packed_wrapper),
           offsetof(struct packed_wrapper, node));
    puts("");
    printf("normal parent @ %p low_bits=%#" PRIxPTR "\n",
           (void *)parent, ((uintptr_t)parent & 0x7u));
    printf("normal child  @ %p low_bits=%#" PRIxPTR "\n",
           (void *)child, ((uintptr_t)child & 0x7u));
    printf("normal tagged value=%#" PRIxPTR " recovered_parent=%p color=%u\n",
           normal_tagged, unpack_parent(normal_tagged), unpack_color(normal_tagged));
    puts("");
    printf("packed parent pseudo-node @ %#" PRIxPTR " low_bits=%#" PRIxPTR "\n",
           packed_parent_addr, packed_parent_addr & 0x7u);
    printf("packed child pseudo-node  @ %#" PRIxPTR " low_bits=%#" PRIxPTR "\n",
           packed_child_addr, packed_child_addr & 0x7u);
    printf("packed tagged value=%#" PRIxPTR " recovered_parent=%p color=%u\n",
           packed_tagged, unpack_parent(packed_tagged), unpack_color(packed_tagged));
    printf("packed_recovery_ok=%s\n",
           ((uintptr_t)unpack_parent(packed_tagged) == packed_parent_addr) ? "yes" : "no");

    free(parent);
    free(child);
    free(packed_parent_box);
    free(packed_child_box);
    return 0;
}
