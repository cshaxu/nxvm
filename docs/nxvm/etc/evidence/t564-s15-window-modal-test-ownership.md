# T564 S15 — Window modal integration test ownership

## Reproduction and diagnosis

The clean x64 T564 qualification initially passed 20 of 21 integration tests.
`lib.integration/kvm_window_modal` failed again when executed alone.  A native
debugger stopped in `kvm_component_fail()` with `LIB_STATUS_IO_ERROR` and Win32
last-error `ERROR_ACCESS_DENIED` (5).  The caller was the test's synthetic
`WM_LBUTTONDOWN` branch:

```text
kvm_win32_mouse_capture
  -> kvm_win32_mouse_refresh_bounds
  -> ClipCursor
  -> RDP desktop rejects confinement
```

The test's intended proof is native Window frame scale, all sizing edges,
maximize/restore, modal-loop admission, control delivery and teardown.  Its
real capture gesture does not contribute to any of those assertions.  It was
also the only capture exercise in this test.

`test/lib/kvm-window/win32/kvm_window_capture_contract_smoke.c` is the correct
owner for mouse capture/release semantics.  It uses controlled native-call
fixtures to prove success, focus/capture loss, registration failure, clipping
failure and release outcomes without depending on a particular desktop's
permission policy.

## Change and retained coverage

S15 removes only from `kvm_window_modal_smoke.c`:

- the synthetic left-button message;
- its test-only capture-observation message and assertion; and
- the now-vacuous explicit release operation.

The test remains registered as an `integration` and `desktop` test.  No
production code, public API, capture contract, test registration or product
artifact input changes.  The Test Lib manifest records the changed source.

## Verification

Focused tests:

| Width | Modal integration | Capture contract |
| --- | --- | --- |
| x64 | passed, 0.51 s | passed, 0.55 s |
| x86 | passed, 0.48 s | passed, 0.59 s |

Clean serial qualification after the correction:

| Width | Unit suite | Integration suite |
| --- | --- | --- |
| x64 | 697/697 passed, 387.74 s | 21/21 passed, 60.76 s |
| x86 | 697/697 passed, 437.50 s | 21/21 passed, 69.61 s |

Both stable Ninja graphs then reported `ninja: no work to do`.  Test Lib's
manifest and NXVM documentation-governance gates pass.  This verifies the
repository suites on the present Windows/RDP environment; it does not claim a
manual physical desktop, Linux, browser or guest-GUI qualification.

## Artifact determination

The change is test-only.  No executable source, link input, version, firmware,
INI or deployment identity changed, so no PC or MyNES EXE is rebuilt or
refreshed.  Existing released artifacts remain the applicable baseline.
