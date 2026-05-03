#include "list.h"

#include <stddef.h>

const struct node *middle_two_pass(const struct node *head)
{
    const struct node *cursor = head;
    const struct node *middle = head;
    size_t count = 0;

    while (cursor != NULL) {
        ++count;
        cursor = cursor->next;
    }

    for (size_t i = 0; i < count / 2; ++i) {
        middle = middle->next;
    }

    return middle;
}
