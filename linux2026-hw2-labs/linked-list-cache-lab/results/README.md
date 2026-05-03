# Results Notes

Generated artifacts for this lab live here.

Expected files:

- `local_elapsed.csv`: cross-platform elapsed-time sample from `scripts/run_bench_matrix.sh`
- `perf_stat.csv`: GNU/Linux `perf stat` matrix output from `scripts/run_perf_stat.sh`
- `perf.data`: optional GNU/Linux profile from `scripts/run_perf_record.sh`
- `run_perf_report.sh`: helper to inspect `perf.data` on GNU/Linux

The repository can safely include small sample outputs for validation, but the main `perf` results should be regenerated on the actual GNU/Linux target host used for analysis.
