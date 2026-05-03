# fk_bit_fixedpoint

Mini lab for the Linux quiz 3 boundary cases:

- `BIT(n)` and `GENMASK(h,l)`
- `abs(INT_MIN)`
- `time_after(a, b)` wraparound boundaries
- EWMA warm-up vs no warm-up
- PELT-style `period_contrib` accumulation

## Run

```bash
make
```

Artifacts are written to `out/` and the main write-up is `boundary_cases.md`.

## Files

- `lab.py` generates all outputs.
- `Makefile` runs the lab.
- `boundary_cases.md` summarizes the edge cases.
- `out/abs_int_min_cases.csv` shows which inputs still have a representable `int32` absolute value.
- `out/*.csv` and `out/*.txt` contain the supporting data.
