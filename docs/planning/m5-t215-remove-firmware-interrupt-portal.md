# M5 T215: Remove Generic Firmware-Interrupt Portal

## Goal

Remove the unused generic firmware-interrupt portal mechanism from
`core/machine`. The only supported guest software interrupt path becomes CPU
`INT` decoding followed by ordinary IVT delivery.

## S1: Consumer Proof And Contract

Search `src/vm`, `src/vdm`, and profile sources for portal registration,
freeze, and dispatch consumers. The pre-change result is zero production
consumers. Retain ordinary `INT`/IVT semantics with a focused core smoke for
8086 and 80386 decoding.

## S2: Core And Build Graph Removal

Delete the public portal interface, configuration API, executor context
storage, registration/freeze/dispatch helpers, and the CPU decoder branch.
Delete portal-only smoke and authority coverage, plus the obsolete closure
gate. Do not change normal `INT`, IVT, ROM firmware, Console, debugger, or
boot behavior.

## S3: Regression And Closure

Run the focused ordinary-INT smoke, `current-gates-gcc`, and the registered
current CTest matrix. Confirm source has no portal API, storage, registration,
or closure-gate residue. Only this final subtask produces
`build/output/nxvm_0_5_0215.exe`.

## Rules And Stop Conditions

Applicable rules: core remains product-neutral; one CPU execution path; no
profile-private bypass; no second interrupt dispatcher; retain the existing
NXVM Console, debugger, and full-PC boot behavior. Stop if a VM, VDM, or
profile production consumer is found, ordinary guest IVT delivery regresses,
or removal requires a product-specific core exception.

## Result

Completed. The pre-change source audit found no portal registration, freeze,
or dispatch consumer below `src/vm`, `src/vdm`, or either product's profiles.
The core public API, execution-context storage, registration/freeze/dispatch
helpers, and `INT` decoder bypass are removed. The portal-only authority smoke
and closure gate are also removed.

`core-machine-int-ivt-smoke` now proves ordinary `CD imm8` IVT delivery for
both 8086 and 80386 profiles. `current-gates-gcc` passes and the registered
current-gate CTest matrix passes 51/51 tests, including DOS boot, Console,
and debugger coverage. The developer artifact is
`build/output/nxvm_0_5_0215.exe`, SHA-256
`BF2AE72415C9F4B61B3C80DC7B43A4500D66B5186A6C343F605CDA67B2B05EE7`.
