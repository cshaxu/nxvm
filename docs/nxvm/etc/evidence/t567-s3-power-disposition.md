# T567 S3 — PC110 power and suspend/wake disposition

## Result

This receiver does not add a PC110 power, suspend, wake or status device.
The available evidence establishes physical hardware, but it does not establish
one complete guest-visible protocol that can be implemented without inventing
successful behavior.

## Established facts

- `Discovery/Power-Sequence/readme.md` establishes the always-on M38223 U6,
  its P52/P53 rail control, and the physical P2.0/P2.1 handshake.  It also
  records a `5Ah` synchronization byte inside U6 firmware.  These are MCU and
  board facts, not an x86 port or BIOS contract.
- `Discovery/Bowman/readme.md` traces U6's four control lines to Bowman and
  identifies U6 telemetry as serial output.  It says software receives cooked
  status through APM `INT 15h AX=530Ah`.
- `Discovery/PS2/README.md` identifies user-visible suspend, power-off and
  wake controls as APM/BIOS behavior.  It does not supply the complete call
  result registers, state transitions or timing required to emulate them.
- Current `x86-core`, `x86-nxvm` and MyPC110 source own no PC110 APM BIOS,
  U6 serial endpoint or guest-visible power-state adapter.  Existing host
  lifecycle requests therefore cannot lawfully stand in for guest APM calls.

## Why no port model is admitted

The research set contains contradictory historical text about `0ECh/0EDh`:
the PS2 documentation still labels it a U6 power window, while the later U6
analysis explicitly retracts that claim and identifies the pair as the
VL82C420 configuration bank.  The same later document retains an obsolete
indexed-report explanation only as historical hypothesis.  Installing a U6
window at those ports would conflict with the already-admitted chipset route.

The physical P2.0/P2.1 handshake likewise lacks a CPU I/O decode, request
format, response/error contract and wake timing.  It cannot be turned into an
always-ready mailbox or mapped to an Emulator lifecycle request.

## Transfer

The follow-on [PC110 power management and APM](../../proposals/m5-pc110-power-apm.md)
is a source-gated receiver.  It may begin only after read-only BIOS/firmware
analysis or repeatable guest observations establish the exact CPU-visible
interface, including success and failure results, supported suspend/off/wake
states, and the owner of every wake source.  It may use a labelled L2 model
only where a complete reference protocol exists; it must not infer behavior
from physical rail wiring.

No source, test, public interface, asset or executable changes are warranted
by this disposition.
