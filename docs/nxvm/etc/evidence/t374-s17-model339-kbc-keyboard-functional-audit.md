# T374 S17: Model-339 8042 And Keyboard Functional Audit

## Selected Boundary And Sources

The frozen Model-339 ledger selects a 101-key AT keyboard and IRQ1, and
explicitly excludes AUX/mouse and IRQ12.  IBM's March 1986 *Personal Computer
AT Technical Reference* is the controller/board authority; it documents the
8042 keyboard-controller status, command/data ports and keyboard interface.
The selected 101-key keyboard's native scan-code-set requirement is not fully
specified by that board manual.  The owner-approved 86Box source was therefore
consulted only as a behaviour cross-reference, not copied or imported: its
AT-keyboard model keeps keyboard scan-code selection separate from KBC-side
translation.  This agrees with the documented AT keyboard protocol distinction
between native set 2 and the 8042's set-1 translation path.

No source, firmware, ROM, media, binary, or source-derived text was imported.
No command-response, typematic, serial or IRQ service duration is claimed.

## Functional Matrix

| Boundary | Current owner and observable path | Model-339 disposition |
| --- | --- | --- |
| `60h`/`64h`, command byte and output port | `kbc.{c,h}` owns command/data routing, status bits, D0h/D1h output-port/A20/reset path, controller self test and keyboard enable/disable. | Selected and present. S3 already closes F0h--FFh pulse reset without a duration. |
| FIFO, OBF/status and IRQ1 acknowledgement | KBC owns the mixed-origin FIFO. Reading `60h` removes the current byte and deasserts its origin's source before promoting the successor through the PIC owner. | Selected keyboard origin and IRQ1 route present; AUX-origin assertions are excluded evidence. |
| Keyboard command/status/reset surface | KBC owns LED, typematic, echo, identify, scan enable/disable/default/reset and resend replies, plus command-response ownership under a full FIFO. | Selected command/state/reset paths exist and are covered by the KBC smoke. |
| Host input and firmware consumption | VM mapper emits Set-1 bytes directly into the KBC FIFO. Generated firmware IRQ9 handler and INT 16h consumer consume Set-1 bytes. | Existing BIOS route works only with Set-1-visible bytes. No external ROM is used. |
| Native keyboard scan set and 8042 translation | KBC stores only `CORE_MACHINE_KBC_SCAN_SET_1`; `F0h 00h` reports set 1 and `F0h 02h` returns RESEND. Host mapping bypasses a native-keyboard-to-KBC translation boundary. | **Concrete functional gap.** The selected 101-key AT keyboard requires native Set 2 support and correct controller translation/untranslated publication. |

## Owner And Similar-Issue Sweep

The audit searched every KBC command decode, pending parameter route, FIFO
origin/status bit, IRQ source assertion/deassertion, reset/finalize route,
host mapper, profile route, generated keyboard IRQ/INT16 consumer and focused
test.  `kbc.c` is the sole controller/FIFO/IRQ owner.  The VM mapper is a
host-to-keyboard-input producer, not a second guest FIFO.  The profile selects
IRQ1; the generic IRQ12/AUX fields and tests are excluded from Model-339 proof.

No remaining selected command, status, FIFO, reset or IRQ1 defect was found
outside the scan-set/translation mechanism.  Physical serial transfer,
typematic and command-response durations remain T375 timing work.

## Focused Replay

The local MinGW build rebuilt and replayed, without ROM or media input:

```text
M5:T227:S3:KBC-CONTROLLER:OK
M5:T226:S3:SET1-MAPPER:OK
M5:T353:S2:PCAT-TOPOLOGY:OK
M5:T366:S5:MODEL339-COMPOSITION:OK
```

These markers establish existing controller, mapper and selected topology
boundaries; they do not prove native Set 2 or translation.

## Next Repair Selection

T374 S18 must introduce one KBC-owned native keyboard scan-set and translation
mechanism.  It must keep the VM mapper as a host producer, permit `F0h 00h`
query and supported set selection, translate selected native Set-2 make/break
sequences to the existing firmware-visible Set 1 only when command-byte
translation is enabled, and preserve untranslated output when it is disabled.
It must retain FIFO ordering, IRQ1 acknowledgement, reset/default and all
existing Set-1 consumer behavior.  AUX/IRQ12, undocumented per-key modes and
all timing values remain excluded.
