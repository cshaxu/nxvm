# T531 S21 Presentation Contract Corrective Evidence

## Scope

The owner reported repeated cooked-monitor prompts and a text cursor drawn in
the middle of a Window text row. The correction keeps one presentation path;
it does not add an NXVM-specific presentation workaround.

## Findings And Ownership

- `common/session` treated every completion as a reason to arm another monitor
  line. A frame is presentation data, not a monitor-input transition. The
  sole session owner now tracks one pending monitor line, clears it when the
  monitor ceases to be current or consumes a line, and never duplicates it for
  component/broker/frame completion traffic.
- Win32 Console allocated an 80x25 backing buffer without ensuring an 80x25
  visible viewport. The native Console owner now grows only a too-small
  viewport; it preserves a larger caller-owned viewport.
- VADP owns the CRTC maximum-raster-address fact, while KVM owns one 8x16
  glyph asset. The copied CRTC cell height now flows through Core and VM, but
  maps only cursor scan-lines into the stable 16-scan-line glyph cell. It no
  longer changes Window text-frame dimensions or truncates glyph rendering.

## Canonical Corpus Boundary

Before this corrective packet, NXVM and SoftPC had byte-identical `src/lib`
and `src/common` trees. The Common prompt correction and Win32 Console
viewport correction are canonical shared fixes. They use the local revision
`nxvm-t531-s21-p2` until SoftPC adopts the exact files; this task does not
claim whole-T531 corpus identity or closure before that adoption.

## Verification

- `common-test-session_frame`: a Window monitor accepts exactly one pending
  line across startup component/frame completions; raw Console remains unable
  to manufacture monitor text.
- `lib-test-host_console_display`: text surface has an at-least 80x25 visible
  viewport and preserves the existing native-console rollback behavior.
- `core-machine-vadp-text-smoke`: CRTC maximum raster address 7 produces an
  8-scan-line text cell and correct cursor scan-line bounds.
- `vm-machine-frame-smoke`: an 8-scan-line CRTC cursor interval maps to the
  bottom of the stable 16-scan-line KVM glyph cell.
- The x64 unit suite passes 325/325. The full integration invocation is not a
  pass: its early pre-existing/independent failures include timer-firmware,
  paused-Debug, DOS prompt, DOS keyboard and DOS memory-fault probes. S21 is
  therefore a corrective P commit, not a task or subtask closure claim.
- Lib/Common manifests, direct owner tests, required boundary/documentation
  gates, and optimized stripped x64 and x86 `0531` artifact builds are
  recorded with the implementation commit.
