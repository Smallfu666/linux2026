#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BENCH="${BENCH:-$ROOT_DIR/bench}"
OUTPUT="${OUTPUT:-$ROOT_DIR/results/local_elapsed.csv}"
ALGORITHMS="${ALGORITHMS:-fast_slow two_pass}"
ALLOC_MODES="${ALLOC_MODES:-contiguous malloc shuffle page_spread}"
LENGTHS="${LENGTHS:-10000 100000 1000000}"
TRIALS="${TRIALS:-3}"
SEED="${SEED:-1}"
SPREAD_BYTES="${SPREAD_BYTES:-4096}"
MAX_SPREAD_BYTES="${MAX_SPREAD_BYTES:-1073741824}"

if [[ ! -x "$BENCH" ]]; then
  make -C "$ROOT_DIR"
fi

mkdir -p "$(dirname "$OUTPUT")"
printf 'algorithm,alloc_mode,n,trial,elapsed_ns,checksum\n' >"$OUTPUT"

for algorithm in $ALGORITHMS; do
  for alloc_mode in $ALLOC_MODES; do
    for n in $LENGTHS; do
      if [[ "$alloc_mode" == "page_spread" ]] && (( n * SPREAD_BYTES > MAX_SPREAD_BYTES )); then
        printf 'Skipping page_spread for n=%s stride=%s: exceeds MAX_SPREAD_BYTES=%s\n' \
          "$n" "$SPREAD_BYTES" "$MAX_SPREAD_BYTES" >&2
        continue
      fi

      "$BENCH" \
        --algorithm "$algorithm" \
        --alloc "$alloc_mode" \
        --n "$n" \
        --reps "$TRIALS" \
        --seed "$SEED" \
        --spread-bytes "$SPREAD_BYTES" \
        --csv >>"$OUTPUT"
    done
  done
done

printf 'Wrote %s\n' "$OUTPUT"
