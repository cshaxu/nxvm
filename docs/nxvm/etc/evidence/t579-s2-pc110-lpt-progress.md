# M5 T579 S2 — PC110 LPT probe progress

## Decision

The real external TFT BIOS reaches a finite legacy-port probe after the
already-qualified port-61 wait.  Its code writes `21h` to `3BEh` then expects
bit 6 from `3BDh`; an unowned read remains zero and returns to the wait path.

MyPC110 owns a small L2 placeholder for `3BCh--3BEh`.  It retains only the
control byte needed by that probe: data reads return `FFh`, status returns
`90h` normally and `D0h` when control is `21h`, and control reads/writes use
the retained byte.  This behavior is independently implemented from the
PC110-EMU source observation, so it is L2 research evidence, not a claim
about a real PC110 printer device.

## Real firmware checkpoints

Both release-width diagnostics start from the admitted external TFT image,
perform the ordinary 8042 reset, leave the port-61 wait and retain 32 ordinary
retirements beyond it.  Both reports reach the same final retained path:

```text
F000:4C52  call F000:55AD
F000:55AF  mov dx,0190h
F000:55B2  out dx,al
```

The run executes 6,713,081 instructions, reports one reset and no delivered
exception.  It neither enters C000/E000 nor proves POST, panel output, Setup,
boot media or guest operating-system execution.

## Verification

- `mypc110.unit/*`: 7/7 passed on x64 and x86.
- `mypc110.diagnostic/real-firmware-port61-follow`: passed directly on x64
  and x86 with the checkpoints above.
- Existing `real-firmware-port61-progress` remains the prior S1 checkpoint.

The follow diagnostic is deliberately a diagnostic CTest, not an ordinary
unit test; it needs the owner-managed external firmware asset.
