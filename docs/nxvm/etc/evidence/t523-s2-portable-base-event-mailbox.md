# M5 T523 S2: Portable Base And Event Mailbox Evidence

## Result

`src/lib/base/base.h` is the sole public library vocabulary for fixed-width
values, booleans, atomics and status values.  `lib_status` is an integer ABI
with one stable set of `LIB_STATUS_*` values; it does not expose NXVM's
`type_status` enum.  There is no compatibility header: library implementation
files use direct C and `lib_*` spellings, so the reusable source corpus carries
no NXVM legacy vocabulary at all.

The native wake capability is private as well: `mailbox.h` exposes copied
frame publication/capture and generation only.  The Win32 event and Linux
pipe descriptor are available solely through their respective native mailbox
subdirectories, so a product binding cannot mistake either for a portable
public handle.  This batch is a net `+253` library source lines, chiefly the
small base vocabulary and the two native wake implementations; it deletes the
fixed presentation polling paths instead of retaining a compatibility route.

Every `src/lib` include of `type.h` is removed.  The five library targets no
longer link `type-facade`; their public headers depend only on `lib/base` and
the C standard library.  The limited product comparisons made visible by the
new status ABI use the library constants directly; no adapter or duplicate
status route was added.

## Mailbox Wake Ownership

`ux_mailbox` owns one native wake object.  Publishing the latest copied frame
signals it after publishing its generation.  On Win32 this is an auto-reset
event; Console waits on its input handle and that event indefinitely, and
Window waits on the same event together with its message queue indefinitely.
There is no 250 ms Console frame poll, 16 ms Window frame poll, or Window
frame timer.  Linux uses one non-blocking pipe for the identical latest-frame
wake contract; its Console loop polls stdin and that descriptor without a
periodic repaint sleep.  Host synchronization's explicit `Sleep` remains a
separate host capability and is not a presentation-frame route.

The library owns the wake mechanism; the product still owns binding callbacks,
machine state and presentation policy.  No native handle appears in public
frame, input, router or presenter value contracts.

## Verification

- Full x64 rebuild: `cmake --build build/mingw-gcc-x64 -j 4`.
- Focused library tests: `unit.ux-contract-smoke`,
  `unit.ux-linux-input-smoke`, `unit.ux-mailbox-wake-smoke`, and
  `unit.ux-library-manifest` pass.
- Full repository-only unit replay after that rebuild:
  `ctest --test-dir build/mingw-gcc-x64 -L unit --output-on-failure -j 4`:
  `309/309` pass in 17.20 seconds.
- `Verify-DocumentationGovernance.ps1` and `git diff --check` pass.
- Static sweeps find no `#include "type.h"`, `type_*`, `STD_*`, `compat.h`,
  NXVM, SoftPC, or NTVDM64 reference in `src/lib`.
