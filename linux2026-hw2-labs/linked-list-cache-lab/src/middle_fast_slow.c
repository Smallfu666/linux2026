#include "list.h"

const struct node *middle_fast_slow(const struct node *head)
{
    const struct node *slow = head;
    const struct node *fast = head;

    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
    }

    return slow;
}
