# T499 S11: D4 Integration And Replay Status

`M5:T499:S11:D4-INTEGRATION:PARTIAL`

## Completed Integration Proof

The focused Core-owner suite passes after the D4 migration:

```text
core-machine-d4-refresh-hold-smoke
core-machine-pit-divider-smoke
core-machine-timeline-s2-smoke
core-machine-scheduler-smoke
core-machine-hlt-s49-smoke
vm-model40-d4-map-s16-smoke
vm-model40-d4-parity-s22-smoke
vm-model40-private-composition-s7-smoke
vm-model40-integration-s8-smoke
vm-model40-d4-compatibility-s25-smoke
```

This covers the sole Core D4 owner's control/setup reset defaults, aliases,
A20 relation, write protection, parity/IOCHK, port-B refresh output and the
Model-40 controller composition. It now also proves the selected External-L2
setup-low `1` receiver: with A20 enabled, extension RAM below `F00000h` reads
open bus and discards writes; restoring setup-low `2` exposes the same retained
RAM bytes. The selected board retains its established refresh divider; S10/S11
introduce no unproven D4 physical-timing value or a profile callback.

## Real Replay Result

One owner-authorized, read-only Model-40 ROM and supported floppy replay
reached the existing protected-to-real and C1 semantic stages with continuous
Core progress, no CPU fault and no source-unallocated retirement. It did not
reach the FDC data-read boundary before the capture's defined retirement limit.
The copied observation showed continuing firmware memory-diagnostic work and
periodic port-B timer-state changes, rather than a halted CPU or an empty
Core-event deadline.

This is not a boot-success claim and does not justify a firmware workaround,
a guessed D4 timing value or a profile-side time path. The remaining replay
receiver is the existing pre-FDC firmware/board diagnosis, unless a later
source-backed controller relation identifies a more specific Core owner.

## Release ROM/FDC Follow-up

The owner-authorized stripped-Release replay now passes the ROM memory POST
and reaches the 8272A command path. It does not deadlock in the Core deadline
route: the final FDC command is `SENSE DRIVE STATUS` for unit 1, and the Core
returns ST3 `11h` (selected unit plus Track-0, not-ready). That is consistent
with the current one-mounted-drive topology; it is not a fabricated success
or an 8272A random result. The same replay renders the firmware's `601` and
`RESUME` text. Sending F1 through the existing Core scan-set-2 input path is
consumed by the firmware keyboard queue but does not clear that diagnostic.

The original DeskPro 386 technical-reference material located during the
source sweep describes Model 40's standard configuration as one 5.25-inch
1.2-MB drive. This rules out a guessed second-ready-drive workaround. The
remaining owner is the source-gated Model-40 ROM configuration/board-diagnostic
path: its exact required FDC/drive-change observation must be reconciled from
the original reference before any profile or controller change.

A fresh visual reread of the owner-managed DeskPro 386/16 technical-spec
floppy page confirms only that Floppy/Serial/Printer is an option-board
function.  It defines neither a second-drive presence/ready policy nor the
firmware's `601`/`RESUME` diagnostic semantics.  It therefore supplies no
source basis to synthesize a second ready drive or alter the existing one-drive
topology; the source-gated diagnostic boundary remains unchanged.

The original Compaq DeskPro 386 Maintenance and Service Guide separately
defines POST `601` as a diskette-controller error and specifies `RESUME` as
the F1 continuation action.  That is a firmware-visible diagnostic contract,
not evidence that an absent drive must be reported ready.

The corresponding bounded Release replay was repeated with the probe prepared
to send the complete native F1 make/break sequence only after the firmware
published `RESUME`.  That condition did not occur during the replay, and the
FDC still received no command or terminal.  This does not implicate the Core
keyboard path or justify a key-mapping change: the manual-defined 601/F1 branch
has not yet been reached.  The earliest unresolved owner remains the preceding
firmware/board diagnostic progression.

## Current-Gate Separation

The current configured gate is green (302/302); its status is recorded in the
S12 restoration evidence. S11 remains open only for the supported-media
matrix and this honest non-terminal Model-40 ROM replay. This evidence does
not transfer the ROM/board diagnosis or hide it behind the passing synthetic
controller checks.
