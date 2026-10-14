# PC110 Board, Chipset And Firmware Mapping

## Admission And Dependencies

Unnumbered M5 candidate in the [NXVM queue](../states/QUEUE.md), created by the
owner-requested PC110 task split. Allocate a numeric T only at admission.
Depends on: T558 frozen board/firmware roles and both 486 tasks.
Use the [PC110 programme contract](m5-pc110-evidence-and-implementation.md)
and its convergence ledger; the task packet freezes the exact batch and source
baseline before implementation. Research references are not implementation imports.

## Objective And Scope

Establish the sole PC110 composition and construction lifetime; implement VL82C420 memory/config/decode/shadow and Bowman/Pluto routing responsibilities, standard PIC/PIT/DMA/RTC wiring, reset and external-ROM build embedding. Create one staged composition/test harness for later devices, not a second machine.

Expected owners: src/app-mypc110 and test/app-mypc110; genuinely reusable Core chip/board mechanisms and NXVM build glue as needed.

## Non-goals And Stop Conditions

No firmware-offset dispatch, fake register success, copied ordinary-AT identity or declared bootable placeholder EXE. Later device endpoints must be explicit absent/unqualified dependencies.
Stop and report source/model conflicts, missing lawful assets needed by this
batch, or a required Shared-six modification; do not invent an undocumented
fallback. Continue independent in-scope work where possible.

## Suggested S Boundaries

Construction and mapping; chipset/configuration; integrated peripheral routing and failure/reset proof.

These are planning boundaries, not allocated S identifiers. Split an oversized
batch before admission; never use letter-suffixed S numbers or an endless
first-failing-BIOS-instruction repair sequence.

## Verification And Exit

Defined routes, ROM protections, RAM/shadow aliases, reset sequence and construction rollback pass direct and composed tests. Exact missing devices are retained by later candidates; board completion is not POST success.

Use direct owner tests plus the real production connection, full repository
unit gates and required external integrations under Execution. Reuse the two
existing Ninja/ccache graphs, rebuilding each shared target once per host
architecture/configuration. Rebuild and deploy every affected admitted App pair;
no PC110 artifact claims runnable qualification before real-firmware proof.
Record actual source/test diff, source grades, review, unresolved scope and
artifact identities; preserve owner INI, ROM masters, disks and snapshots.
