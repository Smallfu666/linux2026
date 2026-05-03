# ch_sort_trace

Mini lab for quiz 3 Problems C/H:

- `list_sort()` pending-run counter trace for `n=13`
- standard heapsort vs bottom-up heapsort comparison counts

## Build and run

```sh
make
```

Generated files live under `artifacts/`:

- `artifacts/list_sort_trace_n13.csv`
- `artifacts/list_sort_trace_n13.txt`
- `artifacts/heapsort_comparisons.csv`

## Notes

- `trailing_ones` is the carry-chain length when advancing the pending-run counter from `count - 1` to `count`.
- The heapsort benchmark uses a fixed xorshift shuffle seed per `n`, so the CSV is reproducible.
- `list_sort_trace_n13.txt` is generated with plain `tr`, so the lab does not depend on `column(1)`.

## Targets

```sh
make trace
make bench
make clean
```
