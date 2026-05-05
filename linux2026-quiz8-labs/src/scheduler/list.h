#ifndef SCHEDULER_LIST_H
#define SCHEDULER_LIST_H

#include <stddef.h>

typedef struct list_node {
    struct list_node *prev;
    struct list_node *next;
    int value;
} list_node_t;

typedef struct {
    list_node_t *head;
    list_node_t *tail;
} list_t;

static inline void list_init(list_t *list) {
    list->head = NULL;
    list->tail = NULL;
}

static inline void list_add_front(list_t *list, list_node_t *node) {
    node->prev = NULL;
    node->next = list->head;
    if (list->head) {
        list->head->prev = node;
    } else {
        list->tail = node;
    }
    list->head = node;
}

static inline void list_add_back(list_t *list, list_node_t *node) {
    node->next = NULL;
    node->prev = list->tail;
    if (list->tail) {
        list->tail->next = node;
    } else {
        list->head = node;
    }
    list->tail = node;
}

static inline void list_remove(list_t *list, list_node_t *node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }
    node->prev = NULL;
    node->next = NULL;
}

static inline void list_remove_unchecked(list_t *list, list_node_t *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    if (list->head == node) {
        list->head = node->next;
    }
    if (list->tail == node) {
        list->tail = node->prev;
    }
    node->prev = NULL;
    node->next = NULL;
}

#endif
