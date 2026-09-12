# M5 T528 S2 Product-Boundary Cleanup

## Scope

Baseline: `833b920c` (T528 S1). This S performs the independent subtraction
identified by the ledger before the executor cutover: monitor presentation is
NXVM product policy, while `vm/machine` may expose only copied facts.

## One Route

- Deleted `vm_machine_print_machine`, `vm_machine_print_bios`,
  `vm_machine_print_status`, `vm_machine_fault_print`,
  `vm_machine_control_print_status`, and the unused FDD printing helper.
- `vm_machine_get_information()` is the sole replacement query. It copies
  profile, CPU, media, firmware, activity and fault facts; it exposes no Core,
  media, profile or control pointer.
- `vm/product/console.c` is now the sole formatter for the retained `INFO`
  monitor text. The output wording and active-state interpretation are retained
  from the removed implementation.

## Core Product Removal

`src/core/product/config.c/.h` contained only
`core_product_parse_memory_kib()`, with no production or test caller. The
static `core-product` CMake target and both former consumers were deleted.
`banner.h` is NXVM entry data, so it now lives at `src/banner.h`. Consequently
`src/core/` contains only `machine/`.

## Similar-Issue Sweep

The following source/CMake sweep is empty after the deletion:

```text
vm_machine_print_ | vm_machine_fault_print | vm_machine_control_print_status
| vm_machine_fdd_print | core_product_parse_memory_kib | core-product | core/product
```

The specialized gate also exposed one unrelated direct `<stdio.h>` inclusion in
the Common UI test fake. It now uses the existing project C facade and links
the test-only `type-facade`; production Common remains independently Lib-only.

## Verification

- Fresh CMake build of `vm-product-session-smoke`: passed
  (`M5:T7:S1:NXVM-SESSION:OK`).
- Full repository-only unit suite: 299/299 passed, 20.61 seconds.
- `verify-current-specialized-gates`: passed, including Common standalone
  Lib-only corpus and ISO C facade verification.
- `Verify-DocumentationGovernance.ps1`: passed.
- `git diff --check`: passed.

## Explicit Transfer

This S does not delete VM executor, lifecycle, Core-time pacing or Common
request dispatch. S1 proved Common lacks the actual executor receiver; that
finite shared mechanism remains assigned to queued T529. No compatibility
executor or second request path was introduced.
