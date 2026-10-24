# T578 S1 — PC110 Video And Board Source Gate

## Scope

This is a bounded real-firmware observation of the admitted external TFT BIOS
and font inputs.  It makes no POST, display, Setup, DOS-boot, release-artifact
or firmware-compatibility claim.

## Frozen facts

The existing `mypc110-real-firmware-post-diagnostic` establishes on both
Windows widths that the normal real image reaches its existing `F000:6060`
checkpoint.  Both x64 and x86 diagnostic CTest executions passed on
2026-10-24.

The x64 `--after-reset` checkpoint executes the actual guest path for eight
million instructions and records exactly one ordinary 8042 reset:

```text
status=0 reason=1 pc=000fea90 executed=8000000 resets=1
c000-header=55aa00eb exceptions=0 c000=0/0 e000=0/0
```

Its bounded final pre-reset retirement sequence ends with:

```text
0040:00004b89  in al,61h
0040:00004b8b  or al,0ch
0040:00004b8d  out 61h,al
0040:00004b8f and al,f3h
0040:00004b91  out 61h,al
0040:00004b93 jmp ...
0040:00005516 mov al,feh ; out 64h,al
```

After reset, the execution point remains in this F-segment loop:

```text
F000:EA86  in al,61h
F000:EA88  and al,10h
F000:EA8A  cmp al,ah
F000:EA8C  je F000:EA86
F000:EA8E  mov ah,al
F000:EA90  loop F000:EA86
```

No C000 or E000 instruction retirement occurs in that post-reset observation
window.  Consequently the earlier proposed `F000:4CA9`/zero-INT-10 checkpoint
is not the next live firmware boundary.

## Source assessment

Open-Source-PC110 documents C000 as the runtime video-ROM/shadow region and
records that the actual board has an EC/ED-configured shadow/cache mechanism.
It does not establish the exact PC110 port-61 refresh-status source or its
period.  PC110-EMU makes its bit 4 toggle per read, but that is an external
emulator behavior, not source proof for a physical board timing contract.

NXVM already has a generic elapsed-tick alternative for the Board port-61
status bit.  Selecting an arbitrary period would make this loop advance, but
would be an L2 compatibility choice with no source-qualified period.  It must
be reviewed as such in the next PC110 receiver, with a regression that proves
the exact guest boundary it advances and does not call the result hardware
exact.

## Disposition

No production source changed.  The diagnostic-only exploration was removed
before delivery because it did not produce a source-qualified owner repair.
The real C000/video-ROM/INT-10 work transfers to the existing blocked
MyPC110 product-and-guest qualification receiver, after a separately admitted
PC110 port-61 L2 timing decision.  No guest-address interception, synthetic
vector/BDA data, borrowed PC/AT ROM, host fallback, second executor, or raw
firmware commit was introduced.

## Verification

- x64: `mypc110.diagnostic/real-firmware-post` passed (6.70 s).
- x86: `mypc110.diagnostic/real-firmware-post` passed (6.69 s).
- x64: explicit `--after-reset` trace passed its existing reset checkpoint
  (29.1 s) and produced the facts above.
- `git diff --check` was clean before documentation delivery.

The x86 explicit eight-million-instruction trace exceeded the interactive
30-second sampling window; it is not represented as passed.  The two-width
standard diagnostic, not a product boot, is the completed proof.
