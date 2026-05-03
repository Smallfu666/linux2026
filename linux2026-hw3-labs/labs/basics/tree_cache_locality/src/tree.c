#include "tree.h"

#include <stdlib.h>
#include <string.h>

#define BTREE_FANOUT 16
#define BTREE_LEAF_CAP 32

struct bst_node {
    int key;
    struct bst_node *left;
    struct bst_node *right;
};

struct btree_node {
    bool leaf;
    uint8_t key_count;
    uint8_t child_count;
    int keys[BTREE_FANOUT - 1];
    struct btree_node *children[BTREE_FANOUT];
    size_t start;
    size_t count;
};

static void destroy_btree_rec(btree_node_t *node);

static uint64_t next_rng(uint64_t *state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

const char *mode_name(tree_mode_t mode) {
    switch (mode) {
    case TREE_MODE_BST:
        return "bst";
    case TREE_MODE_BTREE16:
        return "btree16";
    }
    return "unknown";
}

tree_mode_t parse_mode(const char *name, int *ok) {
    if (strcmp(name, "bst") == 0) {
        *ok = 1;
        return TREE_MODE_BST;
    }
    if (strcmp(name, "btree16") == 0) {
        *ok = 1;
        return TREE_MODE_BTREE16;
    }
    *ok = 0;
    return TREE_MODE_BST;
}

static bst_node_t *build_bst_from_sorted_nodes(bst_node_t **nodes, size_t lo, size_t hi) {
    if (lo >= hi) {
        return NULL;
    }
    size_t mid = lo + (hi - lo) / 2;
    bst_node_t *node = nodes[mid];
    node->left = build_bst_from_sorted_nodes(nodes, lo, mid);
    node->right = build_bst_from_sorted_nodes(nodes, mid + 1, hi);
    return node;
}

int bst_tree_init(bst_tree_t *tree, const int *keys, size_t key_count, uint64_t seed) {
    memset(tree, 0, sizeof(*tree));
    if (key_count == 0) {
        return -1;
    }

    tree->nodes = calloc(key_count, sizeof(*tree->nodes));
    if (tree->nodes == NULL) {
        return -1;
    }
    tree->node_count = key_count;

    size_t *perm = malloc(key_count * sizeof(*perm));
    if (perm == NULL) {
        bst_tree_destroy(tree);
        return -1;
    }

    for (size_t i = 0; i < key_count; ++i) {
        perm[i] = i;
    }

    uint64_t rng = seed ? seed : 0x9e3779b97f4a7c15ULL;
    for (size_t i = key_count; i > 1; --i) {
        size_t j = (size_t)(next_rng(&rng) % i);
        size_t tmp = perm[i - 1];
        perm[i - 1] = perm[j];
        perm[j] = tmp;
    }

    for (size_t i = 0; i < key_count; ++i) {
        size_t idx = perm[i];
        size_t padding = 16u + (size_t)(next_rng(&rng) & 7u) * 32u;
        bst_node_t *node = malloc(sizeof(*node) + padding);
        if (node == NULL) {
            free(perm);
            bst_tree_destroy(tree);
            return -1;
        }
        node->key = keys[idx];
        node->left = NULL;
        node->right = NULL;
        tree->nodes[idx] = node;
    }

    free(perm);
    tree->root = build_bst_from_sorted_nodes(tree->nodes, 0, key_count);
    return 0;
}

void bst_tree_destroy(bst_tree_t *tree) {
    if (tree->nodes != NULL) {
        for (size_t i = 0; i < tree->node_count; ++i) {
            free(tree->nodes[i]);
        }
    }
    free(tree->nodes);
    tree->nodes = NULL;
    tree->root = NULL;
    tree->node_count = 0;
}

int bst_tree_lookup(const bst_tree_t *tree, int key) {
    const bst_node_t *node = tree->root;
    while (node != NULL) {
        if (key < node->key) {
            node = node->left;
        } else if (key > node->key) {
            node = node->right;
        } else {
            return node->key;
        }
    }
    return -1;
}

static btree_node_t *build_btree_rec(const int *keys, size_t lo, size_t hi) {
    const size_t count = hi - lo;
    btree_node_t *node = calloc(1, sizeof(*node));
    if (node == NULL) {
        return NULL;
    }

    if (count <= BTREE_LEAF_CAP) {
        node->leaf = true;
        node->start = lo;
        node->count = count;
        node->key_count = (uint8_t)count;
        return node;
    }

    size_t child_count = count < BTREE_FANOUT ? count : BTREE_FANOUT;
    if (child_count < 2) {
        child_count = 2;
    }

    const size_t base = count / child_count;
    const size_t rem = count % child_count;
    size_t cursor = lo;

    node->leaf = false;
    node->child_count = (uint8_t)child_count;
    node->key_count = (uint8_t)(child_count - 1);

    for (size_t i = 0; i < child_count; ++i) {
        size_t part = base + (i < rem ? 1 : 0);
        size_t child_lo = cursor;
        size_t child_hi = cursor + part;
        node->children[i] = build_btree_rec(keys, child_lo, child_hi);
        if (node->children[i] == NULL) {
            for (size_t j = 0; j < i; ++j) {
                destroy_btree_rec(node->children[j]);
            }
            free(node);
            return NULL;
        }
        if (i < child_count - 1) {
            node->keys[i] = keys[child_hi - 1];
        }
        cursor = child_hi;
    }

    return node;
}

static void destroy_btree_rec(btree_node_t *node) {
    if (node == NULL) {
        return;
    }
    for (size_t i = 0; i < node->child_count; ++i) {
        destroy_btree_rec(node->children[i]);
    }
    free(node);
}

int btree_tree_init(btree_tree_t *tree, const int *keys, size_t key_count) {
    memset(tree, 0, sizeof(*tree));
    if (key_count == 0) {
        return -1;
    }

    tree->keys = malloc(key_count * sizeof(*tree->keys));
    if (tree->keys == NULL) {
        return -1;
    }
    memcpy(tree->keys, keys, key_count * sizeof(*tree->keys));
    tree->key_count = key_count;

    tree->root = build_btree_rec(tree->keys, 0, key_count);
    if (tree->root == NULL) {
        btree_tree_destroy(tree);
        return -1;
    }

    return 0;
}

void btree_tree_destroy(btree_tree_t *tree) {
    destroy_btree_rec(tree->root);
    free(tree->keys);
    tree->root = NULL;
    tree->keys = NULL;
    tree->key_count = 0;
}

static int btree_leaf_lookup(const btree_tree_t *tree, const btree_node_t *node, int key) {
    for (size_t i = 0; i < node->count; ++i) {
        if (tree->keys[node->start + i] == key) {
            return key;
        }
    }
    return -1;
}

int btree_tree_lookup(const btree_tree_t *tree, int key) {
    const btree_node_t *node = tree->root;
    while (node != NULL) {
        if (node->leaf) {
            return btree_leaf_lookup(tree, node, key);
        }

        size_t i = 0;
        while (i < node->key_count && key > node->keys[i]) {
            ++i;
        }
        node = node->children[i];
    }

    return -1;
}
