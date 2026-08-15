# Compaq DeskPro 386 Model 40 80386 L3 Closure Context

## Purpose

This shared context binds the five ordered DeskPro candidates: Model-40
profile/capability audit, 80386 CPU closure, selected-device functional
closure, board bus/device timing closure, and the final DeskPro L3 audit.
Together they establish the 1986 original Compaq DeskPro 386 Model 40
(DeskPro 386/16) baseline after the final 5170 audit and before the 8088/XT
path. It is not itself a candidate and does not allocate a numeric task.

## Required scope

All five candidates retain the 1986 original Model 40/DeskPro 386/16 identity,
one exact board/revision, 80386 clock, chipset, memory, ROM, display/storage
selection and project-owned probe corpus; "DeskPro 386" alone is not a timing
contract. The profile audit owns selection and gap classification, the CPU
candidate owns instruction/state evidence, the functional candidate owns device
state completeness, the board candidate owns availability/service timing, and
the audit reconciles rather than repairs them. Primary Compaq, Intel and device
documentation plus project-owned probes are required; 86Box, MAME and PCjs may
only cross-check a primary-constrained range or an explicitly recorded
measurement contract.

## Non-goals and stop conditions

No unspecified DeskPro revision, 486+ inference, VME/PVI or x87 numeric claim,
universal 80386 PC timing, copied emulator code, host-time coupling, Windows
claim, or firmware/media import. Each candidate stops at its own boundary:
missing profile evidence returns to the profile audit, functional gaps to the
earliest device owner, CPU evidence to the CPU candidate, board evidence to
the board candidate, and reproduced integration defects to their earlier
owner.

## Evidence standard

Require exact profile provenance across all five candidates and, collectively,
the CPU/bus/device/NMI source ledger, functional-completeness matrix,
source-to-model or reference-exhausted timing table, paging/IRQ/DMA contention
and reset replay, project-owned firmware/DOS probes, current-gate evidence and
a closure matrix that keeps this board distinct from the PC/XT 5160-268 and
5170.
