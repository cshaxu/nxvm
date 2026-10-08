# T546 S24 Model40 Shutdown Wait Regression

## Defect And Repair

After an external wait completed, the Core could continue the same retiring
boundary even when that time advancement had exposed processor shutdown.  The
generic fault-delivery branch then consumed the shutdown indication as an
ordinary non-retiring delivery boundary.  The board never received the
existing shutdown-reset arbitration opportunity and Model40 firmware could
remain on the non-progressing route.

The private Core retirement-wait owner now returns through the run-loop
arbitration boundary after a retiring external wait, and checks shutdown
before generic fault delivery.  The existing board `shutdown_reset` callback
remains the only reset owner.  Without that callback, the machine returns the
existing `WAITING_FOR_INTERRUPT` shutdown result; no clock, reset, input or
second execution path was added.

## Regression And Related Result

The 80386 protected-I/O timing smoke injects a qualified external cycle into
a denied I/O instruction that reaches shutdown.  It proves the Core returns
the shutdown boundary with zero retired instructions and without synthetic
I/O callbacks.  Disabling the shutdown branch makes this regression fail.

The full test replay also exposed a stale IBM PC test oracle: S20 had already
corrected 80286 `LEAVE` to its sourced five-clock row, while the independent
receiver still expected eight.  The receiver now uses five; no timing model
was changed in S24.

## Verification

- Complete repository-only `unit`: **506/506** on x64 (51.62 s).
- Complete repository-only `unit`: **506/506** on x86 (51.60 s).
- Direct Model40 x86 external DOS-installation context: pass (171.85 s),
  within the unchanged 180-second contract.
- `git diff --check`: pass.

## Receiving Artifacts

All are stripped Release 0.5.0546 artifacts built from the current source.

| App | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| My5160 | `B301F0C31EB79F1C65102BA35AEE54A3BE8FDA5FDE1776352E83F2E7B60F6CD0` | `407523DDEE3F2725086F01275CE6B99B9502F3D9E3237D6686AE3D957DF1F0AF` |
| My5170 | `A7BDAFEDB0B75D219275F1E8254145E7738D04D89154A65C0FA14BA1E0DF2034` | `A5C4ABDEA49316636A884239FC1FAB6C876B26CC5427716631F48513518B5F08` |
| MyDeskPro386 | `E2C33D0D67208BD11FF9EA43AC833C708514C52B219DC08F19537C295BEE8455` | `473079B9C700CC5350570B7A341E452421F733FFC185ECDF3D1859F38F07BC8E` |
| NXVM | `C26CD4EFB97B00B86180EAB31B886F89B31E860D4E89CD1278BDF5F6228F6031` | `37A69D61319562616D29F554E6A7677AF0383C7EEED26EC1E0F5DCC0353B1DC6` |
