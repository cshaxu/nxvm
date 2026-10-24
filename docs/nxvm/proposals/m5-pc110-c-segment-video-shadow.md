# MyPC110: C-segment Video BIOS Shadow

## Admission

T576 follows the closed T575 protected-bootstrap receiver.  It consumes new
L2 hardware/reverse-engineering evidence from Open-Source-PC110 that identifies
the TFT flash image's bank 2 (`0x20000..0x2ffff`) as the C&T F65535 video option
ROM and identifies the runtime C0000 window as its writable shadow.  This is a
behavioural model, not copied third-party source or a claim of transistor-level
chipset accuracy.

## Objective

Make the existing MyPC110 firmware owner expose one 48 KiB `C0000–CBFFF`
video-shadow
backing store through the existing Core memory-route contract.  At reset the
raw immutable mapping remains visible; the backing store is cleared.  The
observed EC/ED `ROMSET → CAXS/CCBL → ROMSET` sequence opens it for the guest's
own copy, then makes that copy visible.  This S proves the selected
write/close/reset behavior on both host widths.  It does not claim that real
firmware has yet reached C000 execution.

## Boundaries

- The App owns the bank selection, backing bytes and route.  Core remains a
  neutral route dispatcher.
- Raw flash remains immutable and external.  No raw payload or derived byte
  source enters Git.
- Only the documented EC/ED transition used for C-shadow copy is modeled:
  ROMSET `0Ch` open/relock and CAXS `0Fh` / CCBL `15h` values whose low six
  bits are `2Ah`.  This is not a complete VL82C420 implementation.
- No INT service, BDA/IVT mutation, PC/AT fallback, second executor, synthetic
  POST/Setup/DOS completion or PC110 release claim is permitted.

## Verification

Unit coverage proves that raw C000 remains visible before the sequence, the
sequence opens and later exposes the shadow, and reset discards it while the
existing F000/reset overlays retain their bank-3 contract.  Run affected
App/X86-NXVM diagnostics and structural/manifest gates.  Record the external
evidence, exact changed ownership, both-width observations, and every endpoint
not qualified by the bounded receiver.  The next S owns the separate
real-firmware protected-segment boundary before C000 execution can be proved.
