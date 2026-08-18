# Compaq DeskPro 386 Model 40 80386 L3 Closure Context

## Purpose

This shared context binds the six ordered DeskPro candidates: Model-40
profile/capability audit, 80386 CPU closure, selected-device functional
closure, board-level timing closure, physical-device/firmware timing closure, and the final DeskPro L3 audit. The separate shared 80386 physical-retirement qualification now precedes the board candidate; it is not a Model-40 substitute.
Together they establish the 1986 original Compaq DeskPro 386 Model 40
(DeskPro 386/16) baseline after the final 5170 audit and before the 8088/XT
path. It is not itself a candidate and does not allocate a numeric task.

## Required scope

The six Model-40 candidates retain the 1986 original Model 40/DeskPro 386/16 identity,
one exact board/revision, 80386 clock, chipset, memory, owner-managed Rev-E ROM
constraint, Compaq Enhanced Color Graphics plus Color Monitor, 101-key Enhanced
Keyboard, Compaq Multipurpose Fixed Disk Controller with one 1.2 MB diskette
drive and one 40 MB fixed disk, and project-owned probe corpus. "DeskPro 386"
alone is not a timing contract. The profile audit owns selection and gap
classification, the CPU candidate owns instruction/state evidence, the
functional candidate owns device state completeness, the board candidate owns
availability/service timing, and the audit reconciles rather than repairs them.
Primary Compaq, Intel and device documentation plus project-owned probes are
required; 86Box, MAME and PCjs may only cross-check a primary-constrained range
or an explicitly recorded measurement contract.


## Reference-Derived Extension

When the owner explicitly accepts existing reference simulations because no
physical hardware observation is available, a separately queued DeskPro bridge
may establish a `reference-derived` evidence tier. It consumes primary Compaq
facts for topology and only an exact reproducible DeskPro reference contract
for behavior. It does not revise any completed physical-time conclusion,
permit generic AT substitution, import third-party source/firmware/media, or
claim physical L3. A later independent re-audit must name this tier exactly.
The accepted public Model-40 route is a fixed backbone with a frozen external
ROM manifest. Until the later current-product profile-capability candidate, it
has no public generic `variant` field. T386 functional work must declare its
selected external fixed-disk startup boundary and removable-diskette boundary,
but may neither turn them into a free-form builder nor make a machine-local
variant schema. The post-DeskPro candidate receives the real fixed/allowed/
prohibited facts from this baseline and the Model-339 baseline, then designs
the cross-product contract.

VADP remains a shared core owner. The DeskPro functional candidate owns one
small source-backed Compaq EGA personality/profile contract around that core;
it must not duplicate VADP mechanics or derive defaults from IBM EGA. IBM EGA
remains a distinct current-product capability and is accepted only by the
current-product device-capability L3 closure. No personality's device or timing
evidence transfers to the other.

## Non-goals and stop conditions

No unspecified DeskPro revision, 486+ inference, VME/PVI or x87 numeric claim,
universal 80386 PC timing, copied emulator code, host-time coupling, Windows
claim, or firmware/media import. Each candidate stops at its own boundary:
missing profile evidence returns to the profile audit, functional gaps to the
earliest device owner, CPU evidence to the CPU candidate, board evidence to
the board candidate, and reproduced integration defects to their earlier
owner.

## Evidence standard

Require exact profile provenance across the six Model-40 candidates and, collectively,
the CPU/bus/device/NMI source ledger, functional-completeness matrix,
source-to-model or reference-exhausted timing table, paging/IRQ/DMA contention
and reset replay, project-owned firmware/DOS probes, current-gate evidence and
a closure matrix that keeps this board distinct from the PC/XT 5160-268 and
5170.
