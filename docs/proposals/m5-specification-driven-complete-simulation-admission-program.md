# M5 Specification-Driven Complete Simulation Admission Program

## Purpose

This shared M5 proposal defines the required implementation order for every
admitted hardware T. Its aim is complete observable function and deterministic
timing for the task's frozen hardware surface, using the strongest available
evidence without inventing facts. It supplements the Core L3 timing design; it
does not alter the project Rules, select a machine, or make an L4 claim.

## Mandatory Three-Level Ledger

Before implementation, each hardware T freezes a finite ledger of its chip,
board route, controller phase, media, display, CPU form, or other hardware
units. Each unit has exactly one current level and records its source,
configuration, implementation action, lifecycle/reset behavior, test owner and
exit disposition.

| Level | Admission basis | Required implementation and proof |
| --- | --- | --- |
| 1. Document-primary | Applicable CPU/chip/device manual, board technical reference, schematic, protocol or other checkable documentation defines the observable rule. | Implement the documented function and timing completely for the frozen unit, including declared modes, errors, reset, cancellation and inter-device routes. Map every claimed rule to source and focused regression. |
| 2. Reference-derived | Documentation leaves a bounded observable gap after Level 1 review, and a named version/configuration of PCjs, Bochs, QEMU, 86Box, MAME or another separately approved mature emulator provides a reproducible behavior reference. | Derive a neutral project contract from recorded observations and project-owned differential probes. Implement and test that finite contract; identify its reference version/configuration and preserve its source tier. The reference is evidence, never copied source, a runtime dependency, or an authority for a documented contradictory rule. |
| 3. Maintained boundary | Neither Level 1 nor an admissible Level 2 contract resolves the remaining bounded behavior. | Retain a deterministic, reset-safe and test-covered functional behavior with its limitation visible. It must not claim L3 timing or undocumented compatibility. Record the exact missing fact and earliest future admission path. |

Level 1 is the first implementation obligation wherever documentation exists.
Level 2 is not permission to skip documentation research, and Level 3 is not a
silent fallback. A unit may move upward only by recorded evidence and must
re-run its applicable regressions.

## Mandatory Per-Unit Admission Sequence

Every newly admitted CPU, chip, controller, media controller, or display
category is one independent candidate T.  An aggregate machine or board task
may compose accepted unit contracts, but may not discover and implement a new
unit by adding successive unrelated subtasks.  Before its implementation S,
that T must complete these ordered, finite stages:

1. **Original-source ledger:** acquire and visually check the applicable
   original manual, board reference and selected personality material; record
   page/formula provenance, OCR uncertainty and any lawful reference-emulator
   cross-check boundary.
2. **List 1 - complete function/timing ledger:** freeze every command,
   register, instruction form, state transition, reset/cancellation rule,
   producer/consumer route and timing/phase row in the unit's selected
   surface.  Each row records its source level and exact L3, L2 or maintained
   boundary disposition.
3. **List 2 - current-code gap ledger:** map every List-1 row to current code,
   one owner, test/probe, missing input and implementation disposition.  A
   missing row cannot be discovered opportunistically during implementation.
4. **One-owner implementation and closure:** implement the entire admitted
   List-2 batch at its existing owner, remove obsolete duplicate paths and run
   the unit's complete source-to-test sweep.  A later board task consumes the
   frozen contract; it does not recreate the controller.

No task may substitute one repaired symptom, guest boot, or benchmark for
either complete ledger.  Similar-looking units may share a neutral Core
mechanism only after their separate List-1/List-2 ledgers prove the same
semantic, lifecycle and failure boundary.

## Per-Task Completion Standard

A hardware T may close only when every unit in its frozen ledger has been
implemented and proved at its classified level. A successful guest boot or a
single benchmark is not a unit disposition. The closure record must show:

1. the complete frozen unit set and each unit's classification;
2. the manual/reference identifier and selected machine configuration for
   Levels 1 and 2, or the exact absence finding for Level 3;
3. the implemented state, timing/phase rule where evidence supports one,
   reset/cancellation behavior and all producer/consumer routes;
4. source-to-test and source-to-trace/probe evidence; and
5. each remaining Level 3 unit's bounded behavior, non-claim and earliest
   admission receiver.

An L3 conclusion applies only to Level-1 or qualified Level-2 timing rules.
Level-3 maintenance is legitimate implementation work, but remains visibly
outside that conclusion until better evidence is admitted.

## Source and Reference Safeguards

Reference emulators are used under the source policy: their code is not copied,
transliterated or imported by this program. A Level-2 observation must name the
reference version, machine/device configuration, input sequence, observable
checkpoints and project-owned replay or differential test. A documented
primary rule prevails over a conflicting reference result; the conflict blocks
implementation until reconciled rather than choosing whichever behavior is
convenient.

This program does not require inaccessible proprietary firmware, guest media or
physical measurement. Those remain governed by the source policy and may only
enter a task through an approved lawful boundary.

## Relationship To Existing M5 Work

T433 S7 supplies the initial Core source/ownership classification. The five
Core timing candidates and every later profile/device T must use this program
for the finite hardware units in their own scope. They may implement a neutral
receiver before a profile selects a Level-1 or Level-2 rule, but must never
manufacture the missing rule or disguise a Level-3 boundary as L3.

`M5:Td:S114:SIMULATION-ADMISSION:OK`
