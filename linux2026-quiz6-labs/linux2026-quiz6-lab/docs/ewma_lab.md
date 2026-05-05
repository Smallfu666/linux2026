# Experiment D: fixed-point EWMA

This experiment corresponds to the quiz6 TCP `srtt_us` /
`DECLARE_EWMA`-style fixed-point arithmetic topic.

## What It Verifies

The program compares a floating EWMA:

```c
avg = avg * (1 - alpha) + x * alpha
```

with a fixed-point EWMA:

```c
internal = internal - (internal >> weight_shift)
           + ((x << precision) >> weight_shift);
```

It runs four sequences for 100 samples each:

- `constant100`
- `step0to100`
- `small1`
- `alternating`

and tests `precision = 0, 3, 8, 10` with `weight_rcp = 8, 16`.

## Actual Output

`make run` prints final floating and fixed averages for each parameter set and
writes:

```text
results/ewma_results.csv
```

The CSV columns are:

```text
sequence_name,precision,weight_rcp,step,input,float_avg,fixed_avg,abs_error
```

The `small1` rows show the truncation problem most clearly. With `precision=0`,
`((1 << precision) >> weight_shift)` is zero for weights 8 and 16, so the fixed
average never receives the small increment.

## Portability Notes

The fixed-point update uses unsigned integer arithmetic with bounded test inputs.
The chosen reciprocal weights are powers of two, so division is implemented as a
right shift. That is the reason this style is attractive in kernel code: it
avoids floating point and division in hot paths.

The tradeoff is truncation. Right shifts discard low fractional bits. Keeping
more fractional precision, such as 3 bits for an 8x scale or 8/10 bits for this
lab, reduces the long-term downward bias. Linux kernel code often stores scaled
values such as `srtt_us << 3`, then uses `>> 3` when it needs the integer-scale
value again.
