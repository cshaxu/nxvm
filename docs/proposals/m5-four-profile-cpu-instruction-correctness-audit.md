# M5 Four-Profile CPU Instruction-Correctness Audit And Repair Program

## Purpose

Independently re-audit the current 8086, 80186, 80286 and 80386 CPU
instruction implementations against authoritative Intel processor manuals.
This is a large CPU-compliance program, not an occasion to repair one observed
instruction such as MOV CR and infer that the surrounding semantic surface is
complete. Its purpose is to identify missing, incorrectly decoded,
incorrectly gated, or semantically wrong behavior before later profile and
device work relies on a prior completeness claim.

This candidate follows the DeskPro 386 Model-40 L3 audit because that audit
must not silently absorb a shared CPU defect. The separate
[four-profile physical-timebase closure](../history/M5-T388-four-profile-cpu-physical-timebase-closure-proposal.md)
precedes that audit and owns the CPU-clock eligibility prerequisite. This
candidate then closes the public CPU support surface's instruction-timing
ledger as well as semantics; it does not itself create a board-clock or L3
claim. It precedes
current-product device profile/functional closure so retained product
capability is not built on an unverified CPU semantic surface. The later 8088
task reuses the 8086 instruction-semantic outcome but still owns its distinct
8088 bus, prefetch and timing contract.

## Required Scope And Audit Unit

The audit unit is one Intel-manual instruction family and its concrete decoded
forms for one or more selected CPU profiles. It starts from a complete
source-derived decoder/dispatch inventory, then checks every profile-legal
form against the relevant Intel manual. It must not be driven only by known
bugs, existing handler names or currently convenient tests.

The audit covers, for every reachable selected form:

- opcode maps, escape maps, prefixes, ModR/M, SIB where applicable,
  displacement and immediate-byte consumption, including invalid and reserved
  encodings;
- operand/address-size and segment-override interactions, default-segment
  selection, string/repeat forms and all profile availability gates;
- operands, architectural state and FLAGS side effects, memory and port
  transaction behavior, exception/vector/error-code behavior, restart and
  retirement behavior, and reset-visible constraints;
- real, protected, virtual-8086 and paging-relevant differences where the
  selected profile supports them, including privilege, CPL, IOPL, descriptor
  and task-state checks; and
- 80186, 80286 and 80386 additions, including system instructions, CR/DR/TR
  encodings, control/debug/test-register access, descriptor-table and task
  instructions, and special encodings such as LMSW/SMSW, CLTS, ARPL, LAR,
  LSL, VERR, VERW, LGDT, LIDT, LLDT and LTR.

For each row, the durable evidence under docs/etc/evidence/ records this
minimum schema:

| Field | Required record |
| --- | --- |
| CPU/profile and form | CPU generation, execution mode, opcode bytes, prefixes, ModR/M or other encoding constraints, and valid/invalid form disposition. |
| Manual requirement | Intel manual edition plus section, table and form reference; record an inter-generation conflict instead of silently choosing one. |
| Current owner | Decoder/dispatch, execution, transaction, fault/delivery and state owner locations that implement the form. |
| Timing disposition | One of: exact Intel row plus units/context; an Intel formula implemented at the sole Core timing owner with every input captured and tested; an Intel range with an explicit admissible bound/model and validation; or source-underdetermined. A blank, implicit deterministic fallback or guessed midpoint is forbidden. |
| Test coverage | Existing focused/current-gate evidence, missing coverage, and a reproduction when behavior is wrong. |
| Conclusion | Exactly conforms, implementation gap, test-only gap, source/behavior unknown, selected-profile inapplicable, or explicitly unselected surface. |
| Repair/transfer | Earliest owner and bounded corrective receiver for a gap; no blank or implicit disposition. A reachable source-underdetermined successful form requires an owner-local debug-level timing-observation record with profile/form/context, selected fallback disposition and publication state until it is resolved, made nonphysical, or removed from support. |

A handler, timing-table entry, successful boot, or existing test alone is not
semantic proof. A missing test is not proof of conformity. Unknown and
source-underdetermined rows cannot pass by default: they remain explicit
transfers with an owner and admission condition.

## Authority, Ownership And Repair Boundaries

Use the applicable Intel primary processor manuals as semantic authority.
Record edition, section/table/form reference and every cross-generation
conflict explicitly. 86Box, MAME, PCjs, Bochs and similar implementations may
be used as bounded secondary cross-checks. When the primary manual provides an
exact row, they cannot override it. When primary authority supplies only a
range or no determinate timing, they may constrain an admissibility model or
reference-backed fit, provided the evidence labels that secondary status,
records the selected range/model and preserves the unresolved physical boundary.
They are never authority for instruction semantics. Do not import or derive
third-party source.

Every reproduced implementation defect receives one earliest shared CPU/Core
owner, a bounded correction subtask, a focused regression and a similar-form
sweep. Do not fork opcode execution into a machine profile. A CPU profile may
only expose a documented architectural difference through the existing shared
execution and transaction owners.

The audit does not silently become a repair campaign. A small, source-complete
shared correction may enter a later subtask of this T only when its mechanism,
variants, regression and sweep are bounded. A broad decoder refactor, a new
CPU facility, a cross-device/fault-delivery mechanism, or any device/board
work becomes its own queued receiver. Every discovered gap must be assigned;
none may be hidden by a generic #UD result, an invalid-profile rejection, or a
weakened regression.

## Dependencies And Verification

Begin from the existing four-profile instruction-timing and source-ledger
evidence, but treat it only as form inventory and timing provenance: it does
not close semantic correctness. Reconcile the decoder/dispatch inventory,
profile gates, execution handlers, memory/port transaction paths, fault and
delivery paths, and all current CPU-focused tests.

Instruction timing is a required per-form closure dimension. For every public,
reachable, successfully retired selected form/context, the audit must select
exactly one outcome: implement the authoritative exact Intel row at the sole
Core timing owner; implement its authoritative formula with every documented
dynamic term captured and tested; retain an authoritative range with an
explicit admissibility model or reference-backed fit, including bounds, inputs,
validation corpus and selection/error rule; or retain the form nonphysical.
A range may never silently collapse to a midpoint.

Where authority is absent or cannot determine the context, the nonphysical form
requires an owner-local debug-level timing observation record: profile/form/
context, disposition, fallback value if any, and whether elapsed/device
publication was rejected. The record is diagnostic, not evidence of L3.
86Box, MAME, PCjs, Bochs and similar implementations remain secondary
cross-checks only after the primary row/formula/range is fixed. This task does
not invent cycles, turn a secondary emulator into timing authority, or make a
new board/L3 claim.

Before this candidate closes, require:

- a complete four-profile requirement-to-source matrix, organized by instruction
  family and concrete decoded form, with no unclassified reachable row;
- the fixed evidence schema above for every row, authoritative Intel citations,
  and a recorded disposition for valid, invalid, profile-inapplicable and
  unselected forms;
- explicit coverage of decode/prefix/ModR/M/immediate handling, state/fault
  semantics and the system/control/debug/test-register instruction families;
- a complete timing disposition for every public reachable successful form and
  context: exact-row regression, formula-term regression, range-model
  validation, or retained nonphysical debug-observation proof; no silent
  deterministic timing fallback may remain;
- reproductions and focused regression coverage for every corrected defect;
- a cross-profile/profile-gate and similar-form sweep for each defect class;
- full current-gate verification after each correction and at final audit; and
- explicit transfers for x87/coprocessor breadth, unsupported system
  extensions, unselected forms and physical board timing. A supported
  source-underdetermined successful form cannot transfer without its required
  nonphysical debug observation and a named closure receiver.

The task-level closure audit must independently verify the matrix against the
current source graph and state whether every selected CPU semantic and
source-timing row is proven, fixed, nonphysical-with-observation, or
transferred. It may claim only the matrix's bounded CPU source-timing coverage;
it must never claim physical board-cycle accuracy, complete x87, unrestricted
clone compatibility, complete instruction correctness, or L3 merely because
one family or one regression is closed.

## Non-goals And Stop Conditions

This is not a board timing task, a device functional task, a free-form CPU
feature expansion, or an x87 implementation project. Its CPU timing work does
not establish prefetch, cache, READY/wait, DMA, ISA, device, host-time or
board-clock behavior. Do not infer a manual requirement from a secondary
emulator, a host compiler, a synthetic success path, or a later-processor
behavior.

Stop and record a precise receiver when the authoritative manuals leave a
behavior underdetermined, when an instruction requires an unsupported optional
unit, when the affected mechanism exceeds a bounded shared repair, or when a
real consumer/profile is not selected. Do not conceal a gap by rejecting a
valid encoding, assigning a generic #UD outcome, or weakening an existing
regression.