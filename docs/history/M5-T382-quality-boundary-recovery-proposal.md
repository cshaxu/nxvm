# M5 Quality-Boundary Recovery

## Summary

This candidate packages the current committed-baseline quality findings from
the 2026-08-15 whole-repository audit. It repairs only the named mechanism
owners and their tests. It does not broaden NXVM hardware compatibility,
change guest-media contents, relax any gate, or claim completion of existing
legacy strictness debt.

The candidate is intentionally unnumbered. A coordinator must inspect the
then-current worktree, `CURRENT.md`, `QUEUE.md`, `TODO.md`, the latest audit
evidence, and all affected owner routes before allocating the next numeric T.

## S1: Current-Gate Aggregate Liveness

### Goal

Make `run-current-smokes` complete or fail within an owned aggregate deadline,
with useful diagnostics and no orphan process or locked CTest temporary log.

### Implementation

Trace the CMake custom-target to CTest process creation and all host-specific
wrapper behavior. Give one owner responsibility for aggregate deadline,
no-progress diagnosis, child-process cleanup, and result propagation. Preserve
the full current smoke selection and individual test failures. Test both a
normal current-gate run and a controlled abnormal child path.

### Non-goals

Do not remove, relabel, serialize, skip, or weaken a smoke merely to shorten
or unblock the gate. Do not make owner-provided assets a committed dependency.

### Acceptance

A fresh isolated GCC build runs the full `current-gate` selection to a final
pass/fail result. The abnormal-path regression proves bounded termination,
retains the failing diagnostic, and leaves no CTest/Ninja/test process or
locked `LastTest.log.tmp`.

### Exit

All aggregate CTest entry points are swept and classified. Any unsupported
host-specific limitation is recorded with an exact fallback and admission
boundary rather than silently ignored.

## S2: Console Dead-Code Retirement

### Goal

Remove the unreachable legacy `DEVICE` implementation left after the frozen
YAML Console cutover.

### Implementation

Delete the block after the unconditional return in `doFloppy`, and remove
unreferenced helpers such as the old setting/test routes if no retained command
uses them. Sweep the Console command table, help text, failure test, and
interactive lifecycle callers so retained commands have one reachable route.

### Non-goals

Do not restore the retired `DEVICE`, `SET`, `MODE`, or `TEST` product commands
as compatibility aliases.

### Acceptance

The retained Console smokes and input-failure smoke pass; a source sweep finds
no unreachable retired command block or unreferenced retired helper.

### Exit

The Console exposes only the frozen YAML profile/session and retained floppy
surface, with one authoritative implementation per command.

## S3: YAML Memory-Quantity Boundary

### Goal

Define one strict, overflow-safe conversion from YAML/session-option KiB text
to `memory_bytes`.

### Implementation

Put decimal lexical validation, negativity rejection, range checking, and
checked KiB-to-byte conversion at one owner boundary used by catalog parsing
and session option construction. Reject malformed, signed-negative, trailing,
and overflow input before session construction. Preserve valid configuration
and existing profile restrictions.

### Non-goals

Do not expand the YAML schema, add new profile fields, or silently clamp an
invalid memory amount.

### Acceptance

Focused catalog/provider tests cover valid boundary values plus negative,
overflow, empty, and trailing-junk values; no invalid value reaches memory
configuration. Existing profile/session smokes remain green.

### Exit

All `memory_kib`/`--memory-kib` production parsing sites are swept and either
use the shared checked owner or are explicitly outside the contract.

## S4: FDC Topology Smoke Diagnostic Lifetime

### Goal

Eliminate post-destruction access in the FDC topology smoke's failure report.

### Implementation

Copy any needed diagnostics before `core_machine_destroy`, or remove them from
the report. Sweep modified FDC test fixtures for equivalent teardown-time
owner-pointer reads.

### Non-goals

Do not alter FDC production timing, controller state, or test assertions.

### Acceptance

The focused FDC topology and media-change smokes pass; an induced failure path
emits its deterministic diagnostic without accessing destroyed storage.

### Exit

All teardown diagnostics in the swept FDC fixtures use live copies only.

## S5: xasm/Debug Internal Capacity And Failure Contract

### Goal

Complete the internal statement-capacity contract that the public xasm facade
began, covering private assembler/disassembler/debug construction paths.

### Implementation

First inventory every mutable statement buffer, input length, output capacity,
return/failure path, and public caller in `aasm32`, `dasm32`, `debug`, and
`core_product_utils`. Then implement one bounded construction strategy with
defined capacity and failure atomicity, migrate all callers, and add boundary
regressions. The task must address all production `STD_STRCAT`/`STD_STRCPY`
hits in this owner domain; it may not replace a few calls cosmetically.

### Non-goals

Do not claim global strict compilation for the inherited xasm/debug target,
and do not change instruction encoding/decoding behavior except to reject
contract-invalid oversized data safely.

### Acceptance

Public and private boundary tests prove exact-fit success, overflow rejection,
unchanged output on failure where promised, and caller migration. Existing
xasm contract/debug tests and the current gate pass.

### Exit

The capacity/failure matrix names every public and private production path;
any intentionally retained unsafe legacy route has a narrowly bounded TODO
with an owner and a concrete follow-up admission condition.

## S6: Session-Manager Capacity And ID Exhaustion

### Goal

Give the product session manager deterministic capacity, allocation-overflow,
and 32-bit ID-exhaustion behavior.

### Implementation

Define an explicit maximum/representable boundary before `(count + 1)`
allocation and before advancing `next_id`. Return a stable project status
without opening a provider session or mutating manager state on exhaustion.
Add a focused controllable test seam if required, without exposing a test-only
product interface.

### Non-goals

Do not change ordinary zero-session behavior, selection semantics, or create a
new global session registry.

### Acceptance

Tests prove normal open/close behavior, allocation-limit rejection with no
partial provider session, and ID-exhaustion rejection with no duplicate ID.

### Exit

All manager growth and ID allocation paths are covered by the same explicit
failure contract.

## S7: Debug Debt Record Reconciliation

### Goal

Correct stale debt wording that claims `core_machine_debug_*_borrow()` still
exists, while retaining the independent CR0--CR4 debugger-mutation debt.

### Implementation

Verify the current operation-based core debug boundary and update only the
relevant TODO/history wording. Link to the current owner and preserve the
separate lifecycle, validation, raw-override, and regression admission
requirements for control-register mutation.

### Non-goals

Do not implement CR mutation semantics or change debugger behavior in this
documentation-only S.

### Acceptance

No current authority claims a nonexistent borrow API; the remaining CR mutation
debt names the actual operation-based boundary and an actionable admission
condition.

### Exit

Documentation governance passes and the TODO remains an open debt record, not
an active task contract.

## Feedback And Remaining Boundaries

The audit found no current core-to-product reverse include, public raw mutable
machine-layout exposure, or duplicate production dispatch owner. The retained
mixed/legacy direct-compilation strictness ledger remains valid and is not part
of this candidate unless a named S substantively changes one of its owner
domains. The T330 FDD/HDD null-backing defect is already repaired and must not
be reopened. The current local smoke assets are owner-provided inputs, so any
asset mismatch must fail diagnostically rather than be committed or silently
normalized.

## Overall Completion Standard

The admitted T closes only after every S has complete focused proof, the full
current gate has a trustworthy final result, all in-scope similar-issue sweeps
are recorded, documentation governance and diff checks pass, and the owner
accepts the final task-level audit.
