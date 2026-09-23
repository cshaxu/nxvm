# M3 T17: Product Code And Release Qualification

T17 receives R11/R12 after Window-first acceptance. It owns current product
build/test evidence and may repair project-owned build integration. Adopted
`src/lib`, `src/common`, `test/lib` and `test/common` sources remain unchanged.

## S1 x86 Capture-Contract Qualification

The inherited x86 `library.kvm_window_capture_contract` receiver reproduced a
Segfault by itself. GDB locates the failure in `__chkstk_ms` while entering the
imported test's `check_surface_damage`: the fixture puts a maximum-size
`kvm_window_frame` (about one MiB) on the process stack before it invokes Lib
behavior. This is an x86 MinGW test-host stack reservation issue, not a product
or Lib runtime failure.

The root CMake now gives only that imported x86 MinGW test executable a 4 MiB
stack through `target_link_options`. It leaves all adopted sources byte-for-byte
unchanged and does not affect either MyNes executable. The isolated receiver
passes on x86 and x64 after this host configuration change. Full x64 evidence
from T16 is 102/102; x86 full-suite evidence is being refreshed by this task.

## S2 Strict Product And Artifact Evidence

Root build policy now applies `-Wall -Wextra -Wpedantic -Werror` to every
App/Core target. The first strict build found a dead `pattern` calculation in
`core_ppu_sprite_sample`; sprite pattern bytes are already fetched and cached by
the preceding fetch stage, so the sampling path now uses that cache directly.
Both x64 and x86 strict builds pass. MinGW product executables are linked with
`-s`; `objdump` reports PE x86-64 and PE i386 respectively without debug or
symbol flags.

The final strict JUnit suites each report 102 tests, zero failures and zero
disabled tests. Current SHA-256 identities are:

| Artifact | SHA-256 |
| --- | --- |
| `mynes_0_1_0011_x64.exe` | `B1D709488F53BD0EEFEA33F5BAD2719D2A8F9CD3DA2C62AD3D2E783E64815C4D` |
| `mynes_0_1_0011_x86.exe` | `0F4D610F0B90D7B32DD112433CA81E935EB8F0FCE3829E816A3D118D3355716A` |

T17 accepts R11/R12 and transfers only Console presentation and final
reconciliation to the remaining M3 candidates.
