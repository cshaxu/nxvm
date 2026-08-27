# T492 IBM 5160 8272A FDC And Media Closure Audit

`M5:T492:S5:8272A-CLOSURE:OK`

## Scope and retained architecture

T492 closes the selected IBM 5160 NEC uPD765A-compatible controller and its
logical floppy-media route.  The accepted NEC and IBM source ledger, 30-row
List 1 and List 2 remain the complete authority chain:

- `t492-s1-8272a-original-source-ledger.md`
- `t492-s2-8272a-function-timing-list-1.md`
- `t492-s3-8272a-current-code-gap-list-2.md`
- `t492-s4-8272a-reset-timing-implementation.md`

`core_machine_fdc` remains the only owner of registers, command/result phase,
ready state, media transfer, DRQ, IRQ and FDC deadlines.  The media provider
owns bytes and media facts; profiles only copy immutable topology and timing
values at construction.  There is no ATA shim, media mirror, profile-side FDC,
host-clock callback or second DMA/PIC path.

## Ledger result

All functional rows F1--F23 have a complete owner-path disposition.  The
selected route retains the source-correct MFM `42h` Read Diagnostic/Read Track
path, while 765B `Version` is explicit L0 rather than falsely supported.

Manual-L3 reset, Specify SRT and CCR=0 byte-rate formulas use the copied FDC
conversion input whenever a source-qualified value is selected.  IBM 5160
explicitly selects zero and therefore retains its declared L2 no-delay
fallback.  Model-40 explicitly retains its existing L2 conversion value at
the same construction boundary.  HLT/HUT require a board/media head signal
and rotation owner that do not exist in this logical-media route; they remain
an explicit external L3 boundary.  Raw physical media, electrical/mechanical
behavior and host-file policy are outside the selected capability.

## Verification

- Focused FDC/XT and Model-40 regressions pass, including reset-ready,
  reset-deadline, SRT, byte cadence, Read Diagnostic, DMA, NDMA and selected
  profile fallback coverage.
- One serial `ctest --test-dir build/mingw-gcc-x64 --output-on-failure -j 1`
  completed with **300/300 passed** in 98.40 seconds.  Parallel attempts are
  not closure evidence because they expose unrelated shared-state interference.
- `verify-dma-fdc-boundary` and `verify-fdc-state-machine-boundary` pass.
- Documentation governance passes.
- Stripped optimized Release artifact:
  `build/output/nxvm_0_5_0492.exe`, 1,238,877 bytes, SHA-256
  `1BF1F70B42DE74674F52A07FCB094C0CEE25D0BE257390EB7C09CA26EE9F7531`.
  The Release build's artifact verifier passed and `objdump -h` reports no
  debug section.

## Simplicity result

The result replaces two FDC-wide fixed timing constants and the fabricated
four-slot reset queue with one copied scalar and owner-local deadline state.
Model-40's pre-existing proportional behavior is now an explicit copied value,
not a hidden dependency on a global FDC constant.  Obsolete four-slot test
expectations and their unused helper were removed.
