#ifndef DISPATCH_BYTECODE_H
#define DISPATCH_BYTECODE_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    OP_ADD = 0,
    OP_SUB,
    OP_XOR,
    OP_MUL,
    OP_LOAD_IMM,
    OP_CMP,
    OP_JNZ,
    OP_NOP,
    OP_COUNT
} opcode_t;

typedef enum {
    PATTERN_PREDICTABLE = 0,
    PATTERN_RANDOM
} pattern_t;

typedef struct {
    uint8_t op;
    int32_t imm;
} instruction_t;

typedef struct {
    instruction_t *code;
    size_t len;
} program_t;

const char *pattern_name(pattern_t pattern);
pattern_t parse_pattern(const char *name, int *ok);
int program_init(program_t *program, size_t len, pattern_t pattern, uint64_t seed);
void program_destroy(program_t *program);

#endif
