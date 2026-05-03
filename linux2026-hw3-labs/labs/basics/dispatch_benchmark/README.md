# Dispatch Benchmark

This lab compares two bytecode interpreters:

- `switch`: a traditional `switch`/`case` dispatch loop.
- `computed-goto`: a GCC-style direct-threaded interpreter using label addresses.

The benchmark uses one shared bytecode generator so the two dispatch styles execute the same program data. The instruction mix includes at least eight opcodes:

- `ADD`
- `SUB`
- `XOR`
- `MUL`
- `LOAD_IMM`
- `CMP`
- `JNZ`
- `NOP`

## Layout

- `src/bytecode.c`: bytecode generator for predictable and random opcode streams.
- `src/interpreter.c`: shared interpreter logic for switch and computed-goto dispatch.
- `src/bench.c`: multi-mode CLI benchmark driver and CSV output.
- `src/bench_switch.c`: fixed `switch` frontend for `./bench_switch predictable`.
- `src/bench_cgoto.c`: fixed computed-goto frontend for `./bench_cgoto predictable`.
- `src/timing.c`: portable monotonic timer helper.
- `run.sh`: smoke-run helper that writes `results/local_elapsed.csv`.
- `results/`: output placeholder directory.

## Build

```bash
make
```

You can choose the compiler explicitly:

```bash
make CC=clang
make CC=gcc
```

The build uses `-Wall -Wextra` and `-std=gnu11` so both clang on macOS and gcc on Linux should work.

## Benchmark Usage

Primary commands:

```bash
./dispatch_benchmark --pattern predictable --dispatch switch --program-len 32768 --steps 262144 --reps 5
./dispatch_benchmark --pattern random --dispatch computed --program-len 32768 --steps 262144 --reps 5
./dispatch_benchmark --pattern all --dispatch all --csv
./bench_switch predictable
./bench_cgoto predictable
```

Flags:

- `--pattern predictable|random|all`
- `--dispatch switch|computed|all`
- `--program-len N`
- `--steps N`
- `--reps N`
- `--seed N`
- `--csv`

The generated program is reused across dispatch variants so the dispatch mechanism, not the input generator, dominates the timing.

## Control Flow

The bytecode stream is a linear instruction array. Each interpreter keeps a program counter, decodes one instruction, updates a small VM state, and advances to the next instruction. `JNZ` is intentionally bounded and data-dependent so it exercises branch behavior without turning the benchmark into an uncontrolled control-flow graph.

That makes this a dispatch benchmark, not a compiler or VM correctness test. The instructions are deliberately simple and the state updates are artificial.

## Linux perf Usage

This lab is meant to be profiled later on Linux x86_64 with `perf`. A typical run is:

```bash
PROGRAM_LEN=32768 STEPS=1000000 REPS=10 CSV=1 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses ./bench_switch predictable
PROGRAM_LEN=32768 STEPS=1000000 REPS=10 CSV=1 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses ./bench_cgoto predictable
PROGRAM_LEN=32768 STEPS=1000000 REPS=10 CSV=1 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses ./bench_switch random
PROGRAM_LEN=32768 STEPS=1000000 REPS=10 CSV=1 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses ./bench_cgoto random
```

Useful counters:

- `cycles`
- `instructions`
- `branches`
- `branch-misses`

The interesting comparison is usually between predictable and random opcode streams, and between `switch` dispatch and computed goto. The most direct Linux-side artifacts to save are:

- `results/perf_switch.txt`
- `results/perf_cgoto.txt`

## Notes

- The benchmark prints CSV so the data can be reused later without re-running the experiment design.
- The output in `results/` is intentionally lightweight; no generated binaries or large artifacts are checked in here.
- Computed goto uses GNU C extensions (`&&label` and `goto *ptr`), so this lab intentionally builds with `-std=gnu11`.
