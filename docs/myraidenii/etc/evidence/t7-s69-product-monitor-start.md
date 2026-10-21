# T7 S69 Product Monitor Start Qualification

## Scope

This record qualifies one bounded owner-ROM route only.  It does not retain
ROM contents, guest addresses, traces, screenshots, or third-party source.

## Change

The Windows Console child-process helper moved from the PC-specific
`test/x86-nxvm/setup` directory to `test/emulator/setup`.  Its only public
test function is now `emulator_console_process_run()`: it creates a private
child Console, waits for the monitor prompt, sends supplied monitor text and
checks supplied visible markers.  It has no PC, ROM, board, or Product logic.

The MyRaidenII opt-in smoke uses that one helper to send `start` and `exit`
to the staged real Product.  The existing staging wrapper verifies the owner
archive before copying the selected images, invokes the smoke, and removes
its unique staging child in `finally`.

The current X86 Core memory-route header already requires an explicit
failure observer.  The affected X86-NXVM and DeskPro receiver initializers
now state `LIB_NULL`; this preserves their former no-observer behavior and
keeps strict builds valid.

## Result

On both x64 and x86, the wrapper completed one bounded Product monitor route:
the child reached the standard `Machine started.` result, accepted `exit`,
and the wrapper removed its owned staging directory.  The pre-existing
selected-set Board probe independently establishes that the same verified
set can produce changing frame data.  This S therefore qualifies monitor
start/exit and the Product-to-driver route; it does not claim a captured
Window frame, desktop/RDP input, sound audibility, gameplay, sprite fidelity
or release readiness.

## Verification

- Both width-specific Product smoke targets built.
- Both width-specific external Product smokes passed against the verified
  external archive and cleaned their staging child.
- x64 repository unit suite: 697 tests passed; its cleanup fixture requires
  local junction permission and passed when run with that fixture permission.
- x86 repository unit suite: 697 tests passed.
- Emulator and X86-NXVM ownership, Types and manifest gates pass on both
  widths.

