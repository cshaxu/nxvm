# M7 T44 S3 Final Closure Audit

Owner accepts S2 and requests task closure on 2026-10-06. S3 is final
reconciliation only: no runnable change, new optimization or new speed claim.
Reference is accepted S2 source b22a3bb89 and governance 32f7ed0f8.

## Requirement To Proof

| Requirement / candidate | Final disposition and proof |
| --- | --- |
| Fixed pre-edit baseline and method | S1 pins source/compiler/flags/inputs, warmup/frame/input budgets and bounded reference probes; no production profiler. |
| Palette optimization | Accepted S1: frame-local 512-entry lookup, original RGB deduplication/order/overflow; exact pixels and palette tests, six-ROM proof. |
| Mapper optimization | Accepted S2: constant-domain MMC1/UxROM simplification and MMC3-only offsets; full capacity/value/mode/window proof and snapshot reconstruction. |
| Existing restored CNROM bounds gap | Fixed S2; all invalid bank bytes across admitted capacities reject before live mutation. |
| Guest timing, bus, A12, IRQ and PCM | Original scheduling/event code unchanged; existing PPU/mapper/CPU/APU tests plus paired cycle/instruction/frame/PCM/serialized-state identities pass. |
| Both output modes and widths | Six external ROMs x graphics/text x x64/x86, 24 routes, identical applicable before/after outputs; native Window/Console integration and owner acceptance. |
| Reset, pause/resume, input and snapshots | Complete final MyNES suite covers lifecycle/native input; mapped reset/restore and v3 continuation tests remain passing. |
| Code-owned blank/repeated/emphasis/overflow scenes | Palette unit's eight scenes and independent exact comparison; no external fixture in new units. |
| Scrolling/sprite/IRQ preservation | Existing PPU and visual/motion/mapper-IRQ integration are included in final suites; no tile or event-suppression optimization. Not an all-game compatibility claim. |
| IRQ aggregation / PPU condition cleanup | Not admitted after owner-selected prioritization; small/uncertain ROI and sensitive event boundaries. Original production paths remain intentionally unchanged. |
| Decoded tiles / broad CPU/APU cache ideas | Not selected: no owned measured candidate justifies additional lifetime/invalidation machinery. Not an unresolved repair or implicit future implementation commitment. |
| Native paint / per-device performance attribution | Explicitly unmeasured, not accepted as a completion claim. Further profiling needs its own owner-admitted MyNES/Shared scope; no added runtime API or renderer fork. |
| Nesticle comparison / source import | Optional exact-version comparison not performed; provenance permits architectural research, not copying. No third-party code or protected binary imported. |

This closes the owner-approved two-candidate performance task, not all possible
emulator optimization or all original exploratory instrumentation ideas.
No unimplemented source defect found by T44 remains hidden in this disposition.

## Final Source And Artifact Check

Actual final production diff from fc0d31623 was inspected: driver +9 lines,
cartridge.c -4, cartridge.h +5, snapshot.c +2, total net +12. One frame converter
and one cartridge mapping owner remain. Additional state is a 1 KiB local frame
lookup and 48 bytes of private mapper offsets, not a second guest clock/state.
The old per-read MMC3 formula is gone; original arithmetic remains only as a
code-owned independent test oracle. No new public ABI, engine, thread or cache
framework. Derived offsets are excluded from the existing v3 snapshot format.

All accepted final executable inputs are byte-identical to b22a3bb89: source,
tests, build configuration, tools, deployment and snapshots. Existing final
LastTest logs are checked: 57 passed / zero failed for each width. S2 completed
45 unit/static plus 12 integration cases once per width (54.01 / 73.54 seconds).
S3 reuses these exact-input results, not an older changed-source result; repeating
the same full suite or relinking unchanged artifacts would add no coverage.

Current pair, optimized stripped Release, same hashes as owner-tested S2:

- x64: 12E7686B0DF97E4C0954032AC7BC5E60C3D06B22C5A7618045FF97FA4D959A68.
- x86: 418B1B161938E2FCAC4D957D0AD44C811549DB0674DF5D0F6438AEDA2F7A9357.
- INI: 198846D0D4EB3C7CAB1357992D7AB3441BECA676706AF1BAB0DAF507FB6BCC15.

Six current external ROM hashes match the S1 input register. Owner snapshot
is unchanged. Only this pair remains in assets/mynes; no stale pair or new
artifact version is generated for documentation-only closure.

Shared production/test corpora and adjacent NXVM Apps are unchanged throughout
T44. Its sole Shared change is the separately approved two MyNES root preset
targets in ae15f9129. No manifest rebuild or receiving-App rebuild is needed.

## Performance Claim And Debt Boundary

Retain separate stage reports: S1 conversion observations around 43-85% reduction;
S2 whole-probe CPU totals observed 6.48% x64 / 2.16% x86 reduction. Scope and noise
remain explicit; no compounded percentage, universal per-game improvement, native
FPS, physical timing or Nesticle-equivalent claim. Small negative/neutral routes
remain visible in S2's table rather than removed from the result set.

All five pre-existing TODO entries retain their owners and admission paths:
atomic save replacement, asynchronous synchronization-failure reporting,
executable-path abstraction, stream EOF/trailing-byte completion, and the Shared
modal-test nondeterminism investigation. They are outside this bounded optimization
task, were not introduced here and are not declared fixed by closing T44.

Documentation/link/diff checks and actual archive review are required before
acceptance. Proposal moves to history; Queue remains empty. Bounded results are
ignored evidence, not product inputs. Warm compiler caches remain reusable;
expired task probe binaries/CSV outputs can be removed after recording this audit.

## Executor Result

All admitted batches have a direct final disposition. Recommend T44 closure
without further source changes or performance experiments. Owner acceptance of
S1/S2 is recorded; final pushed P still requires coordinator immutable-diff review
and a pure governance P to close S3/T44.
