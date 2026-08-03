# M5 T143: Hygiene Closure

## Source Ownership

- Deleted `vm/product/media.*`, `vm/product/presentation.*`, their isolated
  smoke targets, and their test-only debugger support. No retained NXVM Console
  path called these models.
- Moved the VDM-minimal-only input event from `core/platform/presentation.h`
  to `vdm/composition/input_event.h` and renamed it
  `vdm_composition_input_event`.
- No `vm_product_media`, `vm_product_presentation`,
  `nxvm_platform_input_event`, or `core/platform/presentation.h` active-source
  reference remains.

## Deferred Work

The four source comments now use `TODO(High|Medium|Low)` and have matching
items in `TODO.md`: CPU naming, protected-mode I/O-map checking, debugger
assembler testing, and PIT read-back support. This task does not implement
those compatibility features.

## Verification

Windows GCC configured and built `nxvm-current-gcc` and
`nxvm-current-gates-gcc`; the retained Console lifecycle gate passed. Additional
runtime checks passed:

```text
M5:T94:S1:VDM-PRESENTATION-ISOLATION:OK
M5:T1:S1:CPU-PROBES:OK
M5:T70:S2:DOS-PROMPT:OK
```

The CPU-probe intentionally logs its invalid-instruction `#UD` vector before
its passing marker. `nxvm_0_5_0143.exe` starts with the `0.5.0143` banner and
has SHA-256 `E5ACDB8EB1A024FEB21FB7361A5F15AC1181F315B437B3AF967C0315CD811F01`.

## Build Hygiene

After verification, the resolved `build/mingw-gcc-x64` root was cleaned while
excluding `build/mingw-gcc-x64/output`. The final count of `.exe`/`.a` files
outside `output/` is zero; 18 verified task executables remain under
`build/output/`.
