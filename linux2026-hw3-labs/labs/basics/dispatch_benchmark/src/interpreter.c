#include "interpreter.h"

#include <string.h>

typedef struct {
    uint64_t acc;
    uint64_t aux;
    int64_t cmp;
} vm_state_t;

static inline void vm_add(vm_state_t *state, int32_t imm) {
    state->acc += state->aux + (uint64_t)imm;
}

static inline void vm_sub(vm_state_t *state, int32_t imm) {
    const int64_t abs_imm = imm < 0 ? -(int64_t)imm : (int64_t)imm;
    state->acc -= (uint64_t)abs_imm;
}

static inline void vm_xor(vm_state_t *state, int32_t imm) {
    state->acc ^= (uint64_t)imm;
}

static inline void vm_mul(vm_state_t *state, int32_t imm) {
    const uint64_t factor = (uint64_t)((imm & 0x1f) + 1);
    state->acc = state->acc * factor + state->aux;
}

static inline void vm_load_imm(vm_state_t *state, int32_t imm) {
    state->aux = (uint64_t)(uint32_t)imm;
}

static inline void vm_cmp(vm_state_t *state, int32_t imm) {
    state->cmp = (int64_t)state->acc - (int64_t)imm;
}

static inline void vm_nop(vm_state_t *state, size_t pc) {
    state->acc ^= (uint64_t)pc;
}

static inline size_t normalize_offset(int32_t imm, size_t len) {
    if (len == 0) {
        return 0;
    }
    int64_t off = (int64_t)imm % (int64_t)len;
    if (off < 0) {
        off += (int64_t)len;
    }
    return (size_t)off;
}

static inline size_t vm_next_pc(const program_t *program, const vm_state_t *state, size_t pc, const instruction_t *insn) {
    const size_t len = program->len;
    switch (insn->op) {
    case OP_JNZ:
        if (state->cmp != 0) {
            return (pc + 1 + normalize_offset(insn->imm, len)) % len;
        }
        break;
    default:
        break;
    }
    return (pc + 1) % len;
}

const char *dispatch_name(dispatch_mode_t mode) {
    switch (mode) {
    case DISPATCH_SWITCH:
        return "switch";
    case DISPATCH_COMPUTED:
        return "computed";
    }
    return "unknown";
}

dispatch_mode_t parse_dispatch(const char *name, int *ok) {
    if (strcmp(name, "switch") == 0) {
        *ok = 1;
        return DISPATCH_SWITCH;
    }
    if (strcmp(name, "computed") == 0) {
        *ok = 1;
        return DISPATCH_COMPUTED;
    }
    *ok = 0;
    return DISPATCH_SWITCH;
}

static uint64_t finish_checksum(const vm_state_t *state, size_t pc, size_t steps) {
    return state->acc ^ (state->aux << 1) ^ (uint64_t)state->cmp ^ (pc << 8) ^ steps;
}

static uint64_t run_switch(const program_t *program, size_t steps, uint64_t seed) {
    vm_state_t state = {
        .acc = seed ^ 0x243f6a8885a308d3ULL,
        .aux = seed + 0x9e3779b97f4a7c15ULL,
        .cmp = (int64_t)(seed ^ (seed >> 7))
    };
    size_t pc = 0;

    if (steps == 0) {
        return finish_checksum(&state, pc, steps);
    }

    for (size_t executed = 0; executed < steps; ++executed) {
        const instruction_t *insn = &program->code[pc];
        switch (insn->op) {
        case OP_ADD:
            vm_add(&state, insn->imm);
            break;
        case OP_SUB:
            vm_sub(&state, insn->imm);
            break;
        case OP_XOR:
            vm_xor(&state, insn->imm);
            break;
        case OP_MUL:
            vm_mul(&state, insn->imm);
            break;
        case OP_LOAD_IMM:
            vm_load_imm(&state, insn->imm);
            break;
        case OP_CMP:
            vm_cmp(&state, insn->imm);
            break;
        case OP_JNZ:
            vm_nop(&state, pc);
            break;
        case OP_NOP:
            vm_nop(&state, pc);
            break;
        case OP_COUNT:
            break;
        }
        pc = vm_next_pc(program, &state, pc, insn);
    }

    return finish_checksum(&state, pc, steps);
}

static uint64_t run_computed_goto(const program_t *program, size_t steps, uint64_t seed) {
#if defined(__GNUC__) || defined(__clang__)
    vm_state_t state = {
        .acc = seed ^ 0x243f6a8885a308d3ULL,
        .aux = seed + 0x9e3779b97f4a7c15ULL,
        .cmp = (int64_t)(seed ^ (seed >> 7))
    };
    size_t pc = 0;
    size_t executed = 0;
    const instruction_t *insn;
    static void *const dispatch[] = {
        &&op_add,
        &&op_sub,
        &&op_xor,
        &&op_mul,
        &&op_load_imm,
        &&op_cmp,
        &&op_jnz,
        &&op_nop
    };

    if (steps == 0) {
        return finish_checksum(&state, pc, steps);
    }

#define ADVANCE()                                                                       \
    do {                                                                                \
        pc = vm_next_pc(program, &state, pc, insn);                                     \
        ++executed;                                                                     \
        if (executed >= steps) {                                                        \
            goto done;                                                                  \
        }                                                                               \
        goto dispatch_entry;                                                            \
    } while (0)

dispatch_entry:
    insn = &program->code[pc];
    goto *dispatch[insn->op];

op_add:
    vm_add(&state, insn->imm);
    ADVANCE();
op_sub:
    vm_sub(&state, insn->imm);
    ADVANCE();
op_xor:
    vm_xor(&state, insn->imm);
    ADVANCE();
op_mul:
    vm_mul(&state, insn->imm);
    ADVANCE();
op_load_imm:
    vm_load_imm(&state, insn->imm);
    ADVANCE();
op_cmp:
    vm_cmp(&state, insn->imm);
    ADVANCE();
op_jnz:
    vm_nop(&state, pc);
    ADVANCE();
op_nop:
    vm_nop(&state, pc);
    ADVANCE();

done:
#undef ADVANCE
    return finish_checksum(&state, pc, executed);
#else
    (void)seed;
    return run_switch(program, steps, seed);
#endif
}

uint64_t run_interpreter(dispatch_mode_t mode, const program_t *program, size_t steps, uint64_t seed) {
    switch (mode) {
    case DISPATCH_SWITCH:
        return run_switch(program, steps, seed);
    case DISPATCH_COMPUTED:
        return run_computed_goto(program, steps, seed);
    }
    return 0;
}
