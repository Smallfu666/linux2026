# Problem E: Hash Lab

This lab compares four multiplicative hash constants over keys `0..10000` with `1024` buckets:

- `0x61C88647`
- `0x9E3779B9`
- `0x80000000`
- `0x12345678`

The hash used for the bucket study is:

```text
bucket = ((key * constant) & 0xffffffff) >> 22
```

The generator also runs a Jenkins `final`-mix avalanche test on the `a` word, holding `b` and `c` fixed, and records a `32x32` flip-probability matrix.

## Run

From the repo root:

```sh
make e_hash
```

Or from this directory:

```sh
make
sh run.sh
```

## Outputs

Generated files land in `labs/e_hash/out/`:

- `bucket_occupancy.csv`
- `hash_stats.csv`
- `hash_summary.txt`
- `hist_*.png`
- `avalanche_matrix.csv`
- `avalanche_summary.txt`
- `avalanche_heatmap.png`

## Notes

- The key range is inclusive, so there are `10001` keys total.
- The PNGs are written with a pure standard-library encoder, so no plotting packages are required.
