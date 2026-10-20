# M1 T4 S13 — CRTC Write Extension L2

## Scope

The selected external-program startup probe reached `0x640` after S12's
accepted CRTC aperture.  This record describes only the exact extension needed
to classify that stop.  It neither describes CRTC register semantics nor
claims physical board timing.

## Evidence and disposition

- The accepted S12 implementation provides reset-cleared, read/write CRTC
  storage at `0x600--0x63f`.
- A separately inspected FBNeo Raiden II driver routes CRTC **writes** through
  `0x600--0x64f`.
- The previously recorded MAME map only establishes read/write ownership
  through `0x63f`.  The references therefore do not establish reads or inspect
  behavior for `0x640--0x64f`.

The admitted L2 contract is consequently asymmetric:

| Board range | write | read / inspect | state owner |
| --- | --- | --- | --- |
| `0x600--0x63f` | accepted | accepted | Video CRTC file |
| `0x640--0x64f` | accepted | refused | same Video CRTC file |
| immediate neighbours | refused | refused | none |

The Video file grows from 64 to 80 reset-cleared bytes.  This is storage for
the existing owner, not generic Board RAM, a second CRTC, or an inferred
register decode.  No external implementation, table, ROM value, trace or
algorithm is copied.

## Required proof

- Direct Video bounds and reset coverage prove the 80-byte storage extent.
- Board coverage proves normal CRTC read/write behavior is retained, extension
  writes preserve all byte lanes, extension reads/inspection refuse, and
  `0x650` remains refused.
- The bounded owner-supplied selected-set probe must advance past `0x640` on
  both host widths to the same next stop.
- Focused Board tests, complete dual-width repository units, documentation
  governance and whitespace checks qualify the committed result.  They do not
  qualify gameplay, rendering, desktop/RDP behavior or hardware accuracy.

## Result

The focused Board test passes on both widths.  The owner-supplied selected-set
probe advances identically on x64 and x86 from `0x640` to `0x658` after
71,404 executed instructions and 1,132,130 nominal ticks.  Its only retained
output is the neutral next fault address; it retains no program payload.

Complete repository suites execute all 669 registered tests on both widths,
with no failures or skips.  Documentation governance and `git diff --check`
pass.  The transient probe staging and CTest logs were removed afterward;
only the persistent build caches remain.  This result validates the narrow
write receiver only.  It does not qualify the next receiver, CRTC rendering,
hardware timing or playable-game behavior.
