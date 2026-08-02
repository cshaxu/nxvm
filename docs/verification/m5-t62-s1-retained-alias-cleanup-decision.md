# M5 T62 S1 Retained Alias Cleanup Decision

## Decision

No retained runtime alias is removed in T62.

The T61 inventory shows that the remaining aliases are either current-object
state accessors or firmware/service spellings tied to retained execution,
BIOS/POST, debugger, media, or device callback order. Removing them inside the
M5 naming window would not only rename symbols; it would start the explicit
context-passing and multi-session work that belongs to a future state-authority
design task.

## Deferred Cleanup Boundaries

- `vcpu` and `vcpuins` wait for explicit CPU executor context.
- `vram` and `vport` wait for explicit memory and port-bus context.
- PIC/PIT/DMA/KBC/VADP aliases wait for context-passing device callbacks.
- VM media, CMOS, and debug aliases wait for VM machine/session-bound providers.
- Default-profile BIOS/QDX/CGA/disk/keyboard spellings wait for a profile
  firmware naming pass with explicit firmware context.

## Guardrail

Until that future task begins, remaining aliases are accepted only as direct
current-object compatibility accessors. They must not become new storage,
caches, second reset paths, or synchronization bridges.

## Verification

- T62 performs no alias source rewrite.
- `build/output/nxvm_0_5_0062.exe` is the task artifact.
- Retained Console smoke and source-DAG gates remain required.
