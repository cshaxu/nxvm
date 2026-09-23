# App/Core Code-Quality Remediation

## Purpose

Repair the confirmed App and Core production-path defects from the 2026-09-21
whole-tree quality audit, then repeat the same ownership, data-flow, duplicate-
path and failure-boundary audit until the frozen corpus has a disposition for
every finding.  This is a subtraction-and-consolidation task: it must not
introduce a framework, compatibility route or a second state owner merely to
make an individual test green.

## Frozen Coverage Universe

The initial universe is every tracked `src/app/**` and `src/core/**` source and
public interface, with its direct repository-only tests under `test/app/**` and
`test/core/**`, at the admission commit.  Each finding is assigned one of:

- repaired by an admitted S with focused proof and the complete unit suite;
- accepted as a live, semantically distinct path with caller and owner proof;
- transferred to the earliest named Queue/TODO receiver when it needs separate
  hardware or source authority.

The convergence ledger records the finding, owner, variants/callers, required
proof and disposition.  A repeated audit expands no scope silently: a newly
found App/Core mechanism becomes the next S or receives an explicit receiver.

## Confirmed Initial Batches

1. Remove the disconnected instruction recorder completely: command, object,
   observer contract, tests, build references and obsolete recording policy.
2. Repair display frame failure atomicity and ensure integration observes the
   sole production presentation route rather than a Core-only test mailbox.
3. Flatten retained recursive profile resolution/provenance mirrors and return
   board-specific materialization from the Machine adapter to Profile.
4. Remove or migrate the duplicate root host/file facade to the canonical Lib
   capability, including INI path edge cases and dead helpers.
5. Re-audit the frozen App/Core corpus after every completed batch.  Repair all
   newly confirmed in-scope defects before this task closes; do not claim a
   complete audit merely from passing gates.

## Boundaries

- Preserve retained XT, 5170, Model 40 and default PC/AT behavior, all CPU
  implementations and their tests.
- Do not change firmware, external asset, guest-media, timing-grade or product
  UX policy unless an affected mechanism demonstrably requires it and the
  active packet is revised with owner approval.
- Lib/Common/x86 source is not a cleanup target unless the App/Core repair
  exposes a concrete shared-contract defect; such a change needs separate
  admission.

## Completion Standard

The task closes only when the convergence ledger has no unclassified App/Core
finding; each repaired mechanism has one retained owner and production path;
the full repository-only unit suite and external integration suite pass; and
the four fixed products have their required stripped x64/x86 artifacts in
their sole profile directories.  A final independent code review must inspect
the actual source graph, changed tests and all live retained paths.
