# M6 T41 S7 MyNES Fixed-Width Completion

## Delivered representation

MyNES now keeps INI input, ROM and battery paths, delayed monitor text and
opcode metadata as `lib_u8` storage.  App predicates and parse results use
`lib_bool`; App process results use `lib_i32`.  The opcode table is byte typed
and crosses to `char *` only at the direct `snprintf` varargs boundary.

Common Session command text, Lib file/Base path APIs and Win32 probes retain
their declared C-string signatures.  Every MyNES conversion to such a
signature is local and explicit.  The KVM hotkey callback statement below was
superseded by the fixed-width correction in
[S8 evidence](m6-t41-s8-hotkey-byte-boundary-corrective.md).  The C entry-point
signature remains the required ISO C `int main(int, char **)` adapter.

## Sweep disposition

- No MyNES `lib_c_strstr` receiver remains.
- Internal native-width and raw fixed-width integer declarations are gone.
- Remaining native scalar hits are required `main`, direct CRT format casts,
  direct Win32 parameter casts, or owner-local test environment/CRT probes.
- Command parser text is the direct Common Session C-string callback adapter;
  its persistent delayed-message storage is `lib_u8`.

## Verification

- MyNES x64 Release build and `^mynes\.` CTest: 53/53 pass.
- MyNES x86 Release build and `^mynes\.` CTest: 53/53 pass.
- The Shared hotkey corrective regression that unblocked both builds passes
  on x64 and x86: `library.win32_presentation`,
  `library.win32_keyboard`, and `common.control_reconciler_integration`.
- Lib Types layout/manifest and MyNES documentation governance pass.

## Artifacts

| Artifact | PE architecture | SHA-256 |
| --- | --- | --- |
| `mynes_0_0_0041_x64.exe` | `i386:x86-64` | `C45D6EDA60C46F7A447FA9D072A2ADF9A949F3948C800C3428FC8ECC3154292B` |
| `mynes_0_0_0041_x86.exe` | `i386` | `9B25E4CD050F13FB5FF468A69A76339CE1E7EA2A855405929F1196908E3D2B29` |

The owner-local `assets/binary-mynes/mynes.ini` is intentionally excluded.
