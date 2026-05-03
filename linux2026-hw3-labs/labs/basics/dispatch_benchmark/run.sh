#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="${BIN:-$ROOT_DIR/dispatch_benchmark}"
OUTPUT="${OUTPUT:-$ROOT_DIR/results/local_elapsed.csv}"

if [[ ! -x "$BIN" ]]; then
  make -C "$ROOT_DIR"
fi

mkdir -p "$(dirname "$OUTPUT")"
: >"$OUTPUT"

"$BIN" \
  --pattern all \
  --dispatch all \
  --program-len "${PROGRAM_LEN:-32768}" \
  --steps "${STEPS:-262144}" \
  --reps "${REPS:-3}" \
  --seed "${SEED:-1}" \
  --csv >>"$OUTPUT"

cat >"$ROOT_DIR/results/perf_switch.txt" <<EOF
Run on Linux x86_64:
PROGRAM_LEN=32768 STEPS=1000000 REPS=10 CSV=1 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses ./bench_switch predictable
PROGRAM_LEN=32768 STEPS=1000000 REPS=10 CSV=1 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses ./bench_switch random
EOF

cat >"$ROOT_DIR/results/perf_cgoto.txt" <<EOF
Run on Linux x86_64:
PROGRAM_LEN=32768 STEPS=1000000 REPS=10 CSV=1 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses ./bench_cgoto predictable
PROGRAM_LEN=32768 STEPS=1000000 REPS=10 CSV=1 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses ./bench_cgoto random
EOF

printf 'Wrote %s\n' "$OUTPUT"
