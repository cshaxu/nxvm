# T493 S5 IBM 5160 CGA VADP Closure Audit

`M5:T493:S5:CGA-CLOSURE:OK`

T493 closes the finite C1--C33 CGA universe frozen in
[List 1](t493-s2-cga-function-timing-list-1.md) and mapped in
[List 2](t493-s3-cga-current-code-gap-list-2.md).

| Ledger batch | Final disposition |
| --- | --- |
| C1--C17 | Retained at the sole VADP `3D4h`/`3D5h` CRTC bank and copied-text consumers; selected IBM access classes and current source-qualified logical use are covered. |
| C18--C19 | Retained at VADP mode/color write state, selected graphics snapshot and raw text-attribute copy. |
| C20--C21 | Closed by S4: generic CGA status bit 1 now comes only from the VADP CGA latch; `3DBh`, `3DCh` and reset are focused-proved. Bit 2 and physical status phase remain source-defined external boundaries. |
| C22--C23 | Retained one VADP guest programming and B8000/16 KiB memory-provider route. |
| C24, C31 | IBM names the relationship but supplies no numeric physical raster/contended-memory basis; they remain explicit non-estimated boundaries, not unconsumed implementation defects. |
| C25--C29 | Retained VADP text/graphics capture: cell attributes, 320 by 200 packed PELs, odd/even banks and palette sets are covered by the focused and system routes. |
| C30 | Closed by S4: 640 by 200 VADP capture now uses binary black/white output; `3D9h` no longer causes an irrelevant high-resolution frame update. |
| C32--C33 | Retained one-way VADP snapshot-to-VM copied presentation route and deterministic project reset. IBM power-on image remains unclaimed. |

No C1--C33 implementation row remains. The retained boundaries are not TODO
items: they are excluded physical/input facts that the accepted source does not
define and this task is prohibited from estimating.

## Result simplicity

The only production state added is a standard-CGA light-pen latch owned by
VADP. It deliberately does not reuse the Compaq latch, so distinct cards do
not share mutable state. The high-resolution repair removes the obsolete color
comparison/publication path. Across S4's tracked code/test paths the result is
37 added and 18 removed lines, net +19; the complete T493 runtime repair is
therefore one owner-local state relation plus subtraction, with no new route,
wrapper, profile setting or renderer state.

## Verification and artifact

- Focused VADP/XT/display CTest selection: 5/5 passed.
- Compaq isolation selection: 7/7 passed.
- Full serial CTest: 300/300 passed in 108.44 seconds.
- Documentation governance and `git diff --check` passed.
- Release target `vm-0-5-0493` built with the configured optimized Release
  pre-link verifier. The copied stripped developer artifact is
  `build/output/nxvm_0_5_0493.exe`, 1,238,877 bytes, SHA-256
  `8E5C3E5261E4D5809B28C0863DF4D090F15F210DC75DA581C8F10CB8D8E5E9A4`.
  `objdump -h` found no `.debug` section. The admitted runtime debugger remains
  part of the product; only compiler debug information is absent.
