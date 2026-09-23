# T386 S7: Private Model 40 Composition

`M5:T386:S7:MODEL40-PRIVATE-COMPOSITION:OK`

`M5:T386:S7:EXTERNAL-ROM-GUARD:OK`

## Delivered Boundary

S7 adds a private VM-only Model 40 construction route. It is not a public
profile, catalog entry, Console command, YAML schema, filesystem loader, or
session-selection value. The route accepts only an owner-supplied in-memory
128 KiB ROM span. It rejects a null or wrong-sized span before any firmware
provider is bound. The focused proof uses one project-owned synthetic byte
array; no external ROM, guest media, path, hash, or binary is read or committed.

The VM composition selects the accepted 80386, 1 MiB, no-FPU, second 8254 at
48h, D4 platform, Compaq EGA personality, FDC DMA2/IRQ6, Compaq WD 40 MB
controller configuration, RTC/CMOS, and keyboard-only 8042 topology. The core
owns only the reusable 8042 AUX-presence capability. With AUX absent it seals
the command byte, cannot enable AUX through A8h, reports failure for A9h,
discards the D4h data byte, cannot assert IRQ12, and rejects mouse reports.
The Model 40 route remains the VM-side selector of that generic capability.

The external-ROM provider maps the supplied immutable span through the existing
core firmware interface at E0000h--FFFFFh. The focused smoke reads the supplied
synthetic reset-vector byte back through the physical-memory interface. It also
submits a mouse event through the production host-input route and proves that
the session produces no KBC AUX output.

## Regression Repair

The new route exposed two uninitialized local default-VM configuration structs.
Default display and DMA composition now initializes both structs. This preserves
the existing PC/AT and Model 339 profiles and removes a stack-layout-dependent
creation failure; it does not change either profile contract.

## Verification

- `vm-model40-private-composition-s7-smoke` passed, including invalid geometry,
  immutable mapping, selected hardware observations, no-AUX command behavior,
  and native host-input control.
- `core-machine-kbc-controller-smoke` and `core-machine-kbc-aux-port-smoke`
  passed as existing AUX-present controls.
- Serial `ctest -L current-gate --parallel 1` passed 257/257 tests.
- `verify-vm-provider-composition`, `verify-documentation-governance`, and
  `verify-t345-deferred-direct-ownership` passed. T345 now records 144 owner
  tests, 50 mixed/inherited production entries, and 57 exact residual entries.

## Transfers

S7 does not make a runnable or L3 claim. Firmware programming and BIOS-visible
machine policy still require an owner-managed real-ROM admission. Media attach,
physical 40 MB drive semantics, ECC/recovery and timing remain in the existing
Compaq fixed-disk TODO. CECG firmware, board behavior, waits and raster timing
remain in the existing CECG TODO. Public profile selection and any guest-media
loading remain later work.