# T531 S9 Common API And XASM32 Cleanup

## Removed parallel surface

- Removed `common_ui_set_mouse_capturable()` and `common_ui_release_mouse()`.
  They had no NXVM or read-only SoftPC consumer; Common UI now sends the same
  two existing action kinds through its sole `common_ui_apply_action()` route.
- Removed `common_session_set_target()` and its unconfigured-presentation
  fallback.  It had no production consumer and duplicated immutable
  presentation policy.  A Session now rejects `begin_run()` until the one
  policy is configured.

## Retained contracts and XASM32 repair

- The public Common Session and Machine headers now state the borrowed
  UI/driver-context lifetime and the sole-reducer versus copied-publisher
  rule.  No ownership, worker or queue was added.
- `dasm32` formats pointer-backed context text with its real fixed capacity,
  removing two x86 truncation diagnostics without changing output grammar.
- `aasm32` tests DR/TR operands through their own parsed fields.  The MOV
  table's two unreachable TR-read rows now select `ARG_R32_TR6/TR7`, rather
  than duplicated DR rows.  Its table-driven dispatch and existing address-
  size prefix convention are retained.  The smoke covers DR0 and TR6 in both
  transfer directions.

## Proof

- Whole NXVM and read-only SoftPC source/test sweep found no remaining caller
  or declaration of the three removed public APIs.
- Common manifest verification and clean standalone Common builds passed on
  x64 and x86 with zero XASM32 diagnostics.
- Focused Common Session/UI/adapter/XASM tests passed.
- Full repository-only unit suite: 299/299 passed, `ctest -L unit -j 8`, real
  time 20.95 seconds.
- Optimized stripped artifacts were built and architecture-verified:
  `nxvm_0_5_0531_x64.exe` SHA-256
  `E393B7FC6ACEA1E19C0CB0CAF0433B246C977710D53AB54A6A0EFB0FC597BC7A`;
  `nxvm_0_5_0531_x86.exe` SHA-256
  `E96F23435764934289A9D820FEBCCB5F6700C23A7A8AAF76EF0C9127187AEC33`.
  Each exists identically under both `build/output` and `assets/sessions`.

T531 remains open: S9 does not claim the outstanding SoftPC CCPU-affine
paused-Debug adapter or two-product execution proof.
