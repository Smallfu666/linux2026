#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="${BIN:-$ROOT_DIR/tree_cache_locality}"
OUTPUT="${OUTPUT:-$ROOT_DIR/results/local_elapsed.csv}"

if [[ ! -x "$BIN" ]]; then
  make -C "$ROOT_DIR"
fi

mkdir -p "$(dirname "$OUTPUT")"
: >"$OUTPUT"

"$BIN" \
  --mode all \
  --key-count "${KEY_COUNT:-65536}" \
  --lookups "${LOOKUPS:-262144}" \
  --reps "${REPS:-3}" \
  --seed "${SEED:-1}" \
  --csv >>"$OUTPUT"

cat >"$ROOT_DIR/results/perf_bst.txt" <<EOF
Run on Linux x86_64:
perf stat -e cycles,instructions,cache-references,cache-misses,branches,branch-misses ./bench --type=bst --key-count 100000 --lookups 1000000 --reps 5 --csv
EOF

cat >"$ROOT_DIR/results/perf_btree16.txt" <<EOF
Run on Linux x86_64:
perf stat -e cycles,instructions,cache-references,cache-misses,branches,branch-misses ./bench --type=btree16 --key-count 100000 --lookups 1000000 --reps 5 --csv
EOF

printf 'Wrote %s\n' "$OUTPUT"
