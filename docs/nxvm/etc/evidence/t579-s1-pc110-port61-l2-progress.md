# T579 S1 — PC110 port-61 L2 progress

## Cause and owner

The admitted TFT BIOS makes an ordinary 8042 reset and then executes this
port-61 edge wait before it programs PIT channel 1:

```text
F000:EA86  in  al,61h
F000:EA88  and al,10h
F000:EA8A  cmp al,ah
F000:EA8C  je  F000:EA86
F000:EA8E  mov ah,al
F000:EA90  loop F000:EA86
```

`mypc110_machine_topology` previously selected the Board-AT direct PIT1
output.  That output cannot change before the firmware configures the channel,
so the guest cannot leave this early loop.  The owner is the PC110 App's board
selection, not a Core, emulator, or generic PC device defect.

## L2 decision

The App selects the pre-existing elapsed-tick port-61 source with a three-tick
period.  It is evaluated against Core's existing sole guest tick axis.  The
period is deliberately a finite execution-compatibility choice: the observed
four-instruction polling body does not alias it.  It does **not** claim a
physical PC110 refresh frequency.  PC110-EMU independently synthesizes a
changing bit 4 for the same early BIOS wait; that is supporting L2 research,
not hardware proof.

Parity-memory behavior remains disabled (`memory_bytes = 0`); the change only
supplies the already-configured timer-status read bit.

## Actual real-firmware result

The new `--port61-progress` diagnostic runs the external TFT firmware, waits
for its actual 8042 reset, observes entry to `F000:EA86--F000:EA90`, and stops
only after a later retirement leaves that range.  On 2026-10-24 it produced on
both widths:

```text
status=0 reason=1 pc=000fdd05 executed=6712825 resets=1
exceptions=0 c000=0/0 e000=0/0 port61-wait=1/1
```

This proves the selected source resolves only this gate.  It is not a POST,
video, Setup, DOS boot, product artifact, or physical-timing qualification.
