# Linux Validation Notes

Validation run: WSL2 Linux x86_64 on 2026-05-04

Toolchain:
- `gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`
- `Ubuntu clang version 18.1.3 (1ubuntu1)`
- `Linux DESKTOP-OK6POFL 6.6.87.2-microsoft-standard-WSL2`

Results:
- `make clean all` passed after a small portability fix in `labs/bc_list_hlist/bc_list_hlist.c`.
- `make -C labs/a_pointer_layout clean all` passed.
- `make -C labs/bc_list_hlist clean all` passed.
- `make -C labs/e_hash clean all` passed.
- `make -C labs/fk_bit_fixedpoint clean all` passed.
- `make -C labs/ch_sort_trace clean all` passed.

Observed Linux-specific note:
- `bc_list_hlist.c` used `PATH_MAX` for stack buffers, but this symbol was not visible in the Linux build as configured. A local fallback definition keeps the code portable without changing behavior.

Compiler matrix note:
- `labs/a_pointer_layout/compile_matrix.md` regenerated successfully on Linux.
- The expected `-std=c99` failure for the C11 target remains present.
- GCC and Clang report the same failure mode, with only minor diagnostic wording differences.
