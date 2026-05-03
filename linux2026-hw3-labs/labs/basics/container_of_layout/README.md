# Container Of Layout Lab

This lab builds a tiny userspace model of the Linux object layout style: embedded structs, `container_of`, and low-bit pointer tagging. It focuses on evidence you can inspect directly without needing kernel headers or a kernel module.

## Files

- `container_of_demo.c`: demonstrates nested embedding and pointer recovery from `kobject_like *` to `device_like *` to `pci_dev_like *`.
- `pointer_tagging_demo.c`: shows why packing a color bit into a parent pointer depends on alignment guarantees.
- `Makefile`: build entry point.
- `run.sh`: builds both demos and writes text output into `results/`.
- `results/`: generated output.

## Build

```sh
make
```

Use another compiler if needed:

```sh
make CC=clang
make CC=gcc
```

## Run

```sh
./run.sh
```

Generated artifacts:

- `results/container_of_demo.txt`
- `results/pointer_tagging_demo.txt`

## container_of Formula

The macro used here is the classic kernel idea:

```c
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
```

This lab uses the GNU `typeof` form so the compiler also checks that `ptr` has the right member type. That is why the build uses `-std=gnu11`, not strict ISO C11.

## Memory Layout Intuition

The demo structs are nested like this:

```text
struct pci_dev_like
+-- vendor_id
+-- device_id
+-- struct device_like dev
    +-- device_number
    +-- struct kobject_like kobj
        +-- name
        +-- refcount
    +-- driver_name
+-- bus
+-- slot
```

If you only hold `kobject_like *`, you can subtract `offsetof(struct device_like, kobj)` to recover the enclosing `device_like *`. Then subtract `offsetof(struct pci_dev_like, dev)` to recover the enclosing `pci_dev_like *`.

This is different from C++ inheritance with a vptr. Nothing here is runtime type metadata. It is plain pointer arithmetic over a known memory layout.

## Pointer Tagging

The second demo models the `__rb_parent_color` idea used by Linux rbtree code:

- the parent pointer is aligned, so its low bit is normally zero
- one low bit is reused to store node color
- recovering the parent means masking that bit back out

The packed wrapper case intentionally weakens alignment:

- `struct packed_wrapper` has alignment 1
- the embedded node can start at an odd address
- if bit 0 is already part of the real address, using it as a color tag corrupts the pointer

That is why pointer tagging depends on layout and alignment guarantees, not just syntax.

## Kernel Relevance

- Embedded structs are the basis for the kernel's object model.
- `container_of` is how generic subsystems call back into more specific enclosing objects.
- Pointer tagging is a space-saving trick that only works when alignment makes low bits available.

## Limits

- These demos are simplified userspace models, not drop-in copies of `struct device`, `struct pci_dev`, or the real rbtree implementation.
- Packed-struct behavior can vary across compilers and architectures, but the core lesson remains: once alignment assumptions go away, low-bit tagging becomes unsafe.
