# M5 T551 Shared Product-To-X86 Normalization

## Task Record

The owner admitted this task on 2026-10-10 to rename the portable shared
Product component to `x86` and its `surface` subsystem to `product`.

The accepted proposal is
[M5 shared Product-to-x86 normalization](../proposals/m5-shared-product-x86-normalization.md).
Current holds the active S packet and this record receives accepted evidence.

## Closure Evidence

S1 moved the portable shared corpus from `src/product` and `test/product` to
`src/x86` and `test/x86`, and renamed former Surface ownership to
`x86/product`. Shared commit `1e35f83c1` performed the corpus move; NXVM
commit `7e81e37ea` rewired Core and App receivers. NXVM-private `core/x86`
remained unchanged.

S2 completed full receiving qualification on the original x64/x86 Ninja
ccache build directories. Root unit qualification passed 544/544 on each
width. External integration passed 21/21 on each width. MyNES qualification
passed 57/57 on each width, including its serialized native Console/Window
desktop probes. NXVM and MyNES documentation-governance gates passed.

The eight obsolete 0.5.0546 PC binaries were retired. The four PC Apps now
ship their 0.5.0551 x64/x86 pairs and MyNES ships its freshly rebuilt current
0.0.0044 x64/x86 pair. PE inspection confirms every x64 artifact is
`pei-x86-64` and every x86 artifact is `pei-i386`. A live old-identity scan
found no retired portable `product` path or `product_surface_` symbol; the
retained `core/x86` identity is explicitly private and unrelated.

T551 closed on 2026-10-10 after the required artifact, source, test and
documentation evidence was complete.
