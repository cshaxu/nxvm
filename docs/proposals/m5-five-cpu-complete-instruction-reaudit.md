# M5 Five-CPU Complete Instruction Re-audit And Repair

## Purpose

Re-establish a source-backed, complete instruction and architectural-state
conformance result for the project's Intel 8086, 8088, 80186, 80286 and
80386DX CPU profiles.  This follows T499; it is not a narrow FLAGS repair or
a reopening of a closed task by assertion alone.

The trigger is a current inconsistency in shared FLAGS normalization.  Its
lesson is broader: a shared state helper can affect many instructions, frames
and delivery paths.  The task therefore audits the complete admitted
instruction universe before implementation, then repairs each shared mechanism
once at its Core owner and sweeps every affected form/profile.

## Authority And Evidence Order

Intel's original manuals are normative for every instruction, defined state
bit, exception, delivery and timing claim.  Existing project ledgers are a
crosswalk, not proof.  Manual-L3 means that the manual itself gives the exact
number or an executable formula.  A documented range whose implementation
selects a value is L2, never Manual-L3.  86Box, MAME, PCjs, Bochs and QEMU are
read-only cross-checks: any timing model selected from them is L2, even when it
agrees with the manual; a proportional macro model is likewise L2.  They may
corroborate the provenance of a manual result but neither can override a
manual conflict.  Reserved or architecturally undefined state is never
asserted as a fabricated exact value.

## Frozen Coverage Universe

For every supported CPU profile, audit every decoder-admitted instruction form
and every applicable context:

- opcode and `0F` form, ModR/M, immediate, operand/address size, segment and
  repeat/LOCK prefix legality;
- register, memory, descriptor, control/debug/test, stack and FLAGS effects;
- defined FLAGS values, reserved/undefined-bit treatment, PUSHF/POPF, LAHF/
  SAHF, interrupt/exception frames and IRET;
- real mode, protected mode, VM86, privilege, gate, task, paging and restart
  behavior where the profile supports them;
- success, fault, trap, abort, hardware interrupt and NMI delivery ordering;
- every manual timing constant, formula, bounded range and explicit
  unavailable disposition.

8088 is a distinct profile row even where it shares 8086 instruction
semantics.  Its documented bus/fetch timing is not inferred from 8086.

## Ownership And Design Constraint

Core remains the sole owner of CPU state, decode, execution, memory access,
exception/interrupt delivery, retirement and guest-time publication.  A
profile selects a frozen CPU identity only.  Repairs must consolidate a shared
state/commit/delivery mechanism at its owning Core boundary; no per-opcode
compatibility patches, profile-side emulation, duplicate FLAGS image, second
retirement path or external-model runtime dependency is permitted.

## Timing-Tier Closure Rule

For every CPU profile, the audit must enumerate every L1 disposition and
repair it to Manual-L3 or a named L2 model whenever the admitted sources
support that result.  L2 includes a manual range selection, an external-model
selection and a proportional macro model. A row that cannot be upgraded is a
stop-and-report item, not a closure exception. A Manual-L3 row must not be
lowered, nor an L2 row to L1, unless the prior classification is demonstrated
false and the owner approves that correction before it is made.

## S Decomposition

1. **S1 - source admission and cross-validation.** Locate and quality-check
   original manual material for all five CPUs; record locators, OCR limits and
   external-model corroboration/conflicts.
2. **S2 - complete List 1.** Produce one finite manual-derived instruction,
   state, delivery and timing ledger for all five profiles and contexts.
3. **S3 - complete List 2.** Trace every List-1 row through current Core
   decode, state construction, commit/rollback, delivery, retirement and
   regression owner; classify conforming, missing, conflicting or unsupported.
4. **S4 - shared-mechanism repair batch.** Repair every proven common owner
   (including FLAGS lifecycle where applicable) once, remove superseded paths,
   and add mechanism-level regressions.
5. **S5 - 8086/8088 full repair and sweep.** Reconcile all legacy real-mode
   forms, legal prefixes, frames and documented timing without deriving 8088
   bus behavior from 8086.
6. **S6 - 80186 full repair and sweep.** Reconcile 80186-only forms and its
   legacy architectural-state/timing differences.
7. **S7 - 80286 full repair and sweep.** Reconcile 16-bit protected mode,
   descriptor/gate/task/stack delivery and documented timing.
8. **S8 - 80386DX full repair and sweep.** Reconcile 32-bit forms, VM86,
   paging, debug/system state, delivery and documented timing.
9. **S9 - cross-profile closure.** Reconcile all five ledgers against source
   and code, run focused/profile gates and the full configured suite, and
   record every remaining non-L3 row with its named receiver.

No implementation S may begin before S1-S3 accept the whole coverage universe.

## Exit Criteria

Every in-scope row has a manual locator and one current-code disposition; every
L1 row has been upgraded or reported as blocked; every proven defect is repaired
at its sole Core owner and swept across equivalent forms; undefined state is not
overclaimed; no decoder-admitted row remains unclassified; focused and
cross-profile regressions plus the complete configured gate are green. Any
unavailable physical timing or unsupported later-CPU feature is explicitly
labelled with its receiver rather than estimated.
