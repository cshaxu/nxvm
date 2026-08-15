# T374 S5: Model-339 FDC Ready-Attention Repair

## Change

`core_machine_fdc` now keeps per-drive sampled logical Ready state separately
from `observed_media_generation` and the DIR disk-change latch. The sample is
established at controller initialization and reset. `Specify` enables the
8272A-compatible Ready polling contract. During refresh, an actual Ready edge
publishes exactly one existing FDC interrupt path: ST0 receives the documented
Ready-change interrupt code plus Not Ready indication and the affected drive;
the existing IRQ6 source and `flagINTR` are asserted. The existing `Sense
Interrupt Status` path returns ST0/PCN and clears that IRQ source.

The behavior follows Intel's [8272A controller documentation](https://www.bitsavers.org/components/intel/_dataBooks/1981_Intel_Peripheral_Design_Handbook.pdf): Ready transitions after `Specify` generate an interrupt and are acknowledged through `Sense Interrupt Status`; its seek-interrupt table assigns Ready change the `C0h` interrupt code and its ST0 definition supplies the Not Ready bit. The uPD765 application note selected in S4 independently specifies the same polling/attention/Sense-Interrupt sequence.

`core_machine_fdc_drive_ready()` now reuses the same drive-media readiness
predicate after its selected-drive, DOR-reset and motor conditions. There is
still one FDC IRQ/DRQ owner; the patch neither adds a PIC route nor changes a
DMA transfer path.

## Focused Proof

`core-machine-fdc-media-change-port-smoke` now proves all of the following:

1. after `Specify`, drive 1 changing ready to not-ready raises the existing IRQ
   and `Sense Interrupt Status` returns `C8h | 1` with PCN zero, then clears
   `flagINTR` and the PIC source;
2. the opposite Ready edge also raises and clears one attention IRQ;
3. a drive-0 media-generation-only change still raises DIR Disk Change but not
   an attention IRQ;
4. a later Ready edge is observable, and DOR reset clears its pending IRQ and
   restores command phase;
5. existing selected-drive disk-change, DMA DRQ cancellation, seek/recalibrate
   acknowledgement and FDC reset paths remain valid.

Fresh Git-Bash CMake builds through the existing Ninja generator produced:

```text
M5:T291:S1:FDC:PORT:OK
M5:T283:S2:CORE-FDC-MEDIA:OK
M5:T347:S2:FDC-SERVICE:OK
M5:T290:S1:FDC:PORT:OK
M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK
```

No ROM, firmware, guest media, local asset, external source or runtime
configuration was added.

## Similar-Issue Sweep And Transfer

The sweep covers all FDC media/Ready queries, initialize/reset/finalize,
`Specify`, refresh, DOR cancellation, IRQ/DRQ publication and every
`Sense Interrupt Status` producer/consumer. Media generation remains the sole
input to DIR Disk Change; Ready attention is a distinct edge. A pre-existing
pending FDC interrupt prevents a second publication, so the controller retains
one visible interrupt/result owner instead of inventing an unproven queue.

This closes only the selected controller Ready-attention functional gap. The
poll interval, signal polarity, motor/settling/index behavior, mechanical
operation, DMA grant/service, IRQ phase and all Model-339 board timing remain
exclusive work for the later 5170 timing candidate. Read Deleted, Write
Deleted and Scan remain separate unsupported-command functional dispositions.
**Model-339 FDC functional completeness and Model-339 L3 are not claimed.**
