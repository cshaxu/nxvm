# T523 S7 Win32 Keyboard Owner Ledger

## Disposition

| Former Core surface | Sole replacement | Boundary retained |
| --- | --- | --- |
| `core/platform/win32/keyboard.*` | `lib/ux/win32/input.*` | VM maps the library's product-neutral event to the existing Core guest-input value. |
| Core-path keyboard smoke | `test/lib/ux_win32_input_smoke.c` | The shared virtual-key, recovered-character and UTF-16 cases remain covered. |
| Core CMake source/test target | `ux-win32-native` and `ux-win32-input-smoke` | Core has neither a Win32 SDK source nor a host-layout conversion target. |

## One Route

The shared native normalizer publishes only `ux_event` values. VM performs the
one product binding from that value to `core_platform_input_event`; it does not
own host-layout conversion, guest keyboard state or a native handle. No second
conversion path remains.

## Verification

- Shared native-key, recovered-character and UTF-16 proof passes
  (`M5:T523:S7:UX-WIN32-KEYBOARD-OWNER:OK`).
- Focused VM host-ingress and Win32 host-action proofs pass.
- Repository-only unit: 311/311 pass.
- The Core and VM source trees contain no host-layout API or former Core/VM
  keyboard normalizer symbol.
