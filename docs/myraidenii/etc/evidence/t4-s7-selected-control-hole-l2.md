# T4 S7: Selected Control Hole L2 Contract

## Admission And Model

The owner authorized bounded L2 behavioral models when no original-hardware
material is available.  External emulator facts remained research-only; no
source, algorithm, table, ROM content or protected trace was imported.

After S6, the selected program faulted at `0x692` on both widths.  The current
MAME RAIDEN II map leaves the enclosing `0x690--0x69f` interval unassigned
between named receivers.  The Board admits only the exact two-byte
`0x692--0x693` write-only, no-state span through its existing selected-control
callback.  Reads, inspection, crossing widths and all other bytes retain their
previous disposition.

During this S, a candidate full-gap model was tested and rejected.  It changed
the established result for a wider access crossing the pre-existing `0x68e`
boundary, so it was removed rather than weakening that failure contract merely
to reduce repeated route declarations.  The final model is therefore narrower
than the external map gap and does not claim a physical-board explanation.

## Verification

The existing Board smoke proves byte lanes, contained word write, mixed-width
refusal, no readback, no inspection, immediate unaffected bytes and actual
V30 word-store execution on x64 and x86.  The opt-in owner-ROM probe retained
its identity validation, transient staging and cleanup; both widths advance
past `0x692` by one instruction and now stop at `0x69a`:

| Host width | Executed | Ticks | Next fault code |
| --- | ---: | ---: | ---: |
| x64 | 68,248 | 1,076,656 | `0x69a` |
| x86 | 68,248 | 1,076,656 | `0x69a` |

`0x69a` is not absorbed by this S.  Complete repository-only units ran after
the final source change: both ordinary runs passed 641/642, with only the
existing restricted-process junction case failing; that one
`repository.build-cleanup` test passed when rerun with its required Windows
capability on both widths, completing 642/642.  Documentation governance and
`git diff --check` pass.  The S-owned temporary staging root was empty after
the probe wrappers completed and was removed before closure.
