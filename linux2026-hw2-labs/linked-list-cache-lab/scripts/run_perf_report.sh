#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DATA_FILE="${DATA_FILE:-$ROOT_DIR/results/perf.data}"

usage() {
  cat <<'EOF'
Usage: run_perf_report.sh [--data PATH] [-- perf-report-args...]

Examples:
  ./scripts/run_perf_report.sh
  ./scripts/run_perf_report.sh --data results/perf.data -- --stdio
EOF
}

EXTRA_ARGS=()

while (($#)); do
  case "$1" in
    --data)
      [[ $# -ge 2 ]] || {
        printf 'error: --data expects a path\n' >&2
        exit 1
      }
      DATA_FILE="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      EXTRA_ARGS=("$@")
      break
      ;;
    *)
      EXTRA_ARGS+=("$1")
      shift
      ;;
  esac
done

if [[ "$(uname -s)" != "Linux" ]]; then
  printf 'run_perf_report.sh requires GNU/Linux. Current host: %s\n' "$(uname -s)" >&2
  exit 1
fi

if ! command -v perf >/dev/null 2>&1; then
  printf 'perf is not installed or not in PATH.\n' >&2
  exit 1
fi

if [[ ! -f "$DATA_FILE" ]]; then
  printf 'perf data file not found: %s\n' "$DATA_FILE" >&2
  exit 1
fi

if ((${#EXTRA_ARGS[@]} > 0)); then
  exec perf report -i "$DATA_FILE" "${EXTRA_ARGS[@]}"
fi

exec perf report -i "$DATA_FILE"
