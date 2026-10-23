# T567 S1 — PC110 input and power contract audit

## Basis and scope

This audit consumes the PC110 input/power receiver rows without importing any
third-party source or firmware.  The inspected current implementation is the
neutral PC110 base created by T566: it has a 486SX and neutral PIC/PIT/DMA
board substrate, but deliberately has no keyboard, pointer, display or power
endpoint.

The relevant source evidence is read-only:

- `Open-Source-PC110/Discovery/Pluto/readme.md` and
  `pluto-probe-plan.md` record measured CPU-visible `60h`/`64h` keyboard
  decode and an 8042-style M38813 DBB host interface.
- `Open-Source-PC110/Discovery/Keyboard/readme.md` records the M38813 scan
  firmware's standard Set 1/2/3 output tables.  It does *not* prove every
  physical key/Fn matrix mapping.
- `Open-Source-PC110/Discovery/Trackpoint/readme.md` records a live standard
  PS/2 device response: reset `FA AA 00`, ID `FA 00`, and status
  `FA 00 02 64`.
- `Open-Source-PC110/Discovery/Power-Sequence/readme.md` identifies the U6
  M38223 MCU, its board-level power enables and a P2.0/P2.1 handshake.  It
  does not specify a complete CPU-visible command protocol or a lawful MCU
  execution image.

Original-board observations above are the source authority where stated.  The
two external emulator repositories remain L2 reference material only; no
source from either is copied or linked.

## Existing mechanism audit

| Existing mechanism | Scope actually owned | PC110 disposition |
| --- | --- | --- |
| `x86-nxvm/chips/kbc8042` | One 8042-style `60h`/`64h` transport, command byte, OBF/IBF, IRQ routing and keyboard/AUX serial delivery. | Reusable transport candidate: the PC110 CPU interface is measured as 8042-style on the same ports.  It must be composed with PC110 IRQ/reset/A20 sinks, never with the PC/AT board facade. |
| `x86-nxvm/chips/keyboard` | AT keyboard command endpoint and Set 1/2/3 encoding/repeat behavior. | Reusable endpoint candidate for host KVM input after an explicit PC110 adapter selects the MCU-compatible native set.  It is not a claim to emulate the M38813 matrix or undocumented Fn behavior. |
| `x86-nxvm/chips/ps2mouse` | Three-byte PS/2 AUX device protocol. | Reusable, directly corroborated by the measured PC110 TrackPoint reset/ID/status exchange. |
| `x86-nxvm/board-at/kbc` | PC/AT `60h`/`64h` route plus A20, reset and PC/AT IRQ service binding. | Not reusable as a board owner: its contract explicitly owns PC/AT electrical sinks.  A PC110 App-local adapter may reuse the chips but must own only proven PC110 wires. |
| `x86-nxvm/board-base` neutral topology | PIC/PIT/DMA without keyboard endpoint. | Correct current base; it must remain neutral until the App installs a proven input adapter. |

No existing mechanism owns PC110's inking ports `15EAh`/`15EEh`; the same
source says that branch is distinct from `KB_CCS`.  It remains out of scope.

## Finite disposition ledger

| Row | Evidence grade | Next owner | Disposition and proof gate |
| --- | --- | --- | --- |
| `60h`/`64h` host keyboard transport | L3 observed/reverse-engineered | T567 S2, `app-mypc110` | Compose existing KBC transport on those ports with PC110-local IRQ and reset/A20 policy.  Direct test must prove OBF/IBF, command/data distinction, make/break and guest reset without AT facade routes. |
| Keyboard Set 1/2/3 stream | L3 firmware analysis | T567 S2, `app-mypc110` | Use existing canonical host-key mapper/keyboard endpoint only for proven standard scan streams.  Do not map Fn/JP-only matrix behavior without evidence. |
| TrackPoint PS/2 AUX | L3 live probe | T567 S2, `app-mypc110` | Attach existing PS/2 endpoint through the same PC110 KBC transport.  Direct test must prove reset/ID/status and atomic 3-byte movement delivery. |
| Inking pad `15EAh`/`15EEh` | L3 observed distinct decode | Later PC110 inking task | No placeholder port handler; unsupported access remains unclaimed. |
| Power-on/off MCU and rails | L3 board/firmware behavior, incomplete CPU protocol | Later PC110 power task | No always-ready mailbox, host shortcut or Emulator lifecycle alias.  Implement only after a CPU-visible request/reply contract is sourced; current App does not expose a power device. |
| Suspend/wake/APM status | L2/L3 mixed | Later PC110 power task | Requires a documented guest contract and a clock/IRQ owner.  Do not infer it from the physical U6 handshake. |

## S2 boundary

S2 is bounded to keyboard and TrackPoint transport only.  It may add
App-local PC110 composition and direct tests, while using existing X86 NXVM
chips.  It must not alter Lib, Emulator, the PC/AT board adapter, firmware,
power, inking, display or storage.  A PC110 input build remains non-runnable
until later firmware/display/storage qualification.

## Rejected shortcuts

- Do not install the existing PC/AT board facade merely because `60h`/`64h`
  match: its reset/A20 assumptions are not PC110 facts.
- Do not translate host keys straight into BIOS data or use a host key as a
  power button.
- Do not expose a fake power MCU reply or derive a CPU-visible protocol from
  the physical P2.0/P2.1 wires.
- Do not treat the external emulators' convenience stubs as source authority.

## Remaining validation

No physical PC110, protected M38813/U6 firmware execution, Fn matrix proof,
inking input, CPU-visible power mailbox, suspend/wake or guest firmware boot
has been executed by this audit.
