# MyPC110 Product And Guest Qualification

## Admission And Dependencies

Unnumbered M5 candidate in the [NXVM queue](../states/QUEUE.md), created by the
owner-requested PC110 task split. Allocate a numeric T only at admission.
Depends on: Real-firmware boot task and all required feature-ledger dispositions.
Use the [PC110 programme contract](m5-pc110-evidence-and-implementation.md)
and its convergence ledger; the task packet freezes the exact batch and source
baseline before implementation. Research references are not implementation imports.

## Objective And Scope

Consume [S2 qualification rows QUAL-1--3](../etc/evidence/t558-s2-receiver-checklists.md#board-and-qualification-receivers)
and all preceding feature proofs. Product integration is the existing
Core/Product and Emulator route, not a parallel PC110 execution framework.
Qualify original DSTN and upgraded TFT independently in both host widths.
Publish exactly `mypc110_x86.exe`, `mypc110_x64.exe`,
`mypc110_tft_x86.exe` and `mypc110_tft_x64.exe` under `assets/mypc110`.
The owner-selected filenames omit version text; verify internal version,
PE width and compiled firmware/panel identity for each artifact. Both models
use the adjacent `mypc110.ini` runtime options, not runtime model/BIOS loading.

Complete lifecycle/debug/config/Console-Window integration and a frozen guest corpus: DOS shell/storage/input, Easy-Setup, Personaware and selected Windows checkpoints only where lawful media and explicit admission establish them. Audit all feature, source, quality and release ledgers.

Expected owners: PC110 App/build/tests/assets/docs; actual Core regression owners when defects are demonstrated.

## Non-goals And Stop Conditions

No moving guest-compatibility goal, broad all-software claim, cosmetic success marker or benchmark threshold substituting for behavior. Optional guest expansion is a later candidate.
Stop and report source/model conflicts, missing lawful assets needed by this
batch, or a required Shared-six modification; do not invent an undocumented
fallback. Continue independent in-scope work where possible.

## Suggested S Boundaries

Lifecycle/failure and UX qualification; finite guest suite; complete regression/release/quality audit.

These are planning boundaries, not allocated S identifiers. Split an oversized
batch before admission; never use letter-suffixed S numbers or an endless
first-failing-BIOS-instruction repair sequence.

## S28 Closure — Post-Delay Execution Boundary

S28 records the real TFT BIOS retirement sequence immediately after the
established `F000:EA84` delay return, on both host widths.  It proves that the
apparent stall is a finite BIOS port-61 delay and that the subsequent executed
routes are already owned.  The source record identifies a later `8Bh` boot/IPL
latch candidate but does not define its reset state, so no receiver was added.
The temporary probe was removed.  Full evidence is in
[the S28 record](../etc/evidence/t580-s28-post-delay-boundary.md).

## Verification And Exit

Full required repository units and external integrations pass on x86/x64; affected prior Apps remain valid, releases/hashes/PE widths and retained EXE pairs are verified. Every PC110 domain is accepted or owner-disposed, manual limits are recorded, and owner acceptance closes the product claim.

Use direct owner tests plus the real production connection, full repository
unit gates and required external integrations under Execution. Reuse the two
existing Ninja/ccache graphs, rebuilding each shared target once per host
architecture/configuration. Rebuild and deploy every affected admitted App pair;
no PC110 artifact claims runnable qualification before real-firmware proof.
Record actual source/test diff, source grades, review, unresolved scope and
artifact identities; preserve owner INI, ROM masters, disks and snapshots.
