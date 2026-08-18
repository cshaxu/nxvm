# M5 DeskPro 386 Tiered Timing Fallback Foundation

## Purpose

Restore an executable path for the frozen 1986 Compaq DeskPro 386 Model 40
when original physical measurement is unavailable. The work implements missing
hardware mechanisms from the strongest available tier, without relabelling a
reference implementation or a generic IBM AT mechanism as original Compaq
physical evidence.

## Evidence Tiers

Every changed field, event and test record is labelled exactly one of:

1. `original` -- primary Compaq, Intel or component material;
2. `reference-derived` -- a named, read-only PCjs, 86Box, MAME, QEMU or Bochs
   implementation/configuration that is not imported; or
3. `generic-at` -- a declared standard IBM AT-compatible skeleton used only
   where neither of the above yields an implementable mechanism.

A lower tier may implement a reusable Core mechanism or a VM profile-local
binding. It must never silently overwrite a higher-tier selected DeskPro fact,
change the frozen Model-40 capability contract, or support an original or
physical Model-L3 claim. A later L3 audit records the tier of every accepted
row and retains any physical/firmware gap.

## Required Scope

Start with a finite receiver ledger for the five retained T398 boundaries:
CPU-to-board conversion/BWAIT; device DRQ/IRQ/NMI phase; Compaq fixed-disk
media; CECG/monitor/firmware behavior; and firmware-visible lifecycle. For
each receiver, identify the earliest existing Core or VM owner, the strongest
available tier, the exact input/output/reset/cancellation contract, and a
project-owned regression. Implement only one bounded owner mechanism at a
time, beginning with a mechanism that is independently observable without a
vendor ROM or media asset.

PCjs and 86Box are usable implementation/reference inputs despite their
non-baseline DeskPro configurations. Their configuration deltas must be
recorded. Generic AT behavior is likewise usable as a skeleton after the
ledger records the missing higher-tier input. Neither source text nor binaries,
firmware or guest media may be imported.

## Candidate Completion Standard

The foundation closes only when it has produced the finite tiered ledger, a
bounded next implementation task, its owner boundary, tests, stop conditions,
and a Queue order that no longer leaves DeskPro work behind an ineligible
post-L3 candidate. It does not close a hardware receiver or L3.

## Non-goals And Stop Conditions

Do not fabricate original Compaq timing, use host time as guest time, add a
second Core transaction/scheduler path, import third-party source or firmware,
or claim physical timing/L3 from generic behavior. Stop and transfer a row if
its mechanism would alter a frozen machine contract, requires protected media,
or cannot be assigned to one existing owner.