# T386 S9: CECG Control And Static Configuration

`M5:T386:S9:CECG-CONTRACT:OK`

## Source Boundary

The primary contract is Compaq Computer Corporation, *COMPAQ Enhanced Color
Graphics Board / COMPAQ Color Monitor Technical Reference Guide*, first edition,
December 1986, researched transiently under the source policy. The scan is
available from the [Internet Archive record](https://archive.org/details/trg-enhanced-color-graphics-board-1986-12)
and its [OCR text](https://archive.org/stream/trg-enhanced-color-graphics-board-1986-12/TRG_Enhanced_Color_Graphics_Board_1986_12_djvu.txt).
No scan, ROM, firmware, guest media, third-party source, local path, or binary
is retained in Git.

Chapter 4 identifies Compaq-specific `3C6h` as a readable/writable Control
Mode register, `3DBh` and `3DCh` as light-pen latch reset/set writes, and
`3xAh` as Input Status 1. It also identifies the `7C6h`, `BC6h`, and `FC6h`
read-only configuration leaves. The guide's switch tables identify the selected
external Compaq Color Monitor, no internal monitor, and EGA power-on mode.

## Implemented Contract And Ownership

| Observable | Owner and result |
| --- | --- |
| `3C6h` stored Control Mode | Shared `core/machine/vadp` owns the register. It reads and writes for the CECG personality, retains documented writable state, clears the reserved bit 5 on write, and restores the composition-selected value on reset. |
| Light-pen latch/status | Shared VADP owns `3DBh` reset, `3DCh` set, and Input Status 1 bit 1. The selected no-light-pen composition declares its switch as open, reflected in bit 2. |
| Static configuration reads | Shared VADP serves Environment, Display Type, and Initial Mode from a validated CECG hardware declaration rather than hard-coded values. |
| Selected Model 40 board state | `vm/composition/session/model40_composition.c` alone selects `40h`, `00h`, `30h`, `01h`, and an open light-pen switch. This is board/profile composition, not a second device implementation. |

`core_machine_vadp_cecg_config` is a generic hardware declaration consumed by
VADP. It names no machine model. The Model 40's values remain wholly in `vm`.
Generic/IBM EGA does not register the CECG ports or receive its status bits.

## Focused Proof

- `core-machine-compaq-cecg-s9-smoke` proves source-backed CECG port ownership,
  static reads, reserved-bit normalization, light-pen reset/set status, reset,
  and generic-personality isolation: `M5:T386:S9:CECG-CONTRACT:OK`.
- `vm-model40-cecg-s9-smoke` proves the private Model 40 composition supplies
  the selected board state, exercises the shared register/latch owner, and
  recovers the selected state through a session reset:
  `M5:T386:S9:MODEL40-CECG:OK`.
- Both targets are current-gate members. Adding their two project-owned direct
  test targets updates the T345 S2 strict inventory from 142 to 144 pure
  targets; the mixed count remains 3. The final serial current gate passes`n  `260/260` in 73.33 seconds after the final test-only control strengthening.

## Similar-Issue Sweep And Transfers

The S9 sweep used `rg -n -i "3c6|lightpen|cecg|ega_personality|personality"` over
`src/core/machine`, `src/vm`, `tests`, `CMakeLists.txt`, the T386 evidence,
proposal, and `TODO.md`. It found VADP as the sole selected CECG state/port
owner and Model-40 composition as the sole selected-machine receiver; both are
fixed above. Generic/default EGA has no CECG personality or dedicated port
registration and is covered by the focused control. The S6 evidence and TODO
record were updated to transfer only the remaining receiver work. No generic/IBM
EGA production behavior changed.

The source establishes register definitions but does not close the remaining
receiver work: physical monitor/cable output, actual monitor blanking or
selection effects, alternate I/O-base routing, CPU video-memory gating,
feature/special-interface pins, multiplexed live-video status, option-ROM/BIOS
mode programming, raster/service duration, ISA availability, and board waits.
Those items remain transferred through `TODO.md` to later DeskPro functional,
firmware, board, and L3 receivers. This S makes no board-timing, firmware,
runnable-public-profile, or L3 claim.
## P2 Corrective Failure Atomicity

Coordinator review rejected P1 because the initial CECG declaration was
validated after `core_machine_vadp_configure_ega_personality()` had committed
its personality. Port registration was rolled back on a failure, but the
personality could remain changed and block a valid retry on the same machine.

P2 establishes `core_machine_vadp_cecg_config_is_valid()` as the sole CECG
validation owner. `core_machine_configure_display()` invokes it before text,
port, personality, or provider state changes; VADP reuses the same validation
when consuming the declaration. The S9 core smoke now rejects an invalid CECG
declaration, then configures generic EGA successfully on the same machine.
This is a configuration failure-atomicity repair, not a profile or device
behavior expansion.

P2 verification: focused CECG/Model-40 controls and the serial current-gate both pass; the final serial gate reports 260/260 tests passed in 78.15 seconds. Documentation governance, VM provider-composition, and deferred direct-ownership governance checks also pass after the corrective change.

## Coordinator Acceptance

P1 is retained as pushed history but rejected for a configuration failure-atomicity defect. P2 corrects that defect, its actual-change review is accepted, and this evidence records S9 only as functional progress. T386 and every firmware, physical-media, board-timing, and L3 receiver remain open.
