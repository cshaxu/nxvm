# T524 S13: UX owner and lifecycle closure

## Result

Each presenter owns exactly one active native render surface and its matching
lease. A Console-mode graphical frame ends the Console presenter, closes the
handles it opened, and returns the product Console before the window presenter
creates its one DIB. A stable text transition reverses that handoff. The
copied mailbox remains the only frame source.

## Removed ambiguity

- Window context is held by `GWLP_USERDATA`; no thread-local current-window
  owner remains.
- Window cleanup releases selected bitmap, DC, DIB, copied frame and window
  through one owner-local path.
- Mailbox `active`, cursor-hidden mirror, ignored action modifiers and the
  stopped-state pause mutation are gone.
- Linux mailbox construction fails if either nonblocking setup fails and keeps
  its wait reference native rather than encoding an fd as `void *`.

## Verification

- Repository unit: 311/311 passed (`ctest -L unit -j8`, 206.47 s under
  concurrent host load).
- Independent `src/lib` build and CTest: 2/2 passed.
- Manifest, documentation governance, Linux adapter hygiene, Linux platform
  contract and current-artifact target gates passed.
- Stripped Release artifacts: `build/output/nxvm_0_5_0524_x64.exe`
  (`8CBB1D7C3425B4EFC350F7BE3398F14B1B2B044C2504E8367B153A7F8CD87F56`)
  and `build/output/nxvm_0_5_0524_x86.exe`
  (`DC7031CA3E3DF4B7800924C12A7DAA7ABE8ADEFBD057E6E992BB26AAEBDF6EAA`)
  passed their optimized-Release and PE architecture checks.

T524 remains open for the owner re-audit and remaining task-level closure.
