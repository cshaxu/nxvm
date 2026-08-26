# T484 S7 XT Enhanced-Keyboard Source Contract

`M5:T484:S7:XT-KEYBOARD-SOURCE:OK`

`M5:T484:S7:XT-NO-8042-ALIAS:OK`

## Method And Source Quality

T483 S1 freezes IBM `5160-268` with the Product Reference's enhanced
keyboard selection.  This S re-read the IBM *PC/XT 5155/5160 Technical
Reference*, March 1986, section 4's `101/102-Key Keyboard` and the system
board `8255A I/O Bit Map`.  Its OCR text was cross-read against rendered
manual pages 1-28, 4-25, 4-26, 4-32 and 4-33.  The result resolves the
otherwise source-gated keyboard protocol; it imports no manual text, ROM,
firmware, media or external implementation.

The IBM manual is the normative contract.  Read-only 86Box and PCjs inspection
only corroborates the required separation: 86Box has a distinct XT keyboard
device, and the retained IBM XT BIOS listing uses PPI port B to pulse reset and
read the returned byte through port A.  Neither implementation selects a
second owner, adds an 8042 command port, or supplies a source that overrides
the IBM manual.

## Source-Qualified Contract

| Relation | IBM Manual-L3 fact | One Core receiver and boundary |
| --- | --- | --- |
| Selected keyboard and byte form | `5160-268` selects the enhanced keyboard (T483 S1).  The 101/102-key keyboard sends a serial 9-bit frame: start bit `1`, then data bits 0 through 7 least-to-most significant (4-33). | The XT PPI attachment alone deserializes each completed frame into one byte.  VM supplies host input only at the existing typed host-input boundary; it does not own a guest FIFO, frame, or port state. |
| Wire ownership and inhibition | Clock and data are open-collector; either side can force inactive/low.  Host-forced clock-low inhibits transmission (4-32).  The system-board PPI map assigns port `61h` bit 6 to hold keyboard clock low and port `62h` bit 7 to clear/enable the keyboard path (1-28). | PPI port-B/C latch and mode state own these two selected line controls.  No `64h` command/status port, 8042 command byte, A20/reset output port, AUX origin, or 8042 FIFO is permitted. |
| Request/clear/send sequence | A ready keyboard drives clock low for RTS; the system must allow data high for CTS within 250 microseconds.  The first clock edge follows CTS in 60--120 microseconds; high phase is 25--50 microseconds; data is valid across the stated +/-2.5 microsecond edge window (4-33). | The attachment's serial state is the sole future deadline producer for these sourced values.  It may consume the already-established Core timing route only when the XT board timing plan selects a compatible physical unit; it must not estimate or have VM inject elapsed ticks. |
| Reset and completion | System reset is clock-low for at least 12.5 ms; the keyboard then clocks Reset `FF`, runs BAT and returns a completion result (4-26).  BAT success is `AA`; failure is `FC`; detection error/overrun is `FF` (4-25--4-26).  Completion follows Reset acknowledgement in 300--500 ms (4-25). | A PPI-owned reset/serial state starts from port-B control and publishes the returned byte only through the same port-A/PIC IRQ1 path.  The later board-time receiver supplies only the sourced time-unit conversion; it does not invent a different byte protocol. |
| Guest byte delivery and IRQ | The system board exposes keyboard scan code at `60h`; the selected XT interrupt relation is keyboard IRQ1 (T483 S1; IBM system-board/interrupt material).  The IBM XT BIOS is consistent with clearing the PPI shift-register path around an IRQ1 byte read. | The attachment owns byte-ready and asserts/deasserts the existing Core PIC's IRQ1 source.  Reading `60h` consumes only the XT attachment's byte state.  Profile construction provides immutable ports/IRQ selection and never holds a second guest-visible byte. |

## Current Owner Disposition

Current `src/core/machine/kbc.c` and `kbc.h` are explicitly an Intel 8042:
they own ports `60h`/`64h`, a command byte, A20/reset output, AUX state and
8042-specific FIFO/response semantics.  `kbc_aux_absent` changes AUX presence
only; it cannot represent this 8255/serial contract.  The public native-input
entry presently describes an attached 8042 and therefore is not an admissible
XT implementation seam without a source-local XT attachment dispatch.

The next B3 implementation must add exactly one Core-owned XT PPI/keyboard
attachment and one immutable selected-topology declaration.  It can reuse the
Core PIC IRQ1 service and typed host-input producer, but it must replace neither
the AT KBC nor its current callers.  The profile remains construction-only;
the new attachment is the sole owner of PPI latches, serial receive/reset
state, byte-ready condition, port `60h`--`63h` behavior and IRQ1 publication.

## Transfers

The manual establishes the keyboard's protocol and numerical delays, so their
source classification is Manual-L3.  Their conversion to the single Core
physical-time axis remains a consumer of the later XT board/device timing
task; until that profile timebase is qualified, the implementation may expose
the source-backed timing parameters through the existing board-timing path but
must not claim host pacing or substitute an estimate.  The independent B3 FDC
drive/media declaration remains with its already recorded source-gated
receiver.
