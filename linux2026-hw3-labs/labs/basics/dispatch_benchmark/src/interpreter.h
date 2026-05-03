#ifndef DISPATCH_INTERPRETER_H
#define DISPATCH_INTERPRETER_H

#include "bytecode.h"

#include <stdint.h>

typedef enum {
    DISPATCH_SWITCH = 0,
    DISPATCH_COMPUTED
} dispatch_mode_t;

const char *dispatch_name(dispatch_mode_t mode);
dispatch_mode_t parse_dispatch(const char *name, int *ok);
uint64_t run_interpreter(dispatch_mode_t mode, const program_t *program, size_t steps, uint64_t seed);

#endif
