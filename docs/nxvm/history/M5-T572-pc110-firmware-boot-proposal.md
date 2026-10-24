# PC110 Real Firmware POST, Setup And Guest Boot

## Admission And Dependencies

Unnumbered M5 candidate in the [NXVM queue](../states/QUEUE.md), created by the
owner-requested PC110 task split. Allocate a numeric T only at admission.
Depends on: All selected CPU/board/display/storage/input/power and required auxiliary devices qualified.
Use the [PC110 programme contract](m5-pc110-evidence-and-implementation.md)
and its convergence ledger; the task packet freezes the exact batch and source
baseline before implementation. Research references are not implementation imports.

## Objective And Scope

Consume [S2 boot rows BOOT-1--4](../etc/evidence/t558-s2-receiver-checklists.md#board-and-qualification-receivers).
Their guest images/configurations must be identified before the corresponding
integration is admitted. Reference POST support, extracted Setup and modified
EMM386 media are explicitly excluded as qualification substitutes.
The [candidate BIOS panel check](../etc/evidence/t558-s2-source-availability.md#candidate-bios-panel-identity-independent-byte-check)
also excludes the current TFT-table candidate from stock-DSTN qualification.
The owner now requires both stock DSTN and upgraded TFT compiled variants.
Evaluate an unchanged original dump or the confirmed author-supplied
[offline panel restoration route](../etc/evidence/t558-s2-source-availability.md#bidirectional-panel-patch-source-confirmed).
The latter requires explicit preparation admission, exact-site/integrity and
round-trip verification, and derived-image provenance; it is not an original
dump claim. Qualify both firmware/panel pairings independently. Each build
embeds its selected BIOS and other ROMs. No runtime firmware loader is admitted.
The board receiver owns the earlier input-preparation and embedding batch;
this task consumes its frozen hashes and checks them before guest execution.
Discovering an invalid pairing returns that prerequisite to its owner rather
than adding a boot-time patch or a second firmware-loading path.

Connect the one existing staged composition to the ordinary Core Machine/Product route. Execute the admitted firmware from reset through a frozen POST, Easy-Setup and DOS boot corpus; any approved panel-only restoration is fixed offline, never a host boot workaround. Fix hardware causes using complete affected-case ledgers, not repeated first-terminal patches.

Expected owners: PC110 composition and tests; corrections at the actual Core/App mechanism owner.

## Non-goals And Stop Conditions

No POST completer, BIOS instruction-address interception, synthetic DOS services, EMM386-removal workaround or relaxed CPU checks. No alternate production boot path.
Stop and report source/model conflicts, missing lawful assets needed by this
batch, or a required Shared-six modification; do not invent an undocumented
fallback. Continue independent in-scope work where possible.

## Suggested S Boundaries

Reset/POST batch; setup and device discovery; DOS/driver boot with input/display/media checkpoints.

These are planning boundaries, not allocated S identifiers. Split an oversized
batch before admission; never use letter-suffixed S numbers or an endless
first-failing-BIOS-instruction repair sequence.

## Verification And Exit

Every finite boot checkpoint passes on both hosts through guest CPU execution, with event/timeout containment and failure diagnostics. Admitted mypc110 EXEs and own INI are usable; dependencies are not merely mocked. Do not defer known mechanism defects to final release testing.

Use direct owner tests plus the real production connection, full repository
unit gates and required external integrations under Execution. Reuse the two
existing Ninja/ccache graphs, rebuilding each shared target once per host
architecture/configuration. Rebuild and deploy every affected admitted App pair;
no PC110 artifact claims runnable qualification before real-firmware proof.
Record actual source/test diff, source grades, review, unresolved scope and
artifact identities; preserve owner INI, ROM masters, disks and snapshots.
