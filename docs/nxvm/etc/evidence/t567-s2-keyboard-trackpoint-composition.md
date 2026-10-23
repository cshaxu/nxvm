# T567 S2 — PC110 keyboard and TrackPoint composition

## Implemented boundary

`x86-nxvm-board-base` now has one explicit
`X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_8042_NEUTRAL` selection.  It reuses the
existing KBC transport, keyboard endpoint and PS/2 mouse endpoint, but binds
only its IRQ services.  The KBC output port has no A20 or processor-reset
sink.  This differs from the existing PC/AT `8042` selection without adding a
second controller, transport, worker, clock or host-input path.

MyPC110 selects this topology while retaining its existing neutral system
facade.  Therefore the composition supplies CPU-visible `60h`/`64h` transport
and AUX delivery but still does not install port `92h`, the high reset alias,
VADP, inking, power, storage or display endpoints.

## Why this is the smallest correct reuse

The source-qualified PC110 facts are an 8042-style DBB on `60h`/`64h`, standard
Set 1/2/3 keyboard streams, and a standard PS/2 pointing device.  They do not
qualify PC/AT output-port wiring from the generic transport to A20 or CPU
reset.  Reusing the existing PC/AT Board adapter would therefore add behavior
the selected hardware evidence does not establish.  Reimplementing KBC/PS2
would duplicate already-qualified protocol owners.

## Direct proof

`mypc110.unit/base-composition` now proves through the real composed Core port
routes that:

- `20h`, `40h`, `60h` and `64h` are present, while `92h` and `3DAh` remain
  uninstalled;
- a selected raw command byte preserves a submitted Set-2 make/break stream;
- AUX identify yields `FA 00`, reporting enable yields `FA`, and a host
  relative report reaches the guest as the standard three-byte packet
  `29 05 FD`.

The focused MyPC110 and X86 NXVM structural/manifests tests pass on x64 and
x86.  Full guest firmware execution, physical IRQ electrical timing, internal
matrix/Fn behavior, inking and power are outside this proof.

## Scope retained for the next receiver

Power MCU protocol, power button, rail state, CPU-visible mailbox, APM,
suspend/wake and inking remain unimplemented.  They have no fabricated route
or successful placeholder response.  T567 S3 must decide whether enough
source evidence exists to create a deterministic guest-visible power model;
otherwise it transfers those rows to a separately queued evidence receiver.
