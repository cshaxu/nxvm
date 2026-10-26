# T580 S62 — unmodified Setup palette review

## Disposition

No new diagnostic or production change is required.  The candidate diagnostic
in the S62 packet would have repeated the exact real-BIOS route already
recorded by S57, but without its then-available comparison-only Setup image.
Adding a second, temporary command-line mode would not produce a new
configuration input or repair a demonstrated owner defect.

The S57 final tree is still the current tree for every receiver in that route:

```text
git diff --exit-code 59fa5c354 -- \
  src/x86-nxvm/chips/f65535 \
  src/x86-nxvm/machine/frame.c \
  test/app-mypc110/integration/real_firmware_post_integration.c
```

exits successfully.  The only later commits touching the F65535 path after
the S57 implementation commit are S58's unrelated PCIC work; they do not
change the frame or diagnostic route.

## Existing direct evidence

S57 used the unchanged TFT BIOS, actual F12 input, actual retained-power
restore and the existing `F000:9009E` Setup-entry watchpoint.  It copied the
normal Machine graphics frame and wrote it as PPM; it did not patch the BIOS,
intercept a BIOS PC, replace the renderer or introduce a screen-specific
palette constant.

The final x64 and x86 captures were both 640 by 480.  Their ERROR frame hash
was:

```text
D9B110C0CA4F495127B22FF64E80F38AE075ECEF22D4BDBD478802D09C49AFB5
```

and the dominant programmed background was `0x00BE9E96` (`#BE9E96`, rose),
with 113,730 pixels in the ERROR frame and 177,692 pixels in the subsequent
date/time frame.  The complete source provenance, command-result hashes and
both-width verification are retained in
[`t580-s57-f65535-planar-colour.md`](t580-s57-f65535-planar-colour.md).

The production route remains one owner:

```text
guest F65535 DAC/attribute state
  -> x86-nxvm Machine graphics frame (0x00RRGGBB)
  -> Lib KVM Window frame
```

There is no evidence that this route swaps red and blue.  PC110-QEMU's
direct-loader blue frame is not comparable evidence: its own source states
that it skips the BIOS pre-entry initialization and overwrites the guest
Setup image's palette.  PC110-EMU's presentation palette override is likewise
not an implementation source.  Neither is imported.

## Result and remaining boundary

The reported blue screen is not reproducible from the current source route
with the available retained direct capture evidence.  It may be an older
unpublished preview, a different screen/state, or a desktop/RDP presentation
condition; S62 does not guess which.  No source-backed receiver exists for
normal Setup persistence because the exact extended-CMOS capture remains
absent (S61).  That prerequisite, main Setup/persistence, panel identity and
desktop/RDP validation remain open T580 qualification work.

S62 adds no runnable path, artifact, protected asset, or new test mode.
