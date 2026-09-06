# M5 T523 S3: UX Value Contract And Explicit Presentation Evidence

## Result

`lib/ux` now has one copied-value presentation contract. A text frame carries
its geometry, characters, attributes, palette, primary/secondary glyph sets,
selected font, and CRTC cursor shape/visibility. An indexed frame carries its
dimensions, source stride, dirty rectangle, palette and pixels. The Win32 and
Linux presenters consume those values without borrowing Core or VM memory.

Input values carry physical key transition, Unicode scalar, modifier state and
relative/absolute/wheel/button mouse data. Product action identifiers are
registered values: `lib/ux` asks the product binding to release all of its
tracked inputs, while the native presenter releases capture before invoking
the product callback. It has no NXVM action meaning. The new
`ux_capture` value is the one capture-state owner; Win32 retains only the
native `ClipCursor`/`ReleaseCapture` effect.

The router now stores only an explicit target. Frame content cannot route a
presenter. NXVM retains its Console-to-Window policy in its product binding,
where three text frames may explicitly request Console again.

## Similar-Issue Sweep

- `src/lib` contains no NXVM, SoftPC, NTVDM64, `type.h`, `type_*`, `STD_*`,
  `C_*`, or `TYPE_*` dependency.
- No `UX_DISPLAY_*`, `ux_router_observe`, stable-frame router state or
  product-defined action identifier remains in `src/lib`.
- The previous product-shaped Win32 keyboard injection helpers were deleted;
  NXVM's sole guest-key injection stays in `vm/platform/ux_binding.c`.

## Verification

- The generated direct target commands rebuilt and linked
  `ux-contract-smoke`, `vm-platform-ux-frame-smoke`, and
  `vm-platform-win32-host-action-smoke`; each passes. This route was used
  because the existing Ninja invocation stalls before spawning a compiler.
- `ux-contract-smoke` is table-driven across text and indexed-frame values and
  also checks palette, font, cursor, dirty rectangle, input shape,
  action-before-callback release and capture release.
- Complete repository-only x64 unit replay:
  `ctest --test-dir build/mingw-gcc-x64 -L unit -j 8 --output-on-failure`:
  `309/309` pass in 14.56 seconds.
- `git diff --check` and the public-vocabulary/automatic-route sweeps pass.
