# linux2026 quiz4 amei.c lab

This repository contains a reproducible experiment suite for the `amei.c` decoder
used in Linux core design 2026 q1 quiz4.

## Environment

Recorded on this machine:

- `uname -a`
  - `Linux DESKTOP-OK6POFL 6.6.87.2-microsoft-standard-WSL2 #1 SMP PREEMPT_DYNAMIC Thu Jun  5 18:30:46 UTC 2025 x86_64 x86_64 x86_64 GNU/Linux`
- `cc --version | head -n 1`
  - `cc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`
- `python3 --version`
  - `Python 3.14.4`

Current optional Python packages:

- `numpy`: missing
- `PIL`: missing
- `matplotlib`: missing

Compiler flags:

```sh
CFLAGS_COMMON="-std=c99 -Wall -Wextra -Wpedantic -g"
CFLAGS_OPT="-O2"
CFLAGS_SAN="-O1 -g -fsanitize=undefined,address -fno-omit-frame-pointer"
```

## Source

- Quiz page source: see `REPORT.md`
- `amei.c` source URL: `https://gist.github.com/jserv/221efc83f7b60b996dcf9c8b435980d5/raw/amei.c`
- `amei.c` sha256: see `results/source_sha256.txt`

## Layout

- `src/amei.original.c`: unmodified upstream source
- `src/amei.instrumented.c`: instrumentation-enabled copy
- `src/`: standalone experiment programs
- `ub_tests/`: sanitizer / UB probes
- `results/`: all generated outputs

## Reproducible commands

```sh
make fetch
make dct
make idct
make instrument
make context
make exp-golomb
make filter-ablation
make ub
make crc
make color
make memory
make report
make all
```

`make all` runs:

```sh
make fetch && make dct && make idct && make instrument && make context && make exp-golomb && make filter-ablation && make ub && make crc && make color && make memory && make report
```

## Notes

- All generated artifacts stay under `results/`.
- `color` is expected to be skipped on this machine because `numpy` is not installed.
- `REPORT.md` is a factual summary of observations, not a quiz answer sheet.
