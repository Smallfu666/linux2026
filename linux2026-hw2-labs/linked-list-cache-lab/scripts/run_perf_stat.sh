#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BENCH="${BENCH:-$ROOT_DIR/bench}"
OUTPUT="${OUTPUT:-$ROOT_DIR/results/perf_stat.csv}"
ALGORITHMS="${ALGORITHMS:-fast_slow two_pass}"
ALLOC_MODES="${ALLOC_MODES:-contiguous malloc shuffle page_spread}"
LENGTHS="${LENGTHS:-10000 100000 1000000 10000000}"
TRIALS="${TRIALS:-5}"
SEED="${SEED:-1}"
SPREAD_BYTES="${SPREAD_BYTES:-4096}"
MAX_SPREAD_BYTES="${MAX_SPREAD_BYTES:-1073741824}"
EVENTS="${EVENTS:-cycles,instructions,cache-references,cache-misses,branches,branch-misses,LLC-loads,LLC-load-misses}"

extract_perf_value() {
  local file="$1"
  local event="$2"

  awk -F, -v event="$event" '
    $3 == event {
      gsub(/[[:space:]]/, "", $1);
      print $1;
      found = 1;
      exit;
    }
    END {
      if (!found) {
        print "NA";
      }
    }
  ' "$file"
}

if [[ "$(uname -s)" != "Linux" ]]; then
  printf 'run_perf_stat.sh requires GNU/Linux. Current host: %s\n' "$(uname -s)" >&2
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
printf 'algorithm,alloc_mode,n,trial,cycles,instructions,cache_references,cache_misses,branches,branch_misses,llc_loads,llc_load_misses,elapsed_ns,checksum\n' >"$OUTPUT"

for algorithm in $ALGORITHMS; do
  for alloc_mode in $ALLOC_MODES; do
    for n in $LENGTHS; do
      if [[ "$alloc_mode" == "page_spread" ]] && (( n * SPREAD_BYTES > MAX_SPREAD_BYTES )); then
        printf 'Skipping page_spread for n=%s stride=%s: exceeds MAX_SPREAD_BYTES=%s\n' \
          "$n" "$SPREAD_BYTES" "$MAX_SPREAD_BYTES" >&2
        continue
      fi

      for ((trial = 1; trial <= TRIALS; ++trial)); do
        stdout_file="$(mktemp)"
        stderr_file="$(mktemp)"

        perf stat -x, -e "$EVENTS" -- \
          "$BENCH" \
          --algorithm "$algorithm" \
          --alloc "$alloc_mode" \
          --n "$n" \
          --reps 1 \
          --seed "$((SEED + trial - 1))" \
          --spread-bytes "$SPREAD_BYTES" \
          --csv >"$stdout_file" 2>"$stderr_file"

        IFS=, read -r bench_algorithm bench_alloc bench_n bench_trial elapsed_ns checksum <"$stdout_file"

        cycles="$(extract_perf_value "$stderr_file" "cycles")"
        instructions="$(extract_perf_value "$stderr_file" "instructions")"
        cache_references="$(extract_perf_value "$stderr_file" "cache-references")"
        cache_misses="$(extract_perf_value "$stderr_file" "cache-misses")"
        branches="$(extract_perf_value "$stderr_file" "branches")"
        branch_misses="$(extract_perf_value "$stderr_file" "branch-misses")"
        llc_loads="$(extract_perf_value "$stderr_file" "LLC-loads")"
        llc_load_misses="$(extract_perf_value "$stderr_file" "LLC-load-misses")"

        printf '%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n' \
          "$bench_algorithm" \
          "$bench_alloc" \
          "$bench_n" \
          "$bench_trial" \
          "$cycles" \
          "$instructions" \
          "$cache_references" \
          "$cache_misses" \
          "$branches" \
          "$branch_misses" \
          "$llc_loads" \
          "$llc_load_misses" \
          "$elapsed_ns" \
          "$checksum" >>"$OUTPUT"

        rm -f "$stdout_file" "$stderr_file"
      done
    done
  done
done

printf 'Wrote %s\n' "$OUTPUT"
