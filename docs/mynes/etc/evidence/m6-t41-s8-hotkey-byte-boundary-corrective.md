# M6 T41 S8 Shared Hotkey Byte-Boundary Corrective

## Defect

S7 preserved `lib_u8` KVM-event storage but converted it to `const char *` in
Common Session before dispatching the product-owned hotkey callback.  That
cast obscured a mismatched Shared ABI: the callback semantically receives the
same fixed-width byte identifier owned by KVM.

## Correction and same-class sweep

- `common_session_command_provider.handle_hotkey` now accepts `const lib_u8 *`.
- Common forwards `event->data.hotkey.identifier` without a cast.
- MyNES receives `const lib_u8 *`; its one `lib_c_strcmp` call is the explicit
  existing C-string-wrapper boundary.
- NXVM's only Common hotkey provider receives and forwards `const lib_u8 *`
  without a compensating cast.

The sweep found no second KVM byte-text callback.  Console command lines,
Common monitor presentation text, INI paths, and CRT/Win32 calls retain their
documented native C-string boundary; they are not borrowed KVM event payloads.

## Verification

- Fresh MyNES Release builds succeeded on x64 and x86.
- `ctest -R '^mynes\\.'` passed 53/53 on each architecture.
- The static Shared/MyNES/NXVM hotkey callback sweep has no `const char *`
  hotkey receiver, Common event cast, or compensating consumer cast.

The prior S7 evidence remains historical: its callback-adapter statement is
superseded by this corrective record.
