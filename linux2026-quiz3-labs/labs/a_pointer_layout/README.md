# a_pointer_layout

Mini lab for pointer layout, array decay, `offsetof`, and `container_of` variants.

## Covers

- Array decay and `sizeof(a)`, `sizeof(p)`, `sizeof(*p)`.
- `&a + 1` vs `p + 5`.
- `ptrdiff_t` element deltas and `char *` byte deltas.
- `offsetof` on nested structs.
- `container_of` implementations:
  - plain `char *` subtraction
  - GNU statement expression + `typeof`
  - `_Static_assert` checked version
  - `_Generic` const-preserving version

## Run

```sh
make all
```

Or run the script directly:

```sh
./run.sh matrix
./run.sh demo
```

## Generated Artifacts

- `compile_matrix.md`
- `artifacts/portable_stdout.txt`
- `artifacts/gnu_stdout.txt`
- `artifacts/c11_stdout.txt`

