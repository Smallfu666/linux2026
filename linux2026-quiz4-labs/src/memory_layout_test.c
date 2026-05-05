#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int global_static[1 << 20];

static void use_static_tmp(void) {
    static int tmp[1 << 20];
    tmp[0] = 42;
    printf("static_tmp_addr=%p\n", (void *) tmp);
    printf("static_tmp_value=%d\n", tmp[0]);
}

static void use_stack_tmp(void) {
    volatile int tmp[1 << 20];
    tmp[0] = 1;
    printf("stack_tmp_addr=%p\n", (const void *) tmp);
    printf("stack_tmp_value=%d\n", tmp[0]);
}

static void dump_maps(void) {
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f) {
        perror("/proc/self/maps");
        printf("UNKNOWN\n");
        return;
    }
    char buf[4096];
    while (fgets(buf, sizeof buf, f)) {
        fputs(buf, stdout);
    }
    fclose(f);
}

int main(int argc, char **argv) {
    printf("global_static_addr=%p\n", (void *) global_static);
    printf("global_static_bytes=%zu\n", sizeof global_static);
    if (argc < 2) {
        fprintf(stderr, "usage: %s static|stack|maps\n", argv[0]);
        return 2;
    }
    if (strcmp(argv[1], "static") == 0) {
        use_static_tmp();
        return 0;
    }
    if (strcmp(argv[1], "stack") == 0) {
        use_stack_tmp();
        return 0;
    }
    if (strcmp(argv[1], "maps") == 0) {
        dump_maps();
        return 0;
    }
    fprintf(stderr, "unknown mode: %s\n", argv[1]);
    return 2;
}
