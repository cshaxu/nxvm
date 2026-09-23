# T366 S3: Model 339 Planar-Parity NMI Lifecycle

## Scope and authority

This S implements only the selected IBM PC/AT system-board RAM-parity source.
The March 1986 IBM PC/AT Technical Reference assigns RAM parity and I/O-channel
check as NMI sources, identifies port `61h` status/control use, and assigns the
CMOS `70h` bit 7 to NMI masking. T366 S2 selects the Model 339 Type 3 context.

## Owner contract and proof

`core_machine_configure_planar_parity` is an opt-in configuration-time owner.
It accepts only IBM PC/AT port `61h`. `core_machine_report_planar_parity_fault`
is the future planar-memory producer seam, not a test-only CPU write. A fault
latches; `61h` bit 7 reports the latch; bit 2 enables RAM parity and a clear
write resets latch/publication. `70h` remains mask-only: a masked fault stays
latched, and unmasking publishes the normal core NMI input once. Reset clears
latch/publication and restores RAM parity enabled.

`core-machine-planar-parity-nmi-s3-smoke` proves clean state, masked assertion,
`70h` unmask publication, `61h` visibility, clear/re-enable, and reset. It
emits `M5:T366:S3:PLANAR-PARITY-NMI:OK`. Existing
`core-machine-hardware-delivery-s3-smoke` remains the shared real/protected/
VM86 NMI-consumer proof. The complete current smoke gate was rebuilt and run.

## Transfers

This does not model corrupted parity bits in physical RAM; a later selected
5170 composition/memory task must bind that producer to the 512 KB planar
memory contract. I/O-channel check, adapter parity, speaker/timer behavior,
bus availability, wait states, DMA arbitration and physical/cycle timing are
excluded. No L3 or cycle-exact claim is made.
