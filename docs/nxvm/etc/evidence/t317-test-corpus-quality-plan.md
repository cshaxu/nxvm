# M5 T317: Test-Corpus Quality Corrective Plan

## Approved Boundary

T317 repairs only project-owned CPU test corpus code added by T316 and its
setup-only support surface. It precedes, but does not replace or reorder, the
existing 80386DX capability candidates in [Queue](../../states/QUEUE.md). It makes no
CPU behavior, ABI, product, x87, device, timing, or Windows-readiness claim.

The audited baseline is 47 `tests/machine` smoke sources added by T316. The
S33--S65 subset contains 33 sources; every one lacks target-local GCC strict
options and contains direct fixed-width C types. Existing inherited sources
outside this list remain a separately governed migration issue.

## Package Completion Standard

- Every one of the 47 T316-added smoke targets compiles under target-local GCC
  `-Wall -Wextra -Wpedantic -Werror`, verified from actual Ninja commands.
- The 47 owner sources and touched `tests/support` headers use project types;
  a narrow static gate prevents new direct `uint*_t`/`int*_t` use there while
  allowing the foundational definitions in `src/type.h`.
- Pure test lifecycle, provider binding/freezing, real-mode preparation,
  controlled execution, and copied observation helpers are centralized in
  `tests/support` without a product-visible API or mirror state. Instruction,
  GDT/IDT, PIC, and family-specific assertions remain in their owner smokes.
- The final audit maps each changed helper to callers, verifies no capability
  semantics changed, runs full current-gate, and records remaining inherited
  type migration as explicit debt rather than a waiver.

## Ordered S Tasks

| S | Objective | Required proof | Stop boundary |
| --- | --- | --- | --- |
| S1 | Add target-local strict GCC options to all 47 T316-added smoke targets. | Configure/build plus actual Ninja command evidence for all four flags on all 47. | Any warning requiring a product/legacy source change, global flags, or an unclear target ownership classification. |
| S2 | Migrate the 47 owner sources and needed test-support headers to project types; add a narrow type-vocabulary gate. | Zero forbidden fixed-width types in the governed set; gate negative/positive self-check; strict build and current-gate. | A required migration outside the governed test set or an exception beyond `src/type.h`. |
| S3 | Consolidate duplicated test-only fixture mechanics into `tests/support`. | Caller inventory, focused evidence for each migrated family, no `src/` include of support, and no product/API change. | Any helper would carry instruction semantics, mirror mutable machine state, or lacks caller coverage. |
| S4 | Perform package closure audit. | Recheck 47 command lines, type gate, support ownership, full current-gate, documentation governance, and task history. | Any residual corpus defect, undocumented exception, or behavior change. |

## Owner-Approved Corrective Extension

S5 is a corrective extension admitted after the S4 closure audit identified the
recorded inherited raw-type migration boundary.  It replaces direct standard
fixed-width vocabulary across the approved tracked C, header, CMake, and
PowerShell surface, with only `src/type.h` foundational aliases and a controlled
negative verifier fixture as lexical exceptions.  It changes no capability or
runtime behavior and retains the original S1--S4 evidence boundary.

## Deferred Boundary

Repository-wide raw fixed-width C types, inherited NXVM code, and test sources
not added by T316 are not silently exempted. They remain a future incremental
migration with a baseline and explicit admission; T317's narrow scanner must
not misrepresent them as fixed.
