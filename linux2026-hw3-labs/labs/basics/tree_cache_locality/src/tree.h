#ifndef TREE_CACHE_LOCALITY_TREE_H
#define TREE_CACHE_LOCALITY_TREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    TREE_MODE_BST = 0,
    TREE_MODE_BTREE16
} tree_mode_t;

typedef struct bst_node bst_node_t;
typedef struct btree_node btree_node_t;

typedef struct {
    bst_node_t *root;
    bst_node_t **nodes;
    size_t node_count;
} bst_tree_t;

typedef struct {
    btree_node_t *root;
    int *keys;
    size_t key_count;
} btree_tree_t;

typedef struct {
    tree_mode_t *items;
    size_t count;
} mode_list_t;

const char *mode_name(tree_mode_t mode);
tree_mode_t parse_mode(const char *name, int *ok);

int bst_tree_init(bst_tree_t *tree, const int *keys, size_t key_count, uint64_t seed);
void bst_tree_destroy(bst_tree_t *tree);
int bst_tree_lookup(const bst_tree_t *tree, int key);

int btree_tree_init(btree_tree_t *tree, const int *keys, size_t key_count);
void btree_tree_destroy(btree_tree_t *tree);
int btree_tree_lookup(const btree_tree_t *tree, int key);

#endif
