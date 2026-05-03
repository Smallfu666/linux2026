# bc_list_hlist

Tiny user-space lab for the Linux list and hlist idioms used in Quiz 3.

What it covers:

- minimal `list_head` helpers
- minimal `hlist_head` / `hlist_node` helpers
- indirect-pointer deletion with `probe_node_t **pp`
- head, middle, and tail delete traces
- safe vs unsafe list iteration deletion

Run it:

```sh
make run
```

Or:

```sh
./run.sh
```

Outputs land in `artifacts/`:

- `delete_head.dot`
- `delete_middle.dot`
- `delete_tail.dot`
- `iteration.txt`
- `summary.txt`

The `.dot` files are plain Graphviz traces; they are generated even if Graphviz is not installed.
