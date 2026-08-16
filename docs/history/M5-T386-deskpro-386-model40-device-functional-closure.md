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