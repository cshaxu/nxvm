# PC110 PCMCIA, Flash And Boot Storage

## Admission And Dependencies

Unnumbered M5 candidate in the [NXVM queue](../states/QUEUE.md), created by the
owner-requested PC110 task split. Allocate a numeric T only at admission.
Depends on: PC110 board and frozen storage/firmware topology.
Use the [PC110 programme contract](m5-pc110-evidence-and-implementation.md)
and its convergence ledger; the task packet freezes the exact batch and source
baseline before implementation. Research references are not implementation imports.

## Objective And Scope

Consume the [S2 receiver checklist](../etc/evidence/t558-s2-receiver-checklists.md#inspected-storage-and-super-io-contracts)
rows SIO-1--3, PCIC-1--4, ATA-1--2 and FL-1--4. It distinguishes inspected original
contracts from unresolved PC110 wiring; both controller proof and actual
production connection are required.
SIO configuration is supplied by the board receiver; this task owns storage
endpoints and their routing tests, not another configuration/unlock state.
The [flash evidence review](../etc/evidence/t558-s2-source-availability.md#flash-evidence-availability-and-limits)
separates chip command semantics, physical bank contents, CPU decode and
board write-enable/VPP. Intel 290448-005 covers the fitted x8 part and rejects
the reference study's low-VPP explanation for missing read-ID. Referenced dump
artifacts found in Git history are research inputs, while actual board gate
behavior still needs qualification; no magic floppy-boot enable or host
pre-decompression is admitted.

Implement the selected PC Card controller/socket/window/card-detect path, required ATA/CF and internal-storage topology, firmware flash read/ID/protection and admitted command behavior. Keep storage modes through the existing Lib contract.

Expected owners: PC110 Ricoh/card/flash wiring and tests; existing Core ATA/FDC/media mechanisms where semantically applicable.

Consume the [ownership audit](../etc/evidence/t558-s1-core-app-ownership.md):
remove the unconditional first-floppy assumption only through the existing
Machine media path when the selected PC110 topology requires absence. Audit
fixed-disk slot limits against real topology, not against a proposed generic
card manager. Preserve existing four-App media behavior.

## Non-goals And Stop Conditions

Do not assume every onboard disk uses the same CF path; do not synthesize INT13 or alter guest partitions. Flash programming must never modify the external firmware master.
Stop and report source/model conflicts, missing lawful assets needed by this
batch, or a required Shared-six modification; do not invent an undocumented
fallback. Continue independent in-scope work where possible.

## Suggested S Boundaries

Controller windows/card IRQ; ATA/media and flash semantics; guest-facing boot-sector transfer and failures.

These are planning boundaries, not allocated S identifiers. Split an oversized
batch before admission; never use letter-suffixed S numbers or an endless
first-failing-BIOS-instruction repair sequence.

## Verification And Exit

Read/write/reset/removal, address/geometry boundaries, readonly protection and errors pass at controller and composed interfaces; real firmware transfers sectors through modeled hardware. Legal media fixture and flash persistence policy are fixed before acceptance.

Use direct owner tests plus the real production connection, full repository
unit gates and required external integrations under Execution. Reuse the two
existing Ninja/ccache graphs, rebuilding each shared target once per host
architecture/configuration. Rebuild and deploy every affected admitted App pair;
no PC110 artifact claims runnable qualification before real-firmware proof.
Record actual source/test diff, source grades, review, unresolved scope and
artifact identities; preserve owner INI, ROM masters, disks and snapshots.
