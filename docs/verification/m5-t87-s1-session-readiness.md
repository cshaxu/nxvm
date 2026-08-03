# M5 T87 S1: Session Readiness Inventory

## Result

T87 establishes the authoritative module checklist and T88--T95 closure order.
It corrects the `nxvm-current-gcc` preset to the latest verified runnable
artifact (`nxvm-0-5-0086`) and removes the obsolete authority-closure target
from the standard gate preset.

`VerifySessionReadiness.ps1` inventories mutable `static` declarations.
Every detected source file must be classified in
`tools/session-readiness-state.tsv`; unknown or stale entries fail the gate.

## Verification

- `cmake --build --preset nxvm-current-gcc`
- `cmake --build --preset nxvm-current-gates-gcc`

T87 changes governance and build verification only. It creates no runnable
artifact.
