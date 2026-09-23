# M6 T37 App/Core Quality

## Scope

T37 is the owner-authorized quality and simplification follow-up to M6 T36.
It audits and repairs only MyNes App/Core code and its direct receivers. Shared
Lib/Common sources are not modified; any required neutral-contract change is
recorded as a transfer.

## Progress

- S1 closed in `e1436e0`: App/Core ownership, lifecycle, persistence and
  flattening audit; confirmed Core snapshot and App allocation repairs, plus
  two shared-boundary transfers.
- S2 closed in `0a15968`: its moving-worktree comparison was later found to
  be an invalid import baseline. S3 supersedes that conclusion with a
  committed-tree audit against SoftPC `6251f896`.
- S3 closed: all four transferable roots now hash-identically match SoftPC
  `6251f896`; Common carries its committed nonrunning Window gate. MyNes App
  snapshot and Core battery paths use the restored direct writer contract, and
  their receiver tests use real Lib files rather than duplicate Lib symbols.
- S4 closed: `MNS1` version 2 replaces native C layout with an explicit
  fixed-width little-endian Core codec. It stages machine, mapper and RAM input
  before commit, validates booleans/enums/counts, clears host PCM FIFO delivery
  state and writes MMC3 PRG-RAM at its declared length. The x64/x86 Core,
  transaction and App snapshot tests pass.
- S5 closed: running saves and successful loads now retain one pending App
  result until Common reports the resulting paused runtime fact, matching the
  committed SoftPC command/runtime contract. Paused saves and immediate errors
  retain their direct completion paths.
- S6 closed: the final product-only audit found no remaining App/Core defect.
  The complete sequential x64 and x86 suites each pass all 118 tests; the
  portable snapshot fixture asserts the same fixed bytes on both targets.
