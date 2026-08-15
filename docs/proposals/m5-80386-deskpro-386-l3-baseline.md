# Compaq DeskPro 386 80386 L3 Closure Context

## Purpose

This shared context binds the three ordered DeskPro candidates: profile and
80386 CPU closure, board bus/device timing closure, and the final DeskPro L3
audit.  Together they establish one exact DeskPro 386 baseline after the 5170
audit and before the 8088/5150/XT path.  It is not itself a candidate and does
not allocate a numeric task.

## Required scope

All three candidates retain one exact model/revision, 80386 clock, chipset,
memory, ROM, display/storage selection and project-owned probe corpus;
"DeskPro 386" alone is not a timing contract.  The CPU candidate owns
instruction/state evidence, the board candidate owns availability/service
timing, and the audit reconciles rather than repairs them.  Primary Compaq,
Intel and device documentation plus project-owned probes are required; 86Box,
MAME and PCjs may only cross-check a primary-constrained range or an explicitly
recorded measurement contract.

## Non-goals and stop conditions

No unspecified DeskPro revision, 486+ inference, VME/PVI or x87 numeric claim,
universal 80386 PC timing, copied emulator code, host-time coupling, Windows
claim, or firmware/media import.  Each candidate stops at its own boundary;
missing CPU evidence transfers to the CPU candidate, missing board evidence to
the board candidate, and reproduced integration defects back to either owner.

## Evidence standard

Require exact profile provenance across all three candidates and, collectively,
the CPU/bus/device/NMI source ledger, source-to-model or reference-exhausted
timing table, paging/IRQ/DMA contention and reset replay, project-owned
firmware/DOS probes, current-gate evidence and a closure matrix that keeps this
board distinct from the 5150/XT and 5170.
