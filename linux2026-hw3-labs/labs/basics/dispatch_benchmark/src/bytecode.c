#include "bytecode.h"

#include <stdlib.h>
#include <string.h>

static uint64_t next_rng(uint64_t *state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

const char *pattern_name(pattern_t pattern) {
    switch (pattern) {
    case PATTERN_PREDICTABLE:
        return "predictable";
    case PATTERN_RANDOM:
        return "random";
    }
    return "unknown";
}

pattern_t parse_pattern(const char *name, int *ok) {
    if (strcmp(name, "predictable") == 0) {
        *ok = 1;
        return PATTERN_PREDICTABLE;
    }
    if (strcmp(name, "random") == 0) {
        *ok = 1;
        return PATTERN_RANDOM;
    }
    *ok = 0;
    return PATTERN_PREDICTABLE;
}

static int fill_predictable(program_t *program, uint64_t seed) {
    static const uint8_t cycle[] = {
        OP_LOAD_IMM, OP_ADD, OP_CMP, OP_JNZ, OP_XOR, OP_SUB, OP_MUL, OP_NOP
    };

    uint64_t mix = seed ^ 0x9e3779b97f4a7c15ULL;
    for (size_t i = 0; i < program->len; ++i) {
        const uint8_t op = cycle[i % (sizeof(cycle) / sizeof(cycle[0]))];
        int32_t imm = (int32_t)((i * 17u) ^ (mix >> 32));
        switch (op) {
        case OP_LOAD_IMM:
            imm = (int32_t)(0x1000 + (int32_t)(i & 0x3ffu));
            break;
        case OP_ADD:
            imm = (int32_t)(3 + (i % 11u));
            break;
        case OP_SUB:
            imm = (int32_t)(1 + (i % 7u));
            break;
        case OP_XOR:
            imm = (int32_t)(0x5a5a0000u | (uint32_t)(i & 0xffffu));
            break;
        case OP_MUL:
            imm = (int32_t)(3 + (i % 5u));
            break;
        case OP_CMP:
            imm = (int32_t)(64 + (i % 31u));
            break;
        case OP_JNZ:
            imm = (int32_t)(1 + (i % 9u));
            break;
        case OP_NOP:
            imm = (int32_t)(mix & 0x7ffu);
            break;
        case OP_COUNT:
            break;
        }
        program->code[i].op = op;
        program->code[i].imm = imm;
        mix ^= (uint64_t)imm + 0x9e3779b97f4a7c15ULL;
    }
    return 0;
}

static int fill_random(program_t *program, uint64_t seed) {
    uint64_t rng = seed ? seed : 0x0123456789abcdefULL;
    for (size_t i = 0; i < program->len; ++i) {
        const uint64_t r = next_rng(&rng);
        const uint8_t op = (uint8_t)(r % OP_COUNT);
        int32_t imm = (int32_t)(r >> 32);
        switch (op) {
        case OP_LOAD_IMM:
            imm = (int32_t)(r ^ (r >> 11));
            break;
        case OP_ADD:
            imm = (int32_t)((r & 0x3fu) + 1u);
            break;
        case OP_SUB:
            imm = (int32_t)((r & 0x1fu) + 1u);
            break;
        case OP_XOR:
            imm = (int32_t)(r ^ 0xaaaaaaaa55555555ULL);
            break;
        case OP_MUL:
            imm = (int32_t)(3 + (r % 11u));
            break;
        case OP_CMP:
            imm = (int32_t)(r & 0x7fffU);
            break;
        case OP_JNZ:
            imm = (int32_t)(1 + (r % (program->len > 1 ? program->len - 1 : 1)));
            break;
        case OP_NOP:
            imm = (int32_t)(r >> 48);
            break;
        case OP_COUNT:
            break;
        }
        program->code[i].op = op;
        program->code[i].imm = imm;
    }
    return 0;
}

int program_init(program_t *program, size_t len, pattern_t pattern, uint64_t seed) {
    memset(program, 0, sizeof(*program));
    if (len == 0) {
        return -1;
    }

    program->code = (instruction_t *)calloc(len, sizeof(*program->code));
    if (program->code == NULL) {
        return -1;
    }
    program->len = len;

    switch (pattern) {
    case PATTERN_PREDICTABLE:
        return fill_predictable(program, seed);
    case PATTERN_RANDOM:
        return fill_random(program, seed);
    }
    return -1;
}

void program_destroy(program_t *program) {
    free(program->code);
    program->code = NULL;
    program->len = 0;
}
