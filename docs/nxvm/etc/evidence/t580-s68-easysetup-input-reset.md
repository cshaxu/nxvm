# M5 T580 S68 — TFT Easy-Setup input/reset semantic boundary

## Scope

This record concerns the unchanged owner-supplied TFT firmware only.  The
diagnostic adds observations to the existing real-firmware test; it neither
changes the delivered keys nor changes guest timing, firmware bytes, CMOS
defaults, or production code.

## Observation contract

The diagnostic begins its Setup-specific observation only after the existing
firmware watchpoint confirms the original loader handoff at `5000:0000`.
For each of the two already-established ordinary Enter make/break injections,
it records subsequent guest reads of keyboard-controller data port `60h`.
It also counts port writes through RTC index/data ports
`70h/71h` after the first injected key.  Cold-POST RTC traffic is deliberately
outside this record.

The diagnostic succeeds only when it observes, in order:

```text
1C 9C 1C 9C
```

and at least one post-input RTC index/data-port write, followed by the firmware-requested
processor reset at `FFFF:FFF0`.

## x64 result

The x64 real-firmware replay reached the existing loader handoff, published
the `640 x 480` Setup frame, consumed both Enter pairs through the ordinary
8042 route, performed 345 post-input RTC index/data-port writes, and requested the fifth
processor reset at `FFFF:FFF0` after 144,068,624 retired instructions; the
diagnostic returned exit code 0:

```text
PC110:SETUP:input-bytes=4 values=1c 9c 1c 9c cmos-writes=345
PC110:SETUP:processor-reset=1 pc=fffffff0 resets=5
```

This proves the selected firmware consumes normal keyboard input and accesses
the RTC index/data ports before a reset. The trace includes writes to port
`71h`; the count also includes index selection and does not establish a
before/after configuration delta. It does **not** identify a menu selection,
prove the intended durable configuration values, prove the next cold boot
consumes those values, or qualify normal Setup completion or guest boot.

Direct inspection of the captured pre-input frames identifies the visible
path more narrowly: the first frame shows an ERROR dialog with `00173` and
`00163`; after the first Enter, the second frame shows the date/time editor.
The second Enter is delivered from that editor. The corresponding PPM SHA-256
values are, respectively:

```text
B9EA224DFEB96227BC89CF2B1FD9C5147324F85767C21B9FE8259DBAD9159B96
FC147A26D8D1211E2C519A19E83D799B4227B716F44DF706EC9A547F6C253C0C
```

These screen observations do not establish why the firmware recorded those
errors or whether the date/time change survives a later boot.

## External comparison

The local PC110-QEMU README's real-BIOS Easy-Setup section describes an
emulator-assisted entry using the extracted image and a forced empty POST
error log. It says a non-empty log presents the error screen, and also records
an image theme adjustment for its assisted entry. This is a comparison and
research lead, not evidence that NXVM follows the same route or that the
observed reset saved a configuration. NXVM's selected ROM and handoff remain
unchanged. The next receiver must inspect the real firmware's error-log service
and screen/input branch before attributing the result to a missing device.

## x86 result and register observations

The same x86 diagnostic returned exit code 0. It reached the same handoff,
consumed `1C 9C 1C 9C`, recorded 345 RTC index/data-port writes, and reached
the fifth reset at `FFFF:FFF0` after 144,068,624 instructions. Both pre-input
frame hashes match the x64 values above.

The x86 trace provides a concrete before/write pair: after the second Enter,
the guest read RTC index `0Eh` as `64h` and wrote `60h` through data port
`71h`. It also wrote the displayed date/time register values before reset.
That establishes guest-issued writes; this diagnostic stops at the reset
request, before proving which values remain afterwards or why the BIOS chose
the two ERROR codes. Its finite trace buffer retains only the first 128 port
records, not the entire 345-write sequence.

## Verification and disposition

The existing diagnostic target built for both host widths. The complete
`unit`-label suite in the current build trees passed 669/669 on each width;
the focused MyPC110 unit selection passed 12/12 on each width. The Setup
diagnostics are direct external-firmware runs, not registered repository-only
unit tests. No product executable or production source changed.

The evidence supports the normal ERROR → date/time editor → reset path and
does not support a keyboard-delivery repair. Main-menu entry, the origin of
the error log, and values consumed after reset remain with the T580 Setup
receiver; no complete production-repair contract is identified here.
