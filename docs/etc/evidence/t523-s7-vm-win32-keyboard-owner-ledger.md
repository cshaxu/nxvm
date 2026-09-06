# T523 S7 VM Win32 Keyboard Owner Ledger

## Disposition

| Former Core surface | Sole replacement | Boundary retained |
| --- | --- | --- |
| `core/platform/win32/keyboard.*` | `vm/platform/win32/keyboard.*` | VM translates host-layout input into the existing Core guest-input value. |
| Core-path keyboard smoke | `test/vm/platform/vm_platform_win32_keyboard_smoke.c` | The same virtual-key, recovered-character and UTF-16 cases remain covered. |
| Core CMake source/test target | `vm-platform` and `vm-platform-win32-keyboard-smoke` | Core has neither a Win32 SDK source nor a host-layout conversion target. |

## One Route

The VM Win32 runner and UX binding use the same relocated normalizer. The
normalizer publishes only `core_platform_input_event` values through the
already-existing VM input sink; it does not own guest keyboard state, add a
native handle to Core, or introduce a second conversion path.

## Verification

- Relocated native-key, recovered-character and UTF-16 proof passes
  (`M5:T523:S7:VM-WIN32-KEYBOARD-OWNER:OK`).
- Focused VM host-ingress and Win32 host-action proofs pass.
- Repository-only unit: 311/311 pass.
- The Core source tree contains no `windows.h`, host-layout API or former
  Core Win32 keyboard symbol.
