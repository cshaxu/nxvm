# T546 S24 Model40 Shutdown Wait Regression

## Defect And Repair

The generic fault-delivery branch could consume a processor-shutdown
indication as an ordinary non-retiring delivery boundary.  Shutdown is now
resolved at the two actual completion points: immediately after a fault with
no external wait, and after a non-retiring external wait completes.  The
existing board `shutdown_reset` callback remains the only reset owner.
Without that callback, the machine returns the existing
`WAITING_FOR_INTERRUPT` shutdown result; no clock, reset, input or second
execution path was added.

The initial S24 implementation also added a redundant run-loop turn after
every successful external wait.  It did not contribute to shutdown handling
and is removed: the established wait owner directly completes the retiring
instruction after its final external tick, as it did before S24.

## Regression And Related Result

The 80386 protected-I/O timing smoke injects a qualified external cycle into
a denied I/O instruction that reaches shutdown.  It proves the Core returns
the shutdown boundary with zero retired instructions and without synthetic
I/O callbacks.  Disabling the shutdown branch makes this regression fail.

The full test replay also exposed a stale IBM PC test oracle: S20 had already
corrected 80286 `LEAVE` to its sourced five-clock row, while the independent
receiver still expected eight.  The receiver now uses five; no timing model
was changed in S24.

## Current Verification Status

- The direct 80386 protected-I/O shutdown/external-wait regression passes on
  both x64 and x86 after the narrowed repair.
- The current x86 Model40 three-context integration group passes its Console
  and CMOS cases, but its DOS-installation context reaches the unchanged
  180-second host budget before its terminal marker.
- An isolated S20 (`8aef7bea4`) x86 executable, using the same current media
  and INI, also reaches that same 180-second budget without its terminal
  marker.  It reaches approximately 382.7 million guest ticks; current source
  reaches approximately 388-391 million.  The fixed host budget therefore
  cannot presently distinguish this S24 repair from the accepted S20 baseline.
- A one-off 300-second current diagnostic run reaches `installer-running`.
  This is diagnostic only, not a relaxed integration result.
- The x86 manifest and `git diff --check` pass.  Full units, complete external
  matrix and artifact qualification remain required before S24 can close.

## Receiving Artifacts

The following hashes are the prior provisional S24 artifact record.  Current
candidate artifacts are unaccepted until fixed-budget qualification can be
completed.

| App | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| My5160 | `B301F0C31EB79F1C65102BA35AEE54A3BE8FDA5FDE1776352E83F2E7B60F6CD0` | `407523DDEE3F2725086F01275CE6B99B9502F3D9E3237D6686AE3D957DF1F0AF` |
| My5170 | `A7BDAFEDB0B75D219275F1E8254145E7738D04D89154A65C0FA14BA1E0DF2034` | `A5C4ABDEA49316636A884239FC1FAB6C876B26CC5427716631F48513518B5F08` |
| MyDeskPro386 | `E2C33D0D67208BD11FF9EA43AC833C708514C52B219DC08F19537C295BEE8455` | `473079B9C700CC5350570B7A341E452421F733FFC185ECDF3D1859F38F07BC8E` |
| NXVM | `C26CD4EFB97B00B86180EAB31B886F89B31E860D4E89CD1278BDF5F6228F6031` | `37A69D61319562616D29F554E6A7677AF0383C7EEED26EC1E0F5DCC0353B1DC6` |
