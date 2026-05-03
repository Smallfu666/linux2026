#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "layout_c11.c requires C11 for _Static_assert and _Generic"
#endif

#include "../include/pointer_layout.h"

static void demo_checked_container_of(void) {
    struct layout_outer outer = make_outer();
    struct layout_sample *member = &outer.sample;
    struct layout_outer *owner =
        CONTAINER_OF_CHECKED(member, struct layout_outer, sample, struct layout_sample);

    require_check(owner == &outer, "_Static_assert checked container_of");
    printf("_Static_assert checked container_of\n");
    printf("  owner->lead   = %c\n", owner->lead);
    printf("  owner->tail   = %d\n", owner->tail);
}

static void demo_generic_container_of(void) {
    struct layout_outer outer = make_outer();
    const struct layout_sample *member = &outer.sample;
    const struct layout_outer *owner = CONTAINER_OF_GENERIC(member, struct layout_outer, sample);

    require_check(owner == &outer, "_Generic const-preserving container_of");
    printf("_Generic const-preserving container_of\n");
    printf("  owner->lead   = %c\n", owner->lead);
    printf("  owner->tail   = %d\n", owner->tail);
}

int main(void) {
    demo_array_decay();
    demo_offsets();
    demo_checked_container_of();
    demo_generic_container_of();
    return 0;
}
