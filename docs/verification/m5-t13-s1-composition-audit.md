# M5 T13 S1 Composition Audit

The canonical source roots are closed, but M5 cannot yet close merely because
the old roots are gone. The audit separates safe historical-name cleanup from
real ownership work.

## Name-Only VM Owners

These files already live under their correct VM owner and can be renamed by
one `git mv` pair at a time without changing their API behavior:

- `vm/product/baseline_full_pc.*` is the active full-PC composition, used by
  the default VM profile and the product-session tests;
- `vm/platform/baseline_request_transport.*` is VM platform ingress transport;
- `vm/product/baseline_cpu_probe.*` is the VM-only finite CPU verifier.

The corresponding CMake target names and focused smoke names carry the same
historical label. T13 S2 may remove that label from the first pair only, then
build and run the full-PC, CPU, FDD/HDD, Console, and debugger gates.

## Real Ownership Debt

The following cannot be renamed away and blocks M5 closure until separate
bounded slices resolve it:

- `core/product/runtime/session.c` selects both `nxvm.full_pc` and
  `ntvdm64.dos_minimal`, and includes VM/VDM implementation headers. It must
  become product-neutral lifecycle infrastructure with VM and VDM composition
  supplied by their own product owners, or be split by `git mv` according to
  actual product ownership.
- `core/platform/presentation.h` includes the VDM minimal-machine header.
  Its shared presentation contract must use a core-owned snapshot contract.
- `core/product/utils.c` calls the VM platform sleep service; the core utility
  must receive a product/host callback rather than select a VM platform.
- `core/machine/vcpuins.c` calls VM device stop, while PIC/PIT/DMA/KBC directly
  include default-profile BIOS headers. These are legacy behavioral links and
  need explicit machine callbacks or profile registration, retaining original
  execution order and all whole-PC gates.

No `core/*` source may retain a VM/VDM include at M5 closure. The audit found
no old top-level source-root dependency in CMake or production source paths.
No user-visible behavior changed during this audit.
