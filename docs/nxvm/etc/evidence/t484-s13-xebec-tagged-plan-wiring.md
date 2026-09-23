# T484 S13 Xebec Tagged Plan And Wiring

`M5:T484:S13:XEBEC-TAGGED-PLAN:OK`

`M5:T484:S13:XEBEC-DMA3-IRQ5:OK`

`M5:T484:S13:HDC-NO-ATA-ALIAS:OK`

## Retained Shape

`core_machine_hdc_config` now has one explicit `protocol` tag and one selected
bus shape.  ATA PIO, Compaq WD-40MB and IBM WD1003 retain
`bus.task_file`; IBM PC/XT Xebec selects `bus.xebec`.  There is no common
task-file fallback and no second HDC, CHS state or media cache.

The Core board is still the only owner that installs ports, binds PIC IRQ
sources and issues DMA tokens.  For the Xebec tag it admits only the IBM
6139790 contract: `320h` data R/W, `321h` hardware-status R/reset W,
`322h` switch R/select W, `323h` write-only DMA/IRQ mask, DMA3 and IRQ5.
The Core-owned token is held in machine-private HDC DMA state; profile input
remains immutable and cannot mutate controller wiring at runtime.

## Deliberate Boundary

S13 registers the sourced wiring but returns unsupported at the Xebec port
provider.  It deliberately implements neither a DCB collector, command/result
state machine, DMA data callbacks, selected drive geometry, option ROM nor
physical service time.  Consequently no unavailable behavior is presented as
ATA or as an invented Xebec status register.  Those are the next finite Xebec
receivers under the accepted S12 ledger.

## Verification

Focused smoke proves the tag, port directions, Core-issued DMA3 token, IRQ5
and no ATA-port alias.  Existing ATA, Compaq and IBM WD1003 smokes pass.  A
Debug all-target build passes after every former direct task-file fixture was
migrated to its selected shape; current-gate passes.  The stripped Release
`build/output/nxvm_0_5_0484.exe` is SHA-256:

`73464DAEF09C0A948BD80E9CAD1F4A9B4635B6A13FB9E73DA3B25CEE71E43153`

Documentation governance passes.  This evidence imports no firmware,
third-party code, guest media or local resource path.
