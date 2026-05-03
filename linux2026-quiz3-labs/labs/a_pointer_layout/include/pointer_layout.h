#ifndef POINTER_LAYOUT_H
#define POINTER_LAYOUT_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

struct layout_sample {
    int id;
    long weight;
    char tag[8];
};

struct layout_outer {
    char lead;
    struct layout_sample sample;
    int tail;
};

static inline void require_check(int cond, const char *label) {
    if (!cond) {
        fprintf(stderr, "check failed: %s\n", label);
        exit(1);
    }
}

static inline struct layout_outer make_outer(void) {
    return (struct layout_outer){
        .lead = 'L',
        .sample = {.id = 42, .weight = 99L, .tag = "linux"},
        .tail = 7,
    };
}

static inline void demo_array_decay(void) {
    int a[5] = {10, 20, 30, 40, 50};
    int *p = a;
    int (*pa)[5] = &a;

    printf("array decay\n");
    printf("  sizeof(a)  = %zu\n", sizeof(a));
    printf("  sizeof(p)  = %zu\n", sizeof(p));
    printf("  sizeof(*p) = %zu\n", sizeof(*pa));
    printf("  p[0..4]    = %d %d %d %d %d\n", p[0], p[1], p[2], p[3], p[4]);
    printf("  &a + 1     = %p\n", (void *)(&a + 1));
    printf("  p + 5      = %p\n", (void *)(p + 5));
    printf("  &a + 1 == p + 5: %s\n",
           ((void *)(&a + 1) == (void *)(p + 5)) ? "yes" : "no");
    printf("  ptrdiff_t elements from p to p+5 = %td\n", (ptrdiff_t)((p + 5) - p));
    printf("  char* byte delta from p to p+5   = %td\n",
           (ptrdiff_t)((char *)(p + 5) - (char *)p));
    printf("  char* byte delta from &a to &a+1 = %td\n",
           (ptrdiff_t)((char *)(&a + 1) - (char *)&a));
}

static inline void demo_offsets(void) {
    struct layout_outer outer = make_outer();

    printf("layout and offsets\n");
    printf("  offsetof(outer, lead)   = %zu\n", offsetof(struct layout_outer, lead));
    printf("  offsetof(outer, sample) = %zu\n", offsetof(struct layout_outer, sample));
    printf("  offsetof(outer, tail)   = %zu\n", offsetof(struct layout_outer, tail));
    printf("  offsetof(sample, id)     = %zu\n", offsetof(struct layout_sample, id));
    printf("  offsetof(sample, weight) = %zu\n", offsetof(struct layout_sample, weight));
    printf("  offsetof(sample, tag)    = %zu\n", offsetof(struct layout_sample, tag));
    printf("  &outer               = %p\n", (void *)&outer);
    printf("  &outer.sample        = %p\n", (void *)&outer.sample);
    printf("  &outer.tail          = %p\n", (void *)&outer.tail);
    printf("  sample - outer       = %td\n",
           (ptrdiff_t)((char *)&outer.sample - (char *)&outer));
    printf("  tail - sample        = %td\n",
           (ptrdiff_t)((char *)&outer.tail - (char *)&outer.sample));
}

#define CONTAINER_OF_CHAR(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define CONTAINER_OF_GNU(ptr, type, member) \
    __extension__ ({ \
        const __typeof__(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member)); \
    })

#define POINTER_MATCHES_MEMBER_TYPE(ptr, member_type) \
    _Generic((ptr), \
        member_type *: 1, \
        const member_type *: 1, \
        volatile member_type *: 1, \
        const volatile member_type *: 1, \
        default: 0 \
    )

#define CONTAINER_OF_CHECKED(ptr, type, member, member_type) \
    ((void)sizeof(struct { \
        _Static_assert(POINTER_MATCHES_MEMBER_TYPE((ptr), member_type), \
                       "container_of_checked: pointer type does not match member type"); \
        int _; \
    }), \
    (type *)((char *)(ptr) - offsetof(type, member)))

#define CONTAINER_OF_GENERIC(ptr, type, member) \
    _Generic((ptr), \
        const __typeof__(((type *)0)->member) *: \
            (const type *)((const char *)(ptr) - offsetof(type, member)), \
        volatile __typeof__(((type *)0)->member) *: \
            (volatile type *)((char *)(ptr) - offsetof(type, member)), \
        const volatile __typeof__(((type *)0)->member) *: \
            (const volatile type *)((const char *)(ptr) - offsetof(type, member)), \
        default: \
            (type *)((char *)(ptr) - offsetof(type, member)) \
    )

static inline void demo_plain_container_of(void) {
    struct layout_outer outer = make_outer();
    struct layout_sample *member = &outer.sample;
    struct layout_outer *owner = CONTAINER_OF_CHAR(member, struct layout_outer, sample);

    require_check(owner == &outer, "plain char* container_of");
    printf("plain char* subtraction\n");
    printf("  owner->lead   = %c\n", owner->lead);
    printf("  owner->tail   = %d\n", owner->tail);
}

#endif
