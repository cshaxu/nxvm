# PPU And Clock Completion

## Outcome

Deliver the declared NTSC background/sprite/scroll engine and OAM DMA on one production clock, with a fixture demonstrating moving sprites and scrolling through both presentation routes. The current T12 S3 continuation specifically replaces direct background sampling with timed fetch/shifter state, makes remaining sprite phases explicit, and closes the remaining CPU/PPU status/NMI boundaries.

## Prerequisites And Authority

Interactive dual KVM vertical accepted; consume its clock/device residual ledger. See [M3 design](../etc/m3-design.md), [remediation](../etc/m3-remediation.md), [Architecture](../design/ARCHITECTURE.md), [Coding](../design/CODING.md) and [research provenance](../etc/m3-sources.md). This was an unnumbered candidate; implementation and S allocation required separate admission.

## Complete Scope

Finish all eight registers, latches/buffering/mirrors, scroll increments/copies, background clipping, sprite sizes/flips/priority/zero-hit/overflow, frame odd skip, vblank/NMI races, deterministic power and warm reset, OAM DMA halt/alignment/wrap and run-slice continuation. Keep CPU instruction semantics while removing trace-derived timing and mutable-hook alternate paths.

Remediation batch: Close R07 and remaining R08; maintain R10 deterministic CHR behavior and R11/R12.

## Verification And Convergence

Final G02-G06. Freeze finite register x operation x phase and sprite/status timing tables before implementation. Test 513/514 DMA slots, RMW last write, read side effects and stop responsiveness; partitioned run/step yields the same hardware state. Use primary research checkpoints and original fixtures, not host screenshots as timing proof. Full x64/x86 regression.

The design G ledger and remediation R ledger are the task convergence inventory. At admission expand this package's named batch into finite cases, required contexts, evidence and dispositions; no first-failure chasing or test-count-only closure. Every accepted member needs production-path evidence and a regression owner. Retain all earlier accepted behavior; expose unsupported behavior explicitly.

The accepted S2 checkpoint is `c889739`. It proved resumable OAM DMA, frame motion through both Core presentation routes, selected timing/evaluation state and removal of the mutable clock hook. It deliberately does not claim a background fetch/shifter pipeline, per-dot secondary-OAM and sprite-fetch buses, or all CPU/PPU status/NMI sampling offsets; S3 is their same-T12 receiver.

## Boundaries And Transfer

DMC/APU/PAL and analog/sub-dot qualification remain excluded. Input and native output qualification retain their named owners; no commercial compatibility claim. Lib/Common source and tests remain unchanged. No commercial ROM is committed. Any primary-source contradiction, unsafe lifetime or required shared mutation stops its affected work until resolved in the admitted scope. All applicable owned members must close before acceptance; residuals need the named receiving candidate, and no unresolved required row may disappear at milestone closure.
