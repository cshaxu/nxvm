# M5 T366: Bus-Timed PC/AT Operation

## Task Record

T366 follows the completed instruction, selected-device, and NMI ownership
audits. It may admit a bounded PC/AT bus-availability model only after one
documented profile/corpus identifies its routes, availability contract and
excluded hardware; it does not itself imply physical cycle exactness.

## Active Progress

### S1: Selected-profile bus source and route inventory

S1 inventories the current transaction, CPU memory/I/O, DMA, PIC, FDC and HDC
routes against a selected PC/AT hardware/corpus contract. It records whether a
safe next availability-contract S is admissible, without allocating wait states
or changing runtime behavior.

S1 is accepted at `098a146a`. Its
[profile and route inventory](../etc/evidence/t366-s1-pcat-bus-profile-route-inventory.md)
finds no selected physical target or reproducible corpus from which to allocate
availability state. A later S requires the owner decision recorded there; it
must retain the T365 NMI receiver and may not use reference-model code as a
timing authority.

### S2: Exact 5170 profile and probe-corpus lock

S2 selects a field-configured IBM 5170 Model 339 Type 3 at 8 MHz with 512 KB
system-board RAM, Rev.3 ROM slot, 101-key keyboard, IBM CGA and a documented
TEAC 1.44 MB drive. The Model 339 MFM fixed disk and serial/parallel hardware
are deliberately unselected; the drive is a documented field upgrade, not a
factory Model 339 claim. Its [profile lock](../etc/evidence/t366-s2-5170-profile-lock.md)
corrects the stale 360 KB drive wording, names a repository-owned future probe
corpus and MFM/ST-506 TODO receiver, and narrows the next S to the board
memory-parity NMI lifecycle. It allocates no timing or runtime behavior.

### S3: Model 339 planar-parity NMI lifecycle

S3 adds an opt-in core owner for the selected system-board RAM-parity source.
Only `61h` is accepted; its bit 7 reports the latch and bit 2 clears/re-enables
the source. CMOS `70h` remains mask-only. The [lifecycle evidence](../etc/evidence/t366-s3-planar-parity-nmi.md)
records focused producer proof and the retained cross-mode NMI consumer proof.
It does not enable the generic default PC/AT profile, select I/O-channel check,
or allocate timing.

### S4: Model 339 planar memory and parity binding

S4 makes 512 KB an admissible initial memory configuration and binds the S3
source to the one shared physical RAM route. Its [memory/parity evidence](../etc/evidence/t366-s4-planar-memory-parity.md)
records write-establish/read-check behavior, the low-memory high-ROM boundary,
reset and reconfiguration disposition. S4 is accepted at `ff5fbb53`; it adds
no default-profile identity, I/O-channel check or timing allocation.

### S5: Model 339 selectable session composition

S5 adds one explicit Model 339 session selection, carrying the selected 80286,
512 KB planar parity and no-fixed-disk topology through the existing
composition lifecycle. Its [composition evidence](../etc/evidence/t366-s5-model-339-composition.md)
records the suppressed ATA/HDC controller, media, firmware and console paths
and the retained default profile. It does not select the IBM ROM, exact CGA
surface, MFM/ST-506 or any timing contract. S5 is accepted at `fd920773`.

### S6: Model 339 CGA display topology

S6 selects the existing CGA VADP surface and owner-local VRAM for the 512 KB
Model 339 while making EGA registration optional for the retained default
profile. Its [topology evidence](../etc/evidence/t366-s6-model-339-cga-topology.md)
records the focused isolation proof and remaining display/timing transfers.
S6 is accepted at `dd464d74`; IBM ROM identity, exhaustive CGA timing, the
1.44 MB field-upgrade FDC topology, MFM/ST-506 and bus timing remain later
transfers.

### S7: Model 339 firmware and field-upgrade FDC topology

S7 declares the non-vendor Rev.3 firmware slot and the selected drive-A field
upgrade while proving the existing FDC port, IRQ6 and DMA2 composition. Its
[topology evidence](../etc/evidence/t366-s7-model-339-firmware-fdc-topology.md)
records the source boundary and retained timing transfers.
S7 is accepted at `553cd6d6`; firmware behavior and FDC service timing remain
unallocated and transfer to later T366 work.

### S8: FDC service-timing non-admission

S8 finds that the retained readiness tick has no uPD765 clock-domain conversion.
Its [non-admission evidence](../etc/evidence/t366-s8-fdc-service-timing-nonadmission.md)
transfers all numeric FDC service timing to the next T366 S.
S8 is accepted at `5cf16ce4`.

### S9: Physical-time non-admission

S9 finds that successful unallocated 80286 cost publishers prevent `elapsed_ticks`
from denoting a physical 8 MHz clock. Its [non-admission evidence](../etc/evidence/t366-s9-physical-time-nonadmission.md)
transfers that prerequisite to complete CPU timing closure.
S9 is accepted at `16609f35`.

### S10: 80286 unallocated-publisher audit

S10 inventories the direct prefix and default successful-retirement one-tick
publishers, and adds a static check that every control/stack lookup form exists
in the 80286 ledger. Its [inventory evidence](../etc/evidence/t366-s10-80286-unallocated-publisher-inventory.md)
assigns the remaining fallback receiver to source-backed CPU-ledger closure;
it makes no timing allocation or physical-time claim. S10 is accepted at
`cc4c9987`.

### S11: 80286 Group-2 register shift/rotate timing

S11 gives valid register-direct `D0`/`D1` implicit-count-one Group-2 forms
their two-clock 80286 source-table value while preserving memory, count-based,
and `/6` undefined boundaries. Its [timing evidence](../etc/evidence/t366-s11-80286-group2-register-timing.md)
records the sweep and receiver transfer. S11 is accepted at `8190857f`.

### S12: 80286 Group-2 direct-memory timing

S12 gives valid direct-address-memory `D0`/`D1` implicit-count-one Group-2
forms their seven-clock source-table value without treating the `7*`
effective-address notation as a universal memory constant. Its [timing evidence](../etc/evidence/t366-s12-80286-group2-direct-memory-timing.md)
records the retained forms and receiver transfer. S12 is accepted at `5fe66bd8`.

### S13: 80286 Group-2 effective-address memory timing

S13 extends the accepted `D0`/`D1` implicit-count-one memory timing to every
valid memory addressing form through the existing Appendix-B EA helper. Its
[timing evidence](../etc/evidence/t366-s13-80286-group2-ea-memory-timing.md)
records complete memory coverage and the remaining count-form receiver. S13 is
accepted at `56f21513`.

### S14: 80286 Group-2 count-dependent timing

S14 closes valid `C0`--`C1` and `D2`--`D3` Group-2 forms with masked count
capture and the `5+n`/`8+n+EA` source formulas, while retaining `/6` as an
undefined opcode. Its [timing evidence](../etc/evidence/t366-s14-80286-group2-count-timing.md)
records the decoder-capture boundary and sweep. S14 is accepted at `9c437f8b`.

### S15: 80286 FLAGS-control timing

S15 assigns source-backed fixed costs to successful unprefixed
`CMC/STC/CLD/STD/CLI/STI` through the existing retirement owner. Its [timing evidence](../etc/evidence/t366-s15-80286-flags-control-timing.md)
retains CLC/HLT and prefix, x87, and delivery boundaries. S15 is accepted at
`9e6813b9`.

### S16: 80286 XLAT timing

S16 gives successful unprefixed `D7 XLAT` its five-clock 80286 source-table
cost through the existing retirement publisher. Its [timing evidence](../etc/evidence/t366-s16-80286-xlat-timing.md)
records the source, semantic/publication proof and boundary transfers. S16 is
accepted at `ac5d6ec6`.

### S17: 80286 LAHF/SAHF timing

S17 gives successful unprefixed `9E SAHF` and `9F LAHF` their fixed two-clock
80286 source-table costs through the existing retirement publisher. P2 adds
the packet-required focused AH/FLAGS proof. Its [timing evidence](../etc/evidence/t366-s17-80286-lahf-sahf-timing.md)
records the correction, source and boundary transfers. S17 is accepted at
`7d9e27be`.

### S18: 80286 segment-register store timing

S18 gives successful unprefixed legal register-direct `8C MOV Sreg,r16` its
two-clock source-table cost through the existing retirement publisher. Its
[timing evidence](../etc/evidence/t366-s18-80286-sreg-store-timing.md) records
the legal-encoding guard, selector-transfer proof and retained memory/load
boundaries. S18 is accepted at `842848b8`.

### S19: 80286 segment-register memory-store timing

S19 extends `8C MOV Sreg,m16` to successful unprefixed legal memory forms
using the source-backed `3 + EA + odd-word` construction. Its [timing evidence](../etc/evidence/t366-s19-80286-sreg-store-memory-timing.md)
records direct/indexed parity proof and retained `8E`/invalid boundaries. S19
is accepted at `ad1ed794`.

### S20: 80286 real-mode segment-register load timing

S20 gives successful unprefixed real-mode legal `8E MOV Sreg,r/m16` forms the
source-backed register `2` and memory `5 + EA + odd-word` construction. Its
[timing evidence](../etc/evidence/t366-s20-80286-sreg-load-real-timing.md)
records all legal target, direct/indexed parity proof and the retained
protected-mode descriptor-load boundary. S20 is accepted at `25d3a773`.

### S21: 80286 protected-mode segment-register load timing

S21 gives successful unprefixed protected-mode legal `8E MOV Sreg,r/m16` forms
the source-backed register `17` and memory `19 + EA + odd-word` construction.
Its [timing evidence](../etc/evidence/t366-s21-80286-sreg-load-protected-timing.md)
records the isolated descriptor context and retained fault boundary. S21 is
accepted at `e71d2262`.

### S22: 80286 LES/LDS timing

S22 assigns the Appendix-B fixed seven-clock cost to successful unprefixed
memory-form `C4/C5 LES/LDS r16,m16` retirement. Its
[timing evidence](../etc/evidence/t366-s22-80286-les-lds-timing.md) proves
real/protected direct and indexed odd/even pointer cases and retains register,
prefix, selector/operand-fault, delivery, bus and physical-time boundaries.
S22 is accepted at `1d3391bb`.

### S23: 80286 segment-register stack timing

S23 assigns Appendix-B fixed 3/5-clock rows to successful unprefixed legacy
segment-register `PUSH`/`POP` forms. Its
[timing evidence](../etc/evidence/t366-s23-80286-sreg-stack-timing.md) proves
all seven real/protected forms with stack-image/selector results and retains
prefix, selector/stack-fault, POP-SS-shadow, delivery, bus and physical-time
boundaries. S23 is accepted at `46190502`.

### S24: 80286 BOUND timing

S24 assigns the Appendix-B fixed thirteen-clock cost to successful unprefixed
memory-form `62 /r BOUND r16,m16&m16` retirement. Its
[timing evidence](../etc/evidence/t366-s24-80286-bound-timing.md) proves
in-range real/protected direct and indexed cases, including index/FLAGS
preservation, and retains register `#UD`, out-of-range `#BR`, operand-fault,
prefix, delivery, bus and physical-time boundaries.
S24 is accepted at `03869c16`.

### S25: 80286 ARPL timing

S25 assigns the Appendix-B/errata fixed protected-mode register `10` and
memory `11` clocks to successful unprefixed `63 /r ARPL r/m16,r16`
retirement. Its [timing evidence](../etc/evidence/t366-s25-80286-arpl-timing.md)
proves register/direct/indexed adjustment outcomes and retains real-mode,
prefix, null-selector, fault, delivery, bus and physical-time boundaries.
S25 is accepted at `f2e90047`.

### S26: 80286 VERR/VERW timing

S26 assigns Appendix-B fixed register `14` and memory `16` clocks to successful
unprefixed protected `0F 00 /4,/5 VERR/VERW r/m16` retirement. Its
[timing evidence](../etc/evidence/t366-s26-80286-verr-verw-timing.md) retains
mode, prefix, memory-fault, delivery, bus and physical-time boundaries.
S26 is accepted at `42488be5`.

### S27: 80286 LAR timing

S27 assigns the Appendix-B fixed register `14` and memory `16` clocks to
successful unprefixed protected `0F 02 /r LAR r16,r/m16` retirement.  Its
[timing evidence](../etc/evidence/t366-s27-80286-lar-timing.md) retains
mode, prefix, descriptor, fault, delivery, bus and physical-time boundaries.
S27 is accepted at `79ef50bb`.

### S28: 80286 LSL timing

S28 assigns the Appendix-B fixed register `14` and memory `16` clocks to
successful unprefixed protected `0F 03 /r LSL r16,r/m16` retirement.  Its
[timing evidence](../etc/evidence/t366-s28-80286-lsl-timing.md) retains
mode, prefix, descriptor, fault, delivery, bus and physical-time boundaries.
S28 is accepted at `00a0db54`.

### S29: 80286 SMSW timing

S29 assigns Appendix-B fixed register `2` and memory `3` clocks to successful
unprefixed `0F 01 /4 SMSW r/m16` retirement in its documented real/protected
modes.  Its [timing evidence](../etc/evidence/t366-s29-80286-smsw-timing.md)
retains prefix, fault, delivery, bus and physical-time boundaries.
S29 is accepted at `d65d92d8`.

### S30: 80286 SLDT/STR timing

S30 assigns Appendix-B fixed register `2` and memory `3` clocks to successful
unprefixed protected `0F 00 /0,/1 SLDT/STR r/m16` retirement.  Its
[timing evidence](../etc/evidence/t366-s30-80286-sldt-str-timing.md) retains
mode, prefix, fault, delivery, bus and physical-time boundaries.
S30 is accepted at `86f19062`.
