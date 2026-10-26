# T580 S66 — Real Easy-Setup Handoff And First-Frame Diagnostic

## Scope

S66 exercises the selected immutable TFT BIOS through the existing MyPC110
real-firmware integration harness.  It answers only whether the guest itself
can finish the ROM-resident Easy-Setup loader, publish a first graphics frame,
and consume the ordinary Enter key path.  It does not claim normal F1 branch
selection, configuration persistence, external-media boot, PC Card behavior,
desktop use, panel-physical identity, or product qualification.

The harness remains diagnostic-only.  No production source, Shared component,
firmware byte, external media, INI, or PC110 artifact changed.

## Scratch-Only Oracle

The external reference repository's existing extractor was run against the
same owner-supplied immutable TFT BIOS solely to create a comparison buffer in
the S66 task scratch directory.  The output was never supplied to guest memory,
committed, or retained after this record.  It was exactly 232,185 bytes with
SHA-256:

```text
7128F7C5D4907A8C7F1D7B8C0D64F8617CDA43464F36599D21ED7BBCCAB8FE8B
```

This agrees with the existing S55 record and is an oracle for this observation,
not an implementation import or runtime dependency.

## Direct Results

The existing `--setup` diagnostic was run once per host width using the
scratch-only reference.  Both executions exited zero and reported the same
guest-visible checkpoints:

```text
PC110:POWER:rsm status=0 executed=1 return=f000:0000dd11 retained=1
PC110:SETUP:entry=1 target=5000:00000000 bytes=232185 matched=1
PC110:SETUP:frame status=0 produced=1 graphics=1
PC110:SETUP:image width=640 height=480 stride=640
PC110:SETUP:enter-delivered count=1 executed=130159997
PC110:SETUP:enter-delivered count=2 executed=140213030
PC110:SETUP:processor-reset=1 pc=fffffff0 resets=5
PC110:POWER:button=1 restore=1 rsm=1 status=0 pc=fffffff0 resets=5
```

Both widths reached the same final count, 144,096,630 executed instructions,
with the established four cold-prefix exceptions and no Core fault.  The first
captured post-handoff graphics frame is 640×480.  The second Enter frame and
the final frame were byte-identical across both widths, SHA-256:

```text
0644016D2EE985038EFC43E069A260768DE91AC98231A7891A5463EBD45F8EDD
```

The pre-first-Enter frame was likewise identical across widths, SHA-256:

```text
B9EA224DFEB96227BC89CF2B1FD9C5147324F85767C21B9FE8259DBAD9159B96
```

The diagnostic injects ordinary make/break Enter events through the existing
Emulator Machine driver and observes their normal 8042 consumption.  It does
not install a host-side BIOS service, inject an Easy-Setup body, rewrite the
palette, or synthesize a firmware transition.

## Disposition

The previously open loader/UI boundary is no longer a candidate CPU, LZW,
keyboard, or base F65535-frame defect: the unchanged BIOS completes it on both
host widths.  The reported blue-versus-pink observation remains a separate
physical/RDP/panel qualification question; this result neither authorizes nor
needs an RGB/BGR, XR-panel-table, or host-palette patch.

Normal Easy-Setup branch selection and persistence, selected storage/guest
boot, PCIC behavior, live desktop input/lifecycle, original DSTN qualification,
and the four published PC110 artifacts remain open T580 receivers.
