# M6 T41 S7 Shared Hotkey Text Adapter

## Defect and correction

S6 correctly changed KVM hotkey identifier storage to `lib_u8`, but one
Common-to-product callback and two CRT `strcmp` test calls still passed that
storage as `char *`.  Strict MinGW compilation therefore rejected the
pointer-sign mismatch before MyNES could link.

The correction preserves `lib_u8` event storage and adds casts only at the
direct C-string callback and CRT assertion boundaries.  The Common fixture now
uses `lib_memory_copy`, so its event setup also remains byte typed.

## Consumer ledger

| Consumer | Disposition |
| --- | --- |
| `src/lib/kvm-base/hotkey.c` | Direct `lib_u8` copy; unchanged. |
| `src/common/session/session.c` | Explicit Common callback C-string adapter. |
| `test/common/control_reconciler_integration_smoke.c` | Direct `lib_memory_copy` into byte storage. |
| `test/lib/win32_presentation_smoke.c` and `win32_keyboard_smoke.c` | Explicit CRT `strcmp` adapters. |
| `test/lib/kvm_component_contract_smoke.c` | `memcpy` byte copy; unchanged. |

## Verification

The three changed regressions pass on both x64 and x86:
`library.win32_presentation`, `library.win32_keyboard`, and
`common.control_reconciler_integration`.  The unblocked MyNES full x64/x86
suite passes 53/53 on each architecture.
