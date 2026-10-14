# PC110 Audio And Remaining Peripheral Coverage

## Admission And Dependencies

Unnumbered M5 candidate in the [NXVM queue](../states/QUEUE.md), created by the
owner-requested PC110 task split. Allocate a numeric T only at admission.
Depends on: PC110 board/DMA/IRQ plus frozen device/option inventory.
Use the [PC110 programme contract](m5-pc110-evidence-and-implementation.md)
and its convergence ledger; the task packet freezes the exact batch and source
baseline before implementation. Research references are not implementation imports.

## Objective And Scope

Implement selected ES488/OPL2/PC-speaker behavior and remaining required onboard/docking functions. Qualify DMA/IRQ, programmed audio output and reset/teardown without depending on host audibility. Reconcile every optional peripheral from the T558 inventory.

Expected owners: PC110 audio, serial/parallel/IR/modem/dock owners and tests; Core chips only for genuinely independent mechanisms.

## Non-goals And Stop Conditions

Do not call speaker-only sound complete audio. No silent removal of modem/dock/IR obligations; exclusions require owner approval and a named receiver. No new Lib Audio backend merely for this App.
Stop and report source/model conflicts, missing lawful assets needed by this
batch, or a required Shared-six modification; do not invent an undocumented
fallback. Continue independent in-scope work where possible.

## Suggested S Boundaries

Audio register/DMA/FM batches; remaining required interfaces; option/exclusion and resource audit.

These are planning boundaries, not allocated S identifiers. Split an oversized
batch before admission; never use letter-suffixed S numbers or an endless
first-failing-BIOS-instruction repair sequence.

## Verification And Exit

All required peripheral rows are proven; explicit owner-approved exclusions remain visible. Deterministic PCM/event tests cover mute-independent behavior and failures; no placeholder success for omitted hardware.

Use direct owner tests plus the real production connection, full repository
unit gates and required external integrations under Execution. Reuse the two
existing Ninja/ccache graphs, rebuilding each shared target once per host
architecture/configuration. Rebuild and deploy every affected admitted App pair;
no PC110 artifact claims runnable qualification before real-firmware proof.
Record actual source/test diff, source grades, review, unresolved scope and
artifact identities; preserve owner INI, ROM masters, disks and snapshots.
