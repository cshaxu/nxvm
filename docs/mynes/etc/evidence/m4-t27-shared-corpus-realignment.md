# M4 T27 Shared Corpus Realignment Evidence

## Source And Ownership

SoftPC was read at `a2fbf5488cd0a768cbb5dc92d6e23706bf86b32e` and was not
modified. A recursive file-set and SHA-256 comparison found no difference in
the imported roots:

| Root | Files | Lines | Result |
| --- | ---: | ---: | --- |
| `src/lib` | 97 | 8,846 | byte-identical |
| `src/common` | 23 | 3,111 | byte-identical |
| `test/lib` | 49 | 6,976 | byte-identical |
| `test/common` | 20 | 2,817 | byte-identical |

MyNes owns the adapter in `src/core/driver.c`, App lifecycle mapping, and
product tests. No product-specific file remains in a shared root.

## Adapter And Similar-Issue Sweep

The import removed the former Lib audio ABI used by `core_driver_drain_audio`:
the old API exposed i32 samples, queue count and flush; SoftPC exposes i16 PCM
frames plus query/enqueue/clear. The full tracked product scope was searched
for the retired names `lib_audio_stream_queued_samples`,
`lib_audio_stream_flush`, `sample_format` and `capacity_samples`. Production
hit: Core driver, fixed with explicit saturating i32-to-i16 conversion. Test
hit: `test/core/owner_rom_probe.c`, fixed through the neutral query result.
No other MyNes production hit remained.

The lifecycle sweep covered every MyNes state sink and product integration
test that maps `COMMON_MACHINE_RESET_COMPLETED`. App composition and the native
Window integration fixture are the only product paths. They map only a
stopped-window reset selected by App to `PAUSED`; normal `start` retains reset
completion so its automatic resume flow is unchanged. The focused Window,
Console and cartridge-command regressions pass on both architectures.

## Verification

| Check | x64 | x86 |
| --- | --- | --- |
| Configure and Release build | Passed | Passed |
| Registered CTest corpus | 112 / 112 passed | 112 / 112 passed |
| Native Window/Console and media focus | Passed | Passed |
| Shared-root SHA-256 comparison | Passed | Passed against one source pin |
| Documentation governance | Passed | Passed |
| ROM policy | `assets/roms/*` ignored; no tracked ROM | same repository state |

## Artifact And Delivery

| Artifact | SHA-256 |
| --- | --- |
| `assets/binary/mynes_0_1_0011_x64.exe` | `7275964075A5E0EBC58796B699D47230E2F42060664F48C118B707A37106837D` |
| `assets/binary/mynes_0_1_0011_x86.exe` | `EC6D6FD700FFC1056E58FFB190B566B3C043152169914536000EFA560FA0FE93` |

P1 admission is `ec70865`; P2 implementation is `8a2e9de`. No remote is
configured. P commits are local under the approved remote-aware delivery rule;
no push can be performed.

## S2 Initial Prompt Correction

The reported reproducer was a no-ROM startup followed immediately by
`rom insert ../roms/smario1.nes`; Session still held `INIT`, so App rejected
the insertion. App now relays `common_machine_state_get()` only after binding
the state sink and withholds its first cooked prompt until that actual stopped
completion is reduced. Loaded-ROM startup remains reset-driven. The App command,
media and native Window tests pass on x64 and x86. This repair touches only
MyNes App, product test and rebuilt artifacts; all four shared roots remain
unchanged.

## S3 Core Input And Host-Audio Boundary

Commit `849fb44` changes only MyNes Core/App/design/test/artifact paths. The
Core driver consumes KVM neutral keys, with distinct per-source held bindings
for left/right Shift, Ctrl and Alt as well as a scan-less virtual/RDP variant.
It aggregates those bindings into NES button levels, proving that releasing one
physical side preserves the other. `J`/Ctrl maps to B, `K`/Alt maps to A and
either Shift maps to Select; the existing WASD, Enter and Esc contracts remain.

`core_driver_create` now degrades an unavailable native stream (`IO_ERROR` or
`UNSUPPORTED`) to no audio sink, while reset/heartbeat guard that absent sink.
This is a host-availability boundary, not a change to Lib's audio ABI. Focused
x64/x86 Core input tests passed, as did x64 imported `library.win32_keyboard`
and `library.kvm_keyboard_lifetime`. The executable artifacts were rebuilt.
Audible quality is deliberately not claimed; S4 owns the later upstream
`lib/audio` refresh and its integration review.
