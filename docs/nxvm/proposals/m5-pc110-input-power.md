# PC110 Keyboard, Pointer And Power Control

## Admission And Dependencies

Unnumbered M5 candidate in the [NXVM queue](../states/QUEUE.md), created by the
owner-requested PC110 task split. Allocate a numeric T only at admission.
Depends on: PC110 board/configuration ownership established.
Use the [PC110 programme contract](m5-pc110-evidence-and-implementation.md)
and its convergence ledger; the task packet freezes the exact batch and source
baseline before implementation. Research references are not implementation imports.

## Objective And Scope

Model required keyboard-controller commands, make/break, pointing/inking protocols, power MCU/status LCD, cold/warm reset, suspend/wake and their IRQ/clock relationships. Decide firmware MCU execution versus an evidenced behavioral model before coding each controller.

Expected owners: PC110-owned keyboard, pointer, inking, MCU/power/status modules and tests; Core chip contracts only for proven reuse.

## Non-goals And Stop Conditions

No host key translation masquerading as a controller, always-ready MCU, second lifecycle controller or unapproved universal MCU framework.
Stop and report source/model conflicts, missing lawful assets needed by this
batch, or a required Shared-six modification; do not invent an undocumented
fallback. Continue independent in-scope work where possible.

## Suggested S Boundaries

Keyboard and pointer contract; power/reset/wake and status; composed input/power failures.

These are planning boundaries, not allocated S identifiers. Split an oversized
batch before admission; never use letter-suffixed S numbers or an endless
first-failing-BIOS-instruction repair sequence.

## Verification And Exit

Input reaches real guest-facing protocols; pending events/reset and suspend/wake are proven. Documented controller model boundaries and firmware dependencies are explicit; unsupported commands do not falsely succeed.

Use direct owner tests plus the real production connection, full repository
unit gates and required external integrations under Execution. Reuse the two
existing Ninja/ccache graphs, rebuilding each shared target once per host
architecture/configuration. Rebuild and deploy every affected admitted App pair;
no PC110 artifact claims runnable qualification before real-firmware proof.
Record actual source/test diff, source grades, review, unresolved scope and
artifact identities; preserve owner INI, ROM masters, disks and snapshots.
