# M5 T386: DeskPro Model-40 Selected-Device Functional Closure

## Task Record

T386 follows the closed Model-40 profile and CPU audits. It owns complete
functional device composition for the selected original DeskPro 386 Model 40
before its later board-timing work, but does not make an L3 decision.

## Accepted Progress

### S1: Capability And Owner Allocation

S1 maps every selected platform, input, storage, display and firmware-boundary
row to the current source. It proves that generic PC/AT, ATA/HDC, IBM EGA and
IBM MFM paths cannot be used as Model-40 substitutes. The detailed
[functional ledger](../etc/evidence/t386-s1-deskpro-functional-ledger.md)
allocates six dependency-ordered implementation/replay units and preserves
the source-policy boundary for the external Rev-E ROM.

Coordinator review accepted S1 against the owner-selected Model-40 baseline,
the closed T384/T385 transfers, the current source graph and the S1 exit
criteria. The ledger accounts for every selected functional row and identifies
the earliest owner without converting a generic device into a Compaq device.
Documentation governance and actual-change review passed.

## Open Task Boundary

T386 remains open. Its next continuation must define the Model-40 profile
carrier and common-device composition boundary without exposing a partial
runnable profile or selecting/embedding a ROM. Second-PIT, platform
NMI/reset, Compaq storage, and the small Compaq EGA personality remain
separate bounded implementation receivers. IBM EGA stays outside this task's
display scope and with the product-device L3 closure.

### S2: Carrier And Non-Runnable Publication Contract

S2 traces the current descriptor through selection, materialization, device
composition, media and default firmware. It establishes that the existing
carrier is deliberately the default-PC/AT or Model-339 shape, not a generic
host for Model-40 facts: ATA HDC, generated firmware and generic EGA are
structural false substitutes. The accepted [carrier contract](../etc/evidence/t386-s2-model40-carrier-contract.md)
therefore keeps Model-40 private until S3--S6 close the selected receivers.

Coordinator review accepted S2 against the T386 S1 ledger and the actual
profile/session source graph. It confirms that the present descriptor is a
closed default-PC/AT/Model-339 mechanism, not a neutral extension point, and
that its generic firmware/ATA/EGA coupling makes early Model-40 publication
false. Documentation governance and actual-change review passed.
### S3: Shared Optional Second 8254

S3 adds one shared-machine optional 8254 topology through the existing PIT
mechanism, with separate `48h-4Bh` state/ports/lifetime and no output consumer.
The [S3 evidence](../etc/evidence/t386-s3-second-pit-owner.md) retains its
focused isolation proof and full current-gate result. Failsafe/NMI behavior,
firmware binding and board timing transfer unchanged to S4/later work.

Coordinator review accepted S3 against the selected Model-40 `48h-4Bh` fact,
actual core-machine lifecycle and the focused/full-gate evidence. The new
mechanism has one shared PIT owner and no output consumer; it neither chooses a
profile nor leaks an IRQ/NMI route. Documentation governance and actual-change
review passed.
### S21: Current Functional Matrix And Profile Transfer

S21 replaces the obsolete S15 next-step view with the accepted
[S21 current matrix](../etc/evidence/t386-s21-model40-current-functional-matrix.md).
It receives four bounded functional clusters: D4/platform control, selected
1.2 MB FDC/media logic, the fitted 40 MB controller/startup-media route, and
CECG behavior. It retains physical media and DeskPro timing as separate
boundaries. S20 is recorded truthfully as a fixed backbone, not generic variant
selection; the later queued current-product profile-capability candidate owns
the cross-machine allowed-variant contract.

Coordinator review accepted P1/P2 after removal of unrelated encoding noise.
Documentation governance, link/index checks and actual-change review pass; no
runtime source, asset, artifact or L3 claim changes. T386 remains open for its
four functional receivers.

### S22: D4 Memory Parity Diagnostic And IOCHK

S22 implements the source-bounded Model-40 D4 parity diagnostic and IOCHK
mechanism. Core publishes one location-bearing parity event and invokes its
write observer only for ordinary backing-RAM writes; the Model-40 D4 carrier
latches byte lanes and the existing D4 platform owner publishes IOCHK/NMI.
The accepted [S22 evidence](../etc/evidence/t386-s22-d4-parity-diagnostic.md)
records the lane, mask/release, control/reset and planar-isolation regressions,
the 276-test serial gate and the rebuilt `vm-0-5-0389` artifact.

Coordinator review accepted P1 `edaf4a39` against the S22 packet, source-bound
D4 contract, Core/VM ownership rules and actual diff. It confirms no test-only
production operation, IBM parity fork, firmware/media import, timing claim or
L3 claim. D4-SKEY/reset arbitration and the remaining FDC, fixed-disk and CECG
functional receivers remain in T386 before its later board timing work.
