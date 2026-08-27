# T492 S4 8272A Reset And Timing Implementation

`M5:T492:S4:8272A-RESET-TIMING:OK`

## Retained owner and data flow

`core_machine_fdc` remains the only owner of FDC command phase, DOR reset,
ready sampling, deadline, DRQ and IRQ state.  A profile supplies the copied
`core_machine_fdc_config.ticks_per_microsecond` value during plan construction;
the FDC owns every later conversion and transition.  No VM callback, host
clock, media mirror or second controller state was added.

## Corrected behavior

- DOR reset release samples only ready drive inputs, queues only those Sense
  Interrupt responses, and publishes their IRQ at the uPD765A 1.024-ms reset
  deadline.
- The prior fixed three-ms seek and 128-tick byte constants are deleted.
  The FDC derives Specify SRT as `(16 - SRT) * 1 ms` and CCR=0 byte cadence as
  16 us through the frozen conversion value.
- A zero conversion explicitly preserves the documented L2 no-delay fallback.
  The selected IBM 5160 profile copies zero; it does not claim a physical time
  axis.  Focused fixtures inject eight ticks per microsecond solely to prove
  the owner-local manual formulas.
- HLT/HUT have no present Core head signal, rotation owner or external timing
  input to govern.  They remain an explicit external L3 board/media boundary,
  not a guessed delay.
- The MFM `42h` `READ_TRACK` route is retained as the NEC `02h` Read
  Diagnostic command; no duplicate command route was added.

## Verification

- `cmake --build build/mingw-gcc-x64 --target core-machine-fdc-smoke core-machine-fdc-topology-port-smoke core-machine-fdc-media-change-port-smoke vm-xt-5160-268-profile-smoke`
- `ctest --test-dir build/mingw-gcc-x64 --output-on-failure -R "core-machine-fdc-(smoke|topology-port-smoke|media-change-port-smoke)|vm-xt-5160-268-profile-smoke"` - 4/4 passed.
- `cmake --build build/mingw-gcc-x64 --target verify-fdc-state-machine-boundary verify-dma-fdc-boundary` - both passed.
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .` - passed.

## Simplicity result

The implementation removes two production timing constants and the fabricated
four-slot reset queue.  It adds one copied scalar, one local conversion helper,
and reset-deadline state to the existing FDC owner; no source or runtime path
is duplicated.
