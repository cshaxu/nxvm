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
