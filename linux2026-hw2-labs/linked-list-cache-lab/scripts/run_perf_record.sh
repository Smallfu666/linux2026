#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BENCH="${BENCH:-$ROOT_DIR/bench}"
OUTPUT="${OUTPUT:-$ROOT_DIR/results/perf.data}"
ALGORITHM="${ALGORITHM:-fast_slow}"
ALLOC_MODE="${ALLOC_MODE:-shuffle}"
NODES="${NODES:-1000000}"
SEED="${SEED:-1}"
SPREAD_BYTES="${SPREAD_BYTES:-4096}"

if [[ "$(uname -s)" != "Linux" ]]; then
  printf 'run_perf_record.sh requires GNU/Linux. Current host: %s\n' "$(uname -s)" >&2
  exit 1
fi

if ! command -v perf >/dev/null 2>&1; then
  printf 'perf is not installed or not in PATH.\n' >&2
  exit 1
fi

if [[ ! -x "$BENCH" ]]; then
  make -C "$ROOT_DIR"
fi

mkdir -p "$(dirname "$OUTPUT")"

perf record -g -o "$OUTPUT" -- \
  "$BENCH" \
  --algorithm "$ALGORITHM" \
  --alloc "$ALLOC_MODE" \
  --n "$NODES" \
  --reps 1 \
  --seed "$SEED" \
  --spread-bytes "$SPREAD_BYTES"

printf 'Recorded %s\n' "$OUTPUT"
printf 'Next: perf report -i %s\n' "$OUTPUT"
