# T386 S24: Model 40 1.2 MB FDC Logical Closure

`M5:T386:S24:FDC-12MB-LOGICAL:OK`

`M5:T386:S24:FDC-DMA2-IRQ6:OK`

`M5:T386:S24:MODEL40-FDC-BINDING:OK`

## Contract And Owner Decision

The selected Model-40 profile already binds one 5.25-inch 1.2 MB raw-IMG drive
as 80 cylinders, two heads, 15 sectors per track and 512 bytes per sector. Core
owns the neutral 8272A command/state machine, media transaction, reset lifecycle,
DMA request and PIC IRQ publication. VM owns raw-IMG media storage and the
Model-40 fixed geometry/topology. No duplicate controller or profile-local
transfer path is introduced.

S24 verifies the selected 500 kbps logical path through the actual FDC ports:
DOR reset and Sense Interrupt publication, Specify non-DMA state, sector 15
read, normal CHRN result publication, and sector-16 no-data failure. The success
result reports the controller's next record number 16 after consuming sector 15.

## Proof And Boundary

`vm-model40-fdc-s24-smoke` uses a private Model-40 session, installs a correctly
sized raw image, and drives real FDC ports. It proves the fixed 1.2 MB geometry,
CCR 500 kbps control value, IRQ6/DMA2 topology, reset IRQ/sense behavior, normal
last-sector payload/result, and out-of-range error result.

P1 verification passed: focused S24 regression; T345 ownership verification with
225 rows (165 owner tests); 60 specialized/governance checks; and the serial
current gate 278/278. Corrective P2 adds an actual 512-byte Model-40 DMA2 transfer into Core RAM through the production DMA/FDC path; it passes focused verification. Real external Model-40 ROM BIOS storage-consumer replay remains the only acceptance requirement. The unchanged runnable artifact is `vm-0-5-0389` SHA-256
`0CB4173F06E91C3BD5F9493497F2EC5480F361A23493ED7A7326000DADFB3F20`.

Flux/CRC/index/rotation/physical CHRN observables, device service and board
arbitration timing, firmware/media import, and generic variants remain excluded.
The next T386 functional receiver is the fitted 40 MB Compaq fixed-disk startup
route, followed by CECG and the task-level functional audit.