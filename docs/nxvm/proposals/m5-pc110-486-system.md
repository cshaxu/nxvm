# Selected 486SX System, Reset And Timing

## Admission And Dependencies

Unnumbered M5 candidate in the [NXVM queue](../states/QUEUE.md), created by the
owner-requested PC110 task split. Allocate a numeric T only at admission.
Depends on: Selected 486SX instruction semantics.
Use the [PC110 programme contract](m5-pc110-evidence-and-implementation.md)
and its convergence ledger; the task packet freezes the exact batch and source
baseline before implementation. Research references are not implementation imports.

## Objective And Scope

Consume [S2 CPU contracts CPU-4--9](../etc/evidence/t558-s2-receiver-checklists.md#inspected-cpu-delta-and-timing-contracts).
PRM 26.2.2.3 timing assumptions and Appendix E penalties are mandatory ledger
inputs. Distinguish nominal instruction cost from external bus wait and fault
cost; family identity alone does not qualify all embedded-486 features.
Also own CPU-2's cache/TLB effects and CPU-3's hardware-state effects under
the [single-owner handoff](../etc/evidence/t558-s2-receiver-checklists.md#one-implementation-owner-per-cross-task-contract).
Reuse the instruction receiver's decode/exception path; reconcile advertised
features with implemented system behavior before admitting the CPU to a board.

### T559 S5 Cache/TLB Receiver

The [S5 inspection](../etc/evidence/t559-s5-system-instruction-boundaries.md)
confirms that no current cache/TLB operation can discharge I4--I6. This receiver
owns their complete implementation, including any still-unimplemented decode,
effective-address and privilege work needed to connect those operations:

- INVD: internal invalidation and external flush indication; WBINVD: internal
  invalidation, external writeback and subsequent flush in that order.
- INVLPG: memory-only encoding, 16/32-bit effective addresses and segment
  semantics, invalidation without reading target data, and the no-entry case.
- TR3/TR4/TR5: cache fill/read buffers, tags/valid/replacement information and
  actual cache commands, including the documented TR5 read restriction.
- TR6/TR7: TLB lookup/write commands and results, paging-enabled testing,
  replacement behavior and interaction with translation/CR3 invalidation.
- Both MOV directions and all register indices, real/CPL0 versus CPL1--3 and
  VM86, operand/address prefixes, LOCK, fetch failures, unchanged flags and
  fault/restart state. Do not treat the old passive TR6/TR7 copies as 486 proof.

Retained 386 behavior has direct tests. Legal 486 cache/test operations remain
unqualified; their current UD/passive behavior is an implementation limitation,
not a permitted hardware contract. Reconcile each form against the original
manual before changing metadata, preview or execution, and remove the limitation
at the one real state owner rather than layering another dispatch path.
The existing Core admission gate must remain closed until this whole matrix
and the other system obligations are qualified. No new dummy state, successful
no-op, guest exception or internal-error fallback is authorized by this transfer.

Complete reset identity/vector, control/flags/paging/alignment and required SL/SMM/power-entry behavior for the selected silicon. Qualify cache-visible instructions and guest timing at explicitly evidenced L2/L3 levels.

Expected owners: core/chips/cpu, core/x86 and corresponding test/core owners.

## Non-goals And Stop Conditions

Do not weaken segmentation or exception checks to run firmware; do not equate instruction count to a physical clock. No blanket CPUID or SMM assumption across 486 variants.
Stop and report source/model conflicts, missing lawful assets needed by this
batch, or a required Shared-six modification; do not invent an undocumented
fallback. Continue independent in-scope work where possible.

## Suggested S Boundaries

Reset/control and memory contract; required system/power transitions; timing-source ledger and complete CPU closure.

These are planning boundaries, not allocated S identifiers. Split an oversized
batch before admission; never use letter-suffixed S numbers or an endless
first-failing-BIOS-instruction repair sequence.

## Verification And Exit

Selected profile is qualified for board use; every selected instruction/system/timing row is proven or explicitly owner-disposed, including fault/restart behavior. Do not activate the profile while silently missing required SMM or reset semantics.

Use direct owner tests plus the real production connection, full repository
unit gates and required external integrations under Execution. Reuse the two
existing Ninja/ccache graphs, rebuilding each shared target once per host
architecture/configuration. Rebuild and deploy every affected admitted App pair;
no PC110 artifact claims runnable qualification before real-firmware proof.
Record actual source/test diff, source grades, review, unresolved scope and
artifact identities; preserve owner INI, ROM masters, disks and snapshots.
