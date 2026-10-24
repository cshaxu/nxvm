# T577 S1: MyPC110 protected-reset source gate

## Scope and source grade

This record uses the admitted external TFT BIOS/font inputs only at build and
diagnostic time.  Their raw bytes remain under `nxvm-assets`; none are stored
in this repository.  PC110-EMU and PC110-QEMU were read as L2 research, not
copied into production code.

The task began from the hypothesis that the `F000:6038` data-reload sequence
ended in an unmodelled CPU shutdown/reset.  That hypothesis is rejected by the
bounded two-width trace below.

## Observed reset boundary

The App-owned diagnostic's `--after-reset` checkpoint ran the ordinary Core and
Board executor for eight million instructions on both x64 and x86.  Both runs
reported:

- `status=0`, `reason=1` (the configured instruction budget), one reset and
  no delivered fault;
- reset-visible C000 bytes `55 aa 00 eb`; and
- the final pre-reset instruction at `0040:5516`, bytes `b0 fe e6 64`.

Those bytes load `FE` and write it to port `64h`.  The reset is therefore a
guest-issued 8042 reset command, delivered by the existing board/KBC contract;
it is not a CPU shutdown caused by the preceding RET.  The diagnostic retains
the final sixteen retirements so future source-gated work can reproduce that
classification without a second execution path.

## App-owned repair

Two board facts were then required for ordinary progress after the bounded
data-reload checkpoint:

1. The PC110 profile must use its existing full 8042 topology, with output
   port `03h`: system reset released and A20 asserted.  The prior neutral
   topology left A20 masked while firmware entered protected mode and began
   extended-memory probing.
2. The admitted firmware probes exactly the first 64 KiB above the fixed
   20 MiB RAM end (`01400000h`).  The Core otherwise reports an unmapped-memory
   fault there.  The PC110 Board now models only that measured probe span as
   an L2 `FFh` open-bus region: reads return `FFh`, writes are ignored.  It is
   not a broad unmapped-memory policy or a Core exception.

`mypc110.unit/base-composition` proves reset A20 is asserted, the last real
RAM bytes remain writable, and the bounded absent window reads `FFh` and
discards a write.  The same probe after these changes reaches later firmware
at `F000:EA90` without an exception.

## Verification boundary

- x64 and x86 `mypc110.unit/base-composition`: pass.
- x64 and x86 `mypc110.diagnostic/real-firmware-post`: pass at the retained
  `F000:6064` protected-data checkpoint.
- x64 and x86 `--after-reset`: pass as described above.

This classifies the reset and removes the immediate A20/absent-memory blocker.
It does not claim complete POST, display initialization, Setup, DOS boot, a
PC110 release artifact, or physical accuracy of the L2 open-bus response.
