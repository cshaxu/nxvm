# T531 S21 Presentation Contract Corrective Evidence

## Scope

The owner reported repeated cooked-monitor prompts and a text cursor drawn in
the middle of a Window text row. The correction keeps one presentation path;
it does not add an NXVM-specific presentation workaround.

## Findings And Ownership

- `common/session` treated every completed event alike and rearmed the monitor
  after a frame completion. A frame is presentation data, not a monitor
  command completion. `common_session_process_completed()` now leaves the
  monitor arm state unchanged for `COMMON_SESSION_EVENT_FRAME_COMPLETED`.
- Win32 Console allocated an 80x25 backing buffer without ensuring an 80x25
  visible viewport. The native Console owner now grows only a too-small
  viewport; it preserves a larger caller-owned viewport.
- VADP already owns the CRTC maximum-raster-address fact, but the Core-to-VM
  copied-frame route substituted a 16-scan-line font height. That fact now
  flows unchanged from VADP snapshot through the Core guest frame and VM
  display event to the KVM frame. Values outside the supported 1--16 glyph
  scan-line range fail explicitly rather than silently render as 16.

## Canonical Corpus Boundary

Before this corrective packet, NXVM and SoftPC had byte-identical `src/lib`
and `src/common` trees. The Common prompt correction and Win32 Console
viewport correction are canonical shared fixes. They use the local revision
`nxvm-t531-s21-p1` until SoftPC adopts the exact files; this task does not
claim whole-T531 corpus identity or closure before that adoption.

## Verification

- `common-test-session_frame`: raw-Console and Window monitor-current frame
  completion cannot produce a prompt; a component completion still can.
- `lib-test-host_console_display`: text surface has an at-least 80x25 visible
  viewport and preserves the existing native-console rollback behavior.
- `core-machine-vadp-text-smoke`: CRTC maximum raster address 7 produces an
  8-scan-line text cell and correct cursor scan-line bounds.
- `vm-machine-frame-smoke`: the copied 8-scan-line cell and cursor bounds
  reach the KVM frame unchanged.
- The x64 unit suite passes 325/325. The full integration invocation is not a
  pass: its early pre-existing/independent failures include timer-firmware,
  paused-Debug, DOS prompt, DOS keyboard and DOS memory-fault probes. S21 is
  therefore a corrective P commit, not a task or subtask closure claim.
- Lib/Common manifests, direct owner tests, required boundary/documentation
  gates, and optimized stripped x64 and x86 `0531` artifact builds are
  recorded with the implementation commit.
