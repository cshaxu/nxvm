# M5 DeskPro 386 Physical-Cycle And Phase-Timing Closure

## Purpose

Carry the selected 1986 Compaq DeskPro 386 Model 40 from functional device behavior toward a separately auditable physical/L3 timing decision. This is a dependency-ordered candidate package, not an activation, task-number allocation, or physical/L3 acceptance.

## Evidence Tiers

Every admitted receiver records one of the following inputs and retains that label in its evidence and regression:

1. `original` -- primary Compaq, Intel, or selected-component material;
2. `reference-derived` -- behavior or structure observed in a read-only PCjs, 86Box, MAME, QEMU, or Bochs implementation; or
3. `generic-at` -- an explicitly stated IBM PC/AT-compatible skeleton where neither stronger tier gives an implementable mechanism.

A lower-tier mechanism may be useful progress, but it never becomes a Compaq measurement or a Model-L3 claim. No reference source text, firmware, media, or machine-local asset enters this repository.

## Ordered Receivers

1. **CPU external memory-cycle and prefetch boundary (`original`).** D4 material fixes a 2048-byte page, two waits for initial/page-miss CPU memory reads, and zero waits for a page hit. Admit a Core-owned observable external-cycle/prefetch contract that distinguishes speculative fetch, consumed fetch, page walk, and data access. It must define reset, cancellation, HOLD/HLDA, and retirement publication. Do not infer a page hit merely from adjacent logical accesses.
2. **D4 write, DMA/refresh, and BWAIT arbitration (`original` where the selected source defines it; otherwise `reference-derived` or `generic-at`).** Bind the preceding cycle contract to write completion and board arbitration only with an explicit signal/phase source. Preserve the sole Core CPU/DMA transaction owner.
3. **Display/CECG board phase and monitor behavior (`original` or `reference-derived`).** Close raster service, monitor blanking/sync, ISA availability/waits, and reset/cancellation through Core VADP and the Model-40 profile. Generic IBM EGA is a skeleton only and cannot prove CECG behavior.
4. **Storage controller phase timing (`original`, `reference-derived`, or `generic-at` per controller fact).** Extend the existing FDC/HDC owners with selected DRQ/IRQ/reset/ready phases; physical sector/ECC media remains a separately bounded media receiver.
5. **Keyboard/8042 and NMI board phase (`reference-derived` or `generic-at` until primary timing exists).** Retain the existing Core KBC/PIC/NMI route, add only an observable board-phase contract, and do not treat host input cadence as a Compaq measurement.
6. **BYOB firmware-visible lifecycle (`original` only where lawful primary evidence exists).** Define user-supplied ROM POST/lifecycle observations without committing vendor bytes, paths, hashes, or a discovery mechanism.

## Completion Standard

Each receiver is admitted as one bounded numeric task only after it names its owner, evidence tier, finite observation/trace corpus, reset and cancellation contract, focused regression, and remaining transfer. The package does not close until every selected receiver is accepted with that evidence or explicitly transferred to a later named candidate. A final Model-40 L3 audit remains a separate task.

## Constraints And Stop Conditions

Keep reusable instruction, transaction, scheduling, DMA, controller, and display mechanisms in Core; bind only declared Model-40 differences in VM. Stop and transfer a receiver that needs protected firmware/media, a fabricated signal timing, a second transaction/scheduler path, host time, or an undocumented change to the frozen profile contract.