# M5 T61 S1 Retained Runtime Alias Inventory

## Decision

T61 is inventory-only. No stateful alias is removed in this task.

The remaining legacy names are not second storage copies. They are direct
macros or compatibility spellings over the current live machine/profile objects.
They are still technical debt because they hide ownership and block clean
multi-session or VM/VDM same-process execution.

## Alias Families

| Alias family | Count | Owner | Classification | Cleanup rule |
| --- | ---: | --- | --- | --- |
| `vcpu` | 1853 | `core/machine` CPU | Executor instance debt. Heavily used by register macros, decoder, debugger adapters, and firmware handlers. | Defer until CPU executor APIs take an explicit CPU/context pointer. |
| `vcpuins` | 3064 | `core/machine` CPU instruction state | Executor instance debt. It is the current instruction decoder/execution scratch object. | Defer until instruction helpers receive explicit execution context. |
| `vram`, `vram*` helpers | 25 | `core/machine` memory | Current-object compatibility alias plus real-mode helper macros. | Defer until memory helpers accept explicit memory/context. |
| `vport` | 95 | `core/machine` port bus | Current-object compatibility alias used by port-device callbacks. | Defer until port handlers receive explicit bus/context. |
| `vpic1`, `vpic2` | 47 | `core/machine` PIC | Current-object compatibility aliases over composition-owned PIC storage. | Defer until interrupt controller calls are context-passing. |
| `vpit` | 132 | `core/machine` PIT | Current-object compatibility alias and helper macro names. | Defer until timer/device loop receives explicit machine context. |
| `vdma1`, `vdma2`, `vlatch` | 136 | `core/machine` DMA | Current-object compatibility aliases over DMA storage. | Defer until DMA device registration and transfers are context-passing. |
| `vkbc` | 3 | `core/machine` KBC | Low-volume compatibility alias. | Candidate for a future low-risk cleanup only after direct call sites are checked. |
| `vvadp` | 37 | `core/machine` video adapter | Current-object compatibility alias used by display firmware and authority tests. | Defer until display firmware uses explicit adapter/context. |
| `vcmos` | 25 | `vm/machine` CMOS | VM current-object compatibility alias. | Defer until VM profile firmware receives explicit CMOS provider/context. |
| `vfdd`, `vfdc`, `vhdd` | 408 | `vm/machine` disk media/controllers | VM current-object compatibility aliases used by disk lifecycle, BIOS, and block provider paths. | Defer until VM block/media providers are context-passing. |
| `vhdc` | 0 | `vm/machine` HDC | Retained stateless device name has no runtime alias use after earlier cleanup. | No cleanup needed unless file/API naming is revisited. |
| `vdebug` | 51 | `vm/machine` debug state | VM current-object compatibility alias used by retained debugger behavior. | Defer until debugger backend is fully session-bound. |
| `vbios` | 36 | `vm/profile/default_profile` BIOS | Default-profile current-object compatibility alias. | Defer until firmware composition passes explicit BIOS storage/context. |
| `qdxTable` | 12 | `vm/profile/default_profile` interrupt dispatcher | Default-profile current-object convenience macro. | Defer with BIOS/QDX context-passing cleanup. |
| `qdcga`, `qddisk`, `qdkeyb` filenames/symbol roots | 14 | `vm/profile/default_profile` firmware services | Historical default-profile service names, not separate live storage aliases. | Leave until profile firmware naming is redesigned as a group. |

Counts are from `rg -o` over `src`, `tests`, and `CMakeLists.txt` during T61.

## T62 Eligibility

Only aliases that are both low-volume and not part of current-object state
authority are eligible for T62. The scan finds no broad stateful cleanup that is
safe inside the second-pass naming window. `vkbc` is small, but it still names a
current core machine object and should not be removed opportunistically without
checking KBC/device call order.

T62 should therefore either remove a narrowly proven harmless spelling or
record an explicit no-cleanup decision.

## Verification

- T61 performs no source alias removal.
- `build/output/nxvm_0_5_0061.exe` is the task artifact.
- Retained Console smoke and source-DAG gates remain required.
