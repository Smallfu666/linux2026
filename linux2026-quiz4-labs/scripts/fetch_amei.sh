#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT="$ROOT/src/amei.original.c"
MANIFEST="$ROOT/results/source_manifest.txt"
SHA_OUT="$ROOT/results/source_sha256.txt"
URL="https://gist.github.com/jserv/221efc83f7b60b996dcf9c8b435980d5/raw/amei.c"
EXPECTED_SHA256="300cf588f5532c896bcf56deee62ee32a617cbc850a671956cbf0e65ea9ffb36"
TMP="$ROOT/src/.amei.original.c.tmp"

mkdir -p "$ROOT/src" "$ROOT/results"

write_outputs() {
  SHA=$1
  BYTES=$2
  printf 'source_url=%s source_state=verified_against_expected_upstream sha256=%s expected_sha256=%s bytes=%s\n' \
    "$URL" "$SHA" "$EXPECTED_SHA256" "$BYTES" > "$MANIFEST"
  printf '%s  %s\n' "$SHA" "$OUT" > "$SHA_OUT"
}

verify_existing() {
  SHA=$(sha256sum "$OUT" | awk '{print $1}')
  if [ "$SHA" != "$EXPECTED_SHA256" ]; then
    printf 'ERROR: local %s sha256=%s does not match expected upstream sha256=%s\n' \
      "$OUT" "$SHA" "$EXPECTED_SHA256" >&2
    printf 'Delete %s or rerun with FORCE_FETCH=1 to refresh from upstream.\n' "$OUT" >&2
    exit 1
  fi
  BYTES=$(wc -c < "$OUT" | tr -d ' ')
  write_outputs "$SHA" "$BYTES"
}

fetch_upstream() {
  rm -f "$TMP"
  curl --fail --location "$URL" -o "$TMP"

  if [ ! -s "$TMP" ]; then
    printf 'ERROR: failed to fetch amei.c from %s\n' "$URL" >&2
    exit 1
  fi

  SHA=$(sha256sum "$TMP" | awk '{print $1}')
  if [ "$SHA" != "$EXPECTED_SHA256" ]; then
    printf 'ERROR: fetched source sha256=%s does not match expected upstream sha256=%s\n' \
      "$SHA" "$EXPECTED_SHA256" >&2
    rm -f "$TMP"
    exit 1
  fi

  mv "$TMP" "$OUT"
  BYTES=$(wc -c < "$OUT" | tr -d ' ')
  write_outputs "$SHA" "$BYTES"
}

if [ "${FORCE_FETCH:-0}" = "1" ] || [ ! -s "$OUT" ]; then
  fetch_upstream
else
  verify_existing
fi
