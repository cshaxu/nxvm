# Td S139 Controller Manual Inventory Admission

## Scope

The owner approved one unnumbered Queue candidate ahead of the eight existing
controller/device implementation candidates. It is limited to original-source
admission and implementation-gap inventory; no manual was acquired, no
third-party material was imported, and no runtime code changed.

## Result

The candidate requires exactly two durable checklists for each of PIC 8259A,
DMA 8237A, PIT 8254, RTC/CMOS, KBC 8042/NMI, uPD765 FDC plus logical media,
VADP, and HDC/ATA: source/function/timing evidence and current implementation
gaps. Each row must be L3, L2, or blocked, with every nonconforming row owned
by one later controller candidate. HDC remains explicitly blocked unless an
ATA/IDE source and selected adapter/media contract are admitted.

## Review And Transfer

The Queue order is now inventory, PIC, DMA, PIT, RTC, KBC, FDC/media, VADP,
HDC/ATA, then integration. The shared controller program records this singular
prerequisite, preventing eight parallel research paths or a numeric-task
reservation. The later candidate must apply the source policy before research
and retain non-redistributable sources outside the repository.

## Verification

`Verify-DocumentationGovernance.ps1 -RepositoryRoot .` and `git diff --check`
pass at Td closure. Coordinator review confirms one candidate link, complete
eight-unit coverage, no runtime-source change, no machine-local path, and no
duplicate research candidate.
