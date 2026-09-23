# T496 S8 8272A Normal-Result IRQ Release

## Scope

The selected IBM 5160 DOS replay completed two Read Data commands, then the
next INT 13h request returned Carry before it could complete.  Its FDC source
remained asserted after the preceding normal result phase, so the next command
could not produce a new IRQ6 transition.

## Source Decision

NEC uPD765A/uPD765B pages 5-3--5-4 define INT at DMA command termination, and
Table 4 defines the following result phase.  Reset and seek completion remain
the separate Sense Interrupt Status acknowledgement path.  PCjs corroborates
the normal result acknowledgement boundary by releasing its FDC interrupt when
the first result byte is consumed; it is Other L3 corroboration only.

## One Owner And Repair

`core_machine_fdc_read_data()` is the sole result-byte consumer.  On its first
ordinary result-byte read, it releases the FDC's own PIC source and clears the
corresponding pending flag.  It does not alter reset/seek Sense Interrupt
handling, PIC ownership, DMA, firmware, VM state, or media handling.

## Verification

- `core-machine-fdc-smoke` proves normal completion asserts IRQ, first result
  consumption releases it, remaining result bytes remain readable, and the
  next command can signal again.
- Focused FDC media/topology/authority/DMA-boundary/port/read-track and
  XT/PC-AT profile smokes pass in Release.
- Owner-authorized IBM 5160 ROM plus DOS 5 360K reaches the installer terminal
  after 161 successful FDC completions; the prior replay stopped after two.
- Default PC/AT reaches valid DOS terminals with 360K, 720K, 1.2M, and 1.44M
  explicit media selections.

The injected DOS Read Track end-to-end fixture remains a separate later
failure: its direct controller corpus passes, while its self-modifying DOS
boot harness does not reach its program marker.  It is not used to weaken the
selected XT terminal result.
