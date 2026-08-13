# M5 Direct-Compilation Strictness Convergence

## Purpose

Consume the 175 deferred direct C compilation rows established by T344 without
turning linked-library strictness, global compiler flags, or warning
suppression into false quality claims.  The task makes the whole configured
production/current-gate graph actionable: each direct source/target pair is
owned by one of three mechanisms and has either an actual target-local strict
compile command or an exact deferred admission boundary.

## Dependencies

T344 established the canonical 218-target current-gate union, the 305-row
direct-command matrix, and declaration-uniqueness verification.  This task
consumes those mechanisms before the Windows 3.x readiness map, whose result
must not rely on an unclassified compiler-quality surface.

## Ordered Subtasks

### S1 - Whole-Graph Ownership And Diagnostic Baseline

Regenerate the actual Ninja direct-command matrix from a fresh GCC configure.
For all 175 deferred rows, record target, source, ownership class, direct
warning baseline, duplicate-compilation relationship, and one of these
admission mechanisms: project-owned owner-test remediation; safely separable
project production; or inherited/mixed production boundary.  Add a narrow
mechanical check that rejects an unclassified row or a deferred row without a
named admission mechanism.  Do not add strict flags or modify source in this
subtask; the output is the authoritative work partition, not a synthetic
"all-clean" result.

### S2 - Project-Owned Current-Gate Test Cohorts

Apply the four strict GCC flags only to the owner-test executable cohorts that
S1 proves are repository-owned.  Remediate their direct warnings without
weakening assertions, changing runtime behavior, or inventing a broad test
framework.  Group work by genuinely shared fixture/build ownership rather
than one source file at a time.  Actual Ninja commands must prove every
promoted owner source is compiled directly with all four options.

### S3 - Production Ownership Separation And Strict Promotion

Audit all deferred production rows as target/source ownership units.  Promote
only pure or safely separable ntvdm64 production targets after their direct
warning baseline is clean.  For every retained inherited or mixed target,
record the exact source ownership, warning baseline, behavior risk, and a
single future admission trigger.  Do not impose global `-Werror`, rewrite
inherited NXVM behavior, or call a linked strict library direct coverage.

### S4 - Whole-Graph Reconciliation

Regenerate the full matrix and close the task only when every configured
production/current-gate direct C row is either directly strict or has one
precise deferred ownership record.  Move residual production debt to
`states/TODO.md` only where it remains unplanned after S3; retain the row-level
matrix as evidence.  Verify fresh configure, all strict commands, specialized
matrix gates, documentation governance, and current-gate.

## Shared Completion Standard

Each implementation P proves its entire assigned S against actual Ninja
commands and a fresh GCC configure.  No global warning policy, warning
suppression, linked-dependency inference, or blanket inherited exemption is
acceptable.  A resulting TODO names its owning target/source domain, risk,
and future admission condition; it cannot say merely "legacy" or "later".

## Explicit Transfers

xasm/debug internal capacity redesign, runtime semantics, device behavior,
Windows readiness, and fixture shapes already classified as semantically
distinct remain outside this task.  This task may report a source defect
exposed by strict compilation, but a shared runtime repair needs its own
approved mechanism scope.
