# M6 T41 S7 Shared Hotkey Text Adapter

## Historical defect and temporary correction

S6 correctly changed KVM hotkey identifier storage to `lib_u8`, but one
Common-to-product callback and two CRT `strcmp` test calls still passed that
storage as `char *`.  Strict MinGW compilation therefore rejected the
pointer-sign mismatch before MyNES could link.

The correction preserved `lib_u8` event storage but used a temporary Common
C-string callback cast.  It is superseded by the end-to-end `lib_u8` callback
contract recorded in [the S8 corrective evidence](../../mynes/etc/evidence/m6-t41-s8-hotkey-byte-boundary-corrective.md).
The Common fixture uses `lib_memory_copy`, so its event setup remains byte
typed.

## Consumer ledger

| Consumer | Disposition |
| --- | --- |
| `src/lib/kvm-base/hotkey.c` | Direct `lib_u8` copy; unchanged. |
| `src/common/session/session.c` | Historical explicit Common callback adapter; superseded by S8. |
| `test/common/control_reconciler_integration_smoke.c` | Direct `lib_memory_copy` into byte storage. |
| `test/lib/win32_presentation_smoke.c` and `win32_keyboard_smoke.c` | Explicit CRT `strcmp` adapters. |
| `test/lib/kvm_component_contract_smoke.c` | `memcpy` byte copy; unchanged. |

## Verification

The three changed regressions pass on both x64 and x86:
`library.win32_presentation`, `library.win32_keyboard`, and
`common.control_reconciler_integration`.  The unblocked MyNES full x64/x86
suite passes 53/53 on each architecture.
