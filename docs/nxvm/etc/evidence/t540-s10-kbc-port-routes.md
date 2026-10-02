# M5 T540 S10 KBC Port Routes

KBC now publishes its 60h data and 64h status/command endpoints as one
Core-owned typed port-route batch. Core is the sole port-table owner and
publishes both routes or neither. The KBC board attachment still owns the
existing keyboard, auxiliary-device, IRQ, reset and A20 signal wiring; the
8042 and keyboard chips still own their command and BAT behavior. No chip
protocol, reply timing or profile input changed.

The typed read preserves the prior byte-lane behavior: it replaces only the
low byte of the current port value. The typed write passes that low byte to
the same chip function as before. Registration failure destroys the newly
created chips; the Core batch preserves earlier routes without publishing a
partial KBC. The old raw `t_port` callbacks and local registration checkpoint
were deleted rather than retained as a second path.

## Scope and actual-diff review

- Production changes are confined to `devices/{kbc.c,kbc.h,machine.c}`:
  26 added, 36 removed lines (net -10). Tests add 63 and remove 23 lines
  (net +40), including two small owner-local fixtures so port-only KBC and
  DMA tests exercise the same production Core registration path. No Shared,
  MyNES, profile, firmware, INI or media input changed.
- The initializer now receives the Core machine owner for an atomic two-route
  installation. The only production call site supplies that owner. A source
  sweep found no KBC raw `t_port` callback, `core_machine_port_add_*` call or
  registration checkpoint. The remaining KBC functions named `*port` are
  8042 signal-register operations, not a second host I/O registration path.
- Existing KBC construction tests exercise allocation failure, collision
  with an existing 64h route, unchanged pre-existing route, no partial 60h/
  64h registration, chip cleanup and successful retry. Keyboard/auxiliary
  protocol, BAT, serial cadence and machine-level port tests run unchanged
  apart from passing the new owner or using the test-only fixture.
- FDC remains allocated to S11; VADP, HDC, RTC and board port transactions
  to S12; KBC A20/RAM and reset-signal decoupling to S14. S10 does not claim
  the later neutral-Core or IBM-PC board physical move.

## Verification

- Both x64 and x86 unit build trees compile. Focused KBC controller,
  auxiliary, serial-cadence and port-assembly tests pass on both widths.
  Complete repository-only unit suites pass 467/467 per width.
- The T345 source-ownership verifier and negative self-test, NXVM
  documentation governance and `git diff --check` pass.
- All four profile-specific optimized Release pairs were rebuilt in
  `assets/nxvm/<profile>/`. Every x64 file is `pei-x86-64`, every x86 file is
  `pei-i386`, and none has a `.debug` section. Adjacent INIs and MyNES paths
  remain unchanged.

| Profile | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| XT 5160 | `2264CD4E1A2F93F82C8418F8F7FA2369F9735B19F4D003213D71C326AF680000` | `28387950AE14B60111803B52A51E2C4CE797F66EBEB78917B1EA60E36F2B9581` |
| AT 5170 | `C3E208C69240C39110E804BE45FCA8327532345D487B1DA545F5E8E3C07BADEA` | `6DDA21C1B7F78C057822448403FB94B906C206E96EC87AE3A3F584CED9A7C1FF` |
| Model 40 | `2A4C064BFEDB729E456F905147229FAD45DAF2161DA161BFB6A97A5D3BA4A0C6` | `557685B55D991458840C021FB586476146EB99F16F8C862A814DBEE0E29BE800` |
| Default PC/AT | `B2C8F9CF6F30352E2DFBB1A206B18DDDA56D14DAACE59F66EFD8D4917F307605` | `844123292F9E5B1B24F8DA3458C19D4E4DD0623B727C660A75C3BC638029D12A` |

This S makes no new L3/timing claim. The external four-profile integration
gate is reserved for T540 closure.
