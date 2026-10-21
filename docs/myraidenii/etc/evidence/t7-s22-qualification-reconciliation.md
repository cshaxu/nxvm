# M1 T7 S22 — Qualification-Reconciliation Matrix

## Scope and evidence rule

This is a documentation-only reconciliation of the fixed G01--G12 acceptance
matrix in [the T1 contract](../research/t1-integration-contract.md). It does
not add a guest-state mirror, title detector, ROM trace, screenshot, or a new
execution/input path. Runtime observations and external-reference facts remain
**L2**; containment budgets remain **L1**. No row below is promoted to L3.

## Current disposition

| Checkpoint | Current evidence | Disposition and remaining receiver |
| --- | --- | --- |
| G01 | T1 inventory/contract records; ROM-free Product/Machine inventory tests | Partially covered: the fixed role/size/hash transaction has an owner and rejects malformed inputs. A successful selected-set launch is not a replacement/corruption matrix qualification. |
| G02 | T7 S15 and S21 bounded selected-set runs | Partially covered as L2: both host widths execute their bounded guest budgets without the reported CPU/COP/Board fault. This is not a complete guest-initialization checkpoint. |
| G03 | Core COP/Video unit receivers; T7 S15 changing copied-frame samples | Partially covered as L2: local initialization routes and output progression exist. No observation establishes that the selected ROM completed every required COP/graphics initialization. |
| G04 | T7 S15 copied-frame progression; Video/Sound source limits | Blocked: there is no source-owned title/attract semantic observer, named-scene oracle or nonzero selected-set sound-command proof. A digest change cannot be relabelled as a title screen. |
| G05 | Machine driver mapping unit; T7 S21 KVM `5`/`1` pulse | Electrical ingress is covered as L2: KVM reaches the one Machine driver and Board control owner. Guest acceptance, credits and start-state change remain unobserved. |
| G06 | Machine driver mapping unit | Board key mapping is covered. Host Window/RDP delivery, release/focus behavior and guest-visible movement/fire/bomb behavior remain unqualified. |
| G07 | None | Blocked: no boss/state receiver or qualified gameplay observation exists. |
| G08 | Frozen T1 matrix only | Blocked: the required stage/boss/end signature matrix has deliberately not been invented. |
| G09 | None | Blocked: death, game-over, continue and restart have no lawful selected-set observer. |
| G10 | Emulator/Product/Machine unit coverage | Partially covered by the existing lifecycle owners; no selected-set native Window close/teardown qualification is claimed. |
| G11 | Core Video/Sound units; T7 S15/S21 copied output summaries | Partially covered: the local frame/PCM paths have owners. Named-scene palette/scroll/priority/raster fidelity, sprite blend behavior and audible selected-set PCM remain open. |
| G12 | T7 S21 x64/x86 probe and unit results | Blocked: passing bounded units/probes is not the frozen full acceptance set, and release artifacts are not eligible until G01--G11 are qualified. |

## Receiver decision

The current code has no lawful semantic owner for a `title`, `attract`,
`credit`, or `in-game` flag. Adding one from elapsed time, a frame digest or
private memory would fabricate a second game-state model and violate the
frozen contract.

The earliest concrete unconsumed selected-set hardware input is COPX-D2. T7
S5 established that its image identity is known, while the requester,
index/address rule, result width/order and failure behavior are not. It is a
research blocker, not an implementation receiver: a later packet must
establish that finite Board-owned contract before any COPX input or output is
added. Until then, no probe can honestly claim G04 or G05 success.

## Evidence reviewed and verification

- T1 fixed integration contract and G01--G12 matrix.
- T7 S5 COPX receiver review.
- T7 S15 bounded frame progression.
- T7 S19 bounded Board pulse and T7 S20/S21 Product/Machine input routing.
- Current Machine driver KVM mapping, Core Board control ownership, and
  current Video/Sound ownership boundaries.

The cited evidence records and source owners exist. The current driver maps
`5` and `1` through `deliver_input`, while the Board remains the only
electrical-control owner. No runtime process, ROM/archive, external program,
screenshot or generated artifact was read or retained for this reconciliation.
