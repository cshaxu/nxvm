# T386 S24: Selected 1.2-MB FDC Logical Receiver Closure Audit

`M5:T386:S24:FDC-12MB-LOGICAL:OK`

`M5:T386:S24:FDC-DMA2-IRQ6:OK`

`M5:T386:S24:MODEL40-FDC-BINDING:OK`

## Coordinator Review

The coordinator reviewed the S24 packet, the three pushed implementation
commits, actual Core/VM ownership surfaces, focused smoke, and the candidate
proposal. The Core 765A state machine remains the sole owner of controller
commands, media transaction, reset, DMA request and PIC IRQ publication. VM
continues to own the Model-40 fixed 1.2-MB drive geometry and topology. No
profile-local FDC transfer path, public test operation, firmware asset or media
fixture was introduced.

| S24 requirement | Result | Evidence |
| --- | --- | --- |
| Selected 1.2-MB raw-IMG geometry and logical non-DMA path | Accepted | `vm-model40-fdc-s24-smoke` normal sector-15 and out-of-range paths |
| DOR reset, Sense, IRQ6 and DMA2 route | Accepted | focused smoke including production DMA2 copy to Core RAM |
| Firmware-visible storage consumer | Not claimed by S24 closure | external BYOB replay reaches ROM initialization but not the boot marker |
| Physical media and timing | Explicitly deferred | proposal and `TODO.md` boundaries remain unchanged |

## Transfer And Prevention

The optional BYOB replay is retained as an asset-free test route, but its
observed later `#UD` is not evidence of a CPU defect. The current S25 receiver
must diagnose the earliest owner among shared execution and Model-40 board
startup behavior, preserve the S24 proof, and only then prove the boot-sector
consumer. The proposal and Queue now name this receiver before HDC, so later
storage is not admitted on an unproven startup path.

## Verification

The focused `vm-model40-fdc-s24-smoke` build and replay pass. Documentation
governance passes for `vm-0-5-0389`. The serial 278/278 current gate and 59
specialized checks passed at P3 and remain the accepted current-source evidence;
this closure does not claim a new executable artifact or external-ROM success.