# M5 T579 S3 — PC110 LPT-cycle source gate

## Scope

This evidence continues the admitted TFT BIOS diagnostic after T579 S2's
`3BCh--3BEh` LPT checkpoint.  It does not make a PC110 POST, display, Setup,
storage, boot, executable, or hardware-accuracy claim.

## Observed path

The external admitted TFT image executes past the LPT checkpoint at
`F000:55B2`, performs its RTC/CMOS setup through `70h/71h`, and writes the
PC110 `4Fh` index mirror.  It subsequently returns to the already-proven
port-61 bit-4 wait at `F000:EA86--EA90`.

Open-Source-PC110 documents `70h/71h` as the MC146818-compatible RTC/CMOS
pair and describes `4Fh` as the associated extended/index mirror.  That is
research-grade L2 context, not a source import or a claim that every chipset
register is modeled.

## Owner disposition

`x86-nxvm/board-base` already owns the conventional RTC/CMOS route used by
the BIOS.  The diagnostic does not establish that the `4Fh` write blocks,
controls a future wait, or requires a chipset implementation.  Adding a broad
configuration model from this observation alone would be speculative.

The bounded `mypc110.diagnostic/real-firmware-lpt-cycle` test stops when the
same post-LPT execution reaches the previously admitted port-61 loop.  It
passes on x64 and x86 with the external TFT asset.  This provides a finite
handoff: a later receiver must isolate a distinct unowned device or repeatable
failure before changing PC110 production behavior.

## Limits

No Runtime trace is enabled in Release builds, and this evidence does not use
an alternate executor merely to log every I/O access.  It intentionally does
not qualify a BIOS completion, PC110 hardware accuracy, or product artifact.
