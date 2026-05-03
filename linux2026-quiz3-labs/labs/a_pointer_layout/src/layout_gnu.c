#include "../include/pointer_layout.h"

static void demo_gnu_container_of(void) {
    struct layout_outer outer = make_outer();
    const struct layout_sample *member = &outer.sample;
    struct layout_outer *owner = CONTAINER_OF_GNU(member, struct layout_outer, sample);

    require_check(owner == &outer, "GNU statement expression container_of");
    printf("GNU statement expression + typeof\n");
    printf("  owner->lead   = %c\n", owner->lead);
    printf("  owner->tail   = %d\n", owner->tail);
}

int main(void) {
    demo_array_decay();
    demo_offsets();
    demo_gnu_container_of();
    return 0;
}
