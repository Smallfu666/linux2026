# Results Notes

Generated artifacts for this lab live here.

Expected files:

- `local_elapsed.csv`: cross-platform elapsed-time sample from `scripts/run_bench_matrix.sh`
- `perf_stat.csv`: GNU/Linux `perf stat` matrix output from `scripts/run_perf_stat.sh`
- `perf.data`: optional GNU/Linux profile from `scripts/run_perf_record.sh`

Validation status on 2026-05-04:

- `results/local_elapsed.csv` was regenerated in WSL2 Linux x86_64 with GCC
- `perf` is not installed in this WSL image, so `perf_stat.csv` and `perf.data` could not be produced here
- the `perf` helper scripts remain in the lab for a GNU/Linux host that has `perf`

The repository can safely include small sample outputs for validation, but the main `perf` results should be regenerated on a GNU/Linux target host used for analysis.
