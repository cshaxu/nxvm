# M5 DeskPro 386 Physical-Cycle And Phase-Timing Closure

## Current Re-admission Input

The original 1986 DeskPro 386/16 D3PE processor material, consulted transiently under the source policy, names CPU memory/I/O BUSRDY wait behavior and the CPU/system-board NAM*/MRDY* completion protocol. It permits one bounded Core-owned CPU BUSRDY completion gate only; its source file and all temporary copies are deleted and never retained. The source does not yet make an asynchronous prefetch producer, BWAIT waveform, ISA peripheral binding, or physical/L3 result implementable.

## Purpose

Advance the selected 1986 Compaq DeskPro 386 Model 40 through one bounded,
owner-preserving timing receiver at a time. This is the first current DeskPro M5
candidate after the completed tiered-fallback foundation; it is neither a task
activation nor physical/L3 acceptance.

## Evidence Tiers

Every admitted receiver records exactly one evidence tier in its packet,
evidence and regression:

1. `original` -- primary Compaq, Intel, or selected-component material;
2. `reference-derived` -- behavior or structure observed in a read-only PCjs,
   86Box, MAME, QEMU, or Bochs implementation; or
3. `generic-at` -- an explicitly stated IBM PC/AT-compatible skeleton where
   neither stronger tier yields an implementable mechanism.

A lower-tier mechanism can never become a Compaq measurement or physical/L3
claim. No reference source text, firmware, media, or machine-local asset enters
the repository. A completed receiver retains its tier in the final audit.

## Ordered Receivers

1. **CPU-to-board transaction, BWAIT, availability waits, and retirement
   (`original` where D3PE/Intel defines the signal; otherwise an explicitly
   labelled transfer).** Extend the single Core CPU transaction owner so a
   CPU memory or ISA request has observable request, wait, completion,
   cancellation/reset, HOLD/HLDA and retirement states. Apply the original D4
   2048-byte page result (initial/row miss: two waits; page hit: zero) only to
   a real eligible in-flight external transaction; adjacent completed logical
   accesses remain misses. Bind BWAIT, memory/ISA availability waits and CPU
   retirement only when their source/phase contract is named. Do not publish a
   scalar wait, infer an asynchronous prefetch producer, or add a VM scheduler.

2. **Device-phase timing and board arbitration (`original`,
   `reference-derived`, or `generic-at` per bounded signal).** Give the existing
   Core FDC, HDC, DMA, PIC, NMI and KBC owners explicit DRQ/IRQ assertion,
   deassertion, acknowledgement, reset/cancellation and arbitration ordering.
   T406's generic-AT KBC cadence and T419's bounded DMA/refresh bridges are
   completed inputs, not pending queue items. They do not close FDC/HDC phase
   timing, PIC/NMI propagation, peripheral BUSRDY wiring, or physical rates.

3. **Compaq fixed-disk controller (`original` or controller-matched
   `reference-derived`; no generic-AT substitution).** Establish the selected
   40 MB WD/Compaq controller command, data, DRQ, IRQ14, reset, media geometry
   and error-path contract only from a controller-matched source. Existing
   generic-AT HDC infrastructure and closed T407 phase mechanism may be reused
   as neutral Core plumbing, but must not be presented as this controller or as
   Compaq geometry/media behavior. If no matching source exists, retain a TODO
   rather than infer IDE behavior.

4. **CECG, display, and monitor (`original` or `reference-derived`).** Define
   the Compaq Enhanced Color Graphics register, raster/status, monitor and
   availability/wait behavior through the shared Core VADP owner and Model-40
   profile binding. Generic IBM EGA is a skeleton only; it cannot prove a CECG
   difference or monitor signal. Preserve reset/cancellation and copied-frame
   consumer boundaries.

5. **Firmware-visible BYOB lifecycle (`original` for lawful interface facts;
   no firmware import).** Define user-supplied ROM loading, slot validation,
   immutable mapping, reset, POST-visible observations and provenance/legal
   boundary in the existing VM composition/session owner. Do not commit ROM
   bytes, paths, hashes/catalogues, discovery, or a default firmware dependency.

6. **Independent DeskPro physical/L3 re-audit (consumes retained labels).**
   Reconcile every receiver's owner, evidence tier, observation corpus,
   reset/cancellation proof and residual transfer. It may accept a physical/L3
   result only if the complete selected-machine contract is supported; otherwise
   it records the exact non-acceptance and leaves no lower-tier mechanism
   mislabelled as original Compaq timing.

## Completion Standard

Each receiver is admitted as one bounded numeric task only after it names the
owner, evidence tier, finite observation/trace corpus, reset and cancellation
contract, focused regression and remaining transfer. The first receiver is the
CPU-to-board transaction/BWAIT/retirement boundary. The final re-audit is
separate and cannot be replaced by accumulated receiver closures.

## Constraints And Stop Conditions

Keep reusable instruction, transaction, scheduling, DMA, controller and display
mechanisms in Core; bind only declared Model-40 differences in VM. Stop and
transfer a receiver that needs protected firmware/media, a fabricated signal
timing, a second transaction/scheduler path, host time, generic IDE as a Compaq
controller, or an undocumented change to the frozen profile contract.