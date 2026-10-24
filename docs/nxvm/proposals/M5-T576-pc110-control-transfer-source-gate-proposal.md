# MyPC110: protected control-transfer source gate

## Admission

T576 S3 follows the committed data-reload receiver.  With its controlled
watchpoint removed, both the ordinary executor and the admitted TFT image pass
the `F000:605F` data reload, later execute the existing `INT 10h` path, and
shut down through an uninitialized vector without executing C000 or programming
the EC/ED C-shadow sequence.

The failure could arise from an earlier protected-mode control-transfer/stack
frame mismatch, an unmodeled board device, or a firmware mapping condition.
External PC110 emulators have broad compatibility patches, but those combine
code, stack, far-call/return and interrupt behavior.  They are L2 research,
not an implementation specification.

## Objective

Use the existing App diagnostic and Core retirement/fault observations to
identify one first causally relevant control-transfer or stack boundary between
the resolved data reload and the later invalid INT 10 dispatch.  Change
production code only if that one boundary has a generic owner, a minimal
contract and a direct regression.

## Boundaries

- No broad "loose protected mode", address/selector table, hardware-specific
  CPU branch, ROM interception, IVT/BDA repair, fabricated video BIOS, PC/AT
  fallback, second executor or raw asset commit.
- Diagnostics may extend existing App observations only.  They cannot become a
  production execution path or mutate guest memory.
- Code/stack/far transfer semantics remain strict unless the trace isolates one
  complete, generic CPU operation and a bounded L2 contract.

## Verification

Both widths must reproduce the same ordered control/fault facts.  If a repair
is admitted, prove strict behavior remains unchanged and MyPC110 advances past
the exact boundary.  If not admitted, record a finite source/proof stop and
transfer only the minimal next receiver.  Neither outcome is a PC110 boot or
release claim.

## Result

The finite trace isolated an App-owned input-map error before the proposed
control-transfer work: the unshadowed C route selected the image's first bank,
while the TFT image's video firmware occupies its third bank.  The existing
App replacement provider now selects that video bank and retains its existing
EC/ED shadow path.

After this correction, both widths execute the real `CD 10` at `F000:4CA9`.
The ordinary IVT entry at physical `0000:0040` is all zero, and the existing
CPU INT path transfers there.  The traced result rules out a further generic
protected code/stack/far-transfer receiver.  A qualified PC110 display
firmware or board-initialization source is absent from the admitted BYOB asset
set; QEMU's standard-VGA injection is not transferable.  The detailed
evidence is in `docs/nxvm/etc/evidence/t576-s3-pc110-int10-video-boundary.md`.
