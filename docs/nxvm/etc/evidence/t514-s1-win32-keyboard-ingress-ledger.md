# T514 S1: Win32 Keyboard Ingress Ledger

Windows Console `KEY_EVENT_RECORD` carries a virtual key, virtual scan code,
UTF-16 `UnicodeChar`, repeat count and control state. Window input has
`WM_KEY*`, UTF-16 `WM_CHAR`, and UTF-32 `WM_UNICHAR`. The source basis is
[KEY_EVENT_RECORD](https://learn.microsoft.com/en-us/windows/console/key-event-record-str),
[Keyboard Input Overview](https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input),
and [WM_UNICHAR](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-unichar).

The observed RDP soft keyboard failure is now reproducible input evidence: it
can supply a character without a usable physical scan code. Current
`win32con.c` discards `UnicodeChar`; current `win32app.c` ignores `WM_CHAR`
and `WM_UNICHAR`. Both routes only submit scan/virtual-key events, so this
packet is silently dropped before VM.

| Form | S2 disposition |
| --- | --- |
| Usable physical scan (Console or Window) | One normalized physical make/break; preserve extended state. |
| Character-only Console/Window packet | Recover a layout-supported physical sequence, including ordered modifier release; otherwise report unsupported without guest mutation. |
| Physical key followed by its character message | Suppress the character duplicate. |
| Valid UTF-16 pair / `WM_UNICHAR` scalar | Validate first, then recover or report unsupported. |
| Malformed UTF-16, dead composition, unsupported scalar | Explicit no-mutation disposition. |
| F9 | Remains VM product command policy, outside the normalizer. |

S2 replaces the duplicated frontend submission with one Core-platform Win32
normalizer. It emits copied physical transitions to a caller-owned sink only;
it has no KBC, guest-memory, BIOS, profile or session pointer. Console and
Window become thin packet adapters, VM remains the sole sink binder, and Core
machine remains the sole keyboard/KBC owner.
