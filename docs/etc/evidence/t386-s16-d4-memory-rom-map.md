# T386 S16: Model 40 D4 Memory, ROM Replacement, And Diagnostic Map

`M5:T386:S16:D4-ROM-MAP:OK`

`M5:T386:S16:D4-REPLACEMENT:OK`

`M5:T386:S16:D4-RESET-ALIAS:OK`

## Source Decision And Scope

The transient primary Compaq D3PE processor-board description identifies one
read/write-decoded byte at `80C00000h`: reads return D4 diagnostics and writes
control ROM replacement and ROM-space write protection.  Replacement is
low-active at bit 0, write protection is low-active at bit 1, and the reserved
control bits are written as one.  At power-up both functions are disabled.

The same source distinguishes two spaces that earlier historical records had
conflated.  `FE0000h`--`FFFFFFh` is 128 KiB of D4 RAM that provides the
compatibility system-ROM space.  Firmware is responsible for copying a ROM
image there; S16 does not manufacture such a copy.  When replacement is
enabled, that RAM instead decodes `E0000h`--`FFFFFh`, replacing the system ROM.
This source-backed split preserves S14's separate immutable Rev-E two-chip
carrier, `F0000h` mirror, and 80386 reset alias.

No scan, firmware image, vendor byte, local path, hash, media, third-party
source, or catalogue is retained in Git.  The D4 diagnostic implementation
reports the selected 1 MiB/no-2-MiB/no-parity-fault state; D4 parity producers,
status latching and IOCHK interaction remain a later functional receiver.

## Ownership And Implementation

`vm/profile/model40` owns the D4 physical addresses, 128 KiB backing, control
bits, selected diagnostic value, replacement selection, write-protection
policy, and reset state.  It registers the following three VM-owned devices:

| Range | Function |
| --- | --- |
| `E0000h`--`FFFFFh` | Replacement RAM only while D4 control bit 0 is clear; otherwise decline to the lower immutable-ROM/RAM route. |
| `FE0000h`--`FFFFFFh` | Always-visible D4 compatibility RAM; write-protect applies when control bit 1 is clear. |
| `80C00000h` | Read diagnostic / write control byte. |

Core gains no Model-40 knowledge.  Its generic configuration-time memory-device
registration surface accepts composition-owned callbacks; frozen routing tries
registered providers in order and permits `TYPE_STATUS_UNSUPPORTED` to fall
through to a lower provider or ordinary RAM.  This is required for an active
profile overlay to defer to the existing immutable-ROM provider while disabled.
The generic static capacity is increased from five to eight slots to accommodate
the existing composed EGA, ROM, and new D4 device graph without a hidden limit.

## Proof

- `core-machine-memory-device-registration-s16-smoke` proves generic
  configuration-only registration, an inactive overlay falling through to a
  lower immutable ROM, active overlay read/write ownership, and freeze
  rejection.
- `vm-model40-d4-map-s16-smoke` uses only project-owned synthetic two-chip ROM
  bytes.  It proves the immutable low ROM while replacement is disabled,
  writable high compatibility RAM, selected diagnostic read, low-active
  replacement, low-active write protection, post-reset disabled controls, and
  retained S14 reset-alias behavior.
- Retained `vm-model40-rom-layout-s14-smoke` continues to prove interleave,
  mirror, immutable write rejection and reset fetch.
- Focused CTest: all three controls pass.
- Serial `ctest -L current-gate --parallel 1` passes **271/271** tests.
- The runnable developer artifact is `vm-0-5-0387`,
  `build/output/nxvm_0_5_0387.exe`, SHA-256
  `1F294C9D0A04D5F3E32F4C06DDD6E6CC6DD543F14E5807C5BB9C9075F446213E`.
- Applicable composition, ownership, artifact and documentation gates are
  recorded before P1 acceptance.
## Boundaries And Transfer

This is selected-machine functional progress, not a vendor-ROM loader or BIOS
execution result.  The compatibility RAM begins zeroed only as project-owned
volatile backing; the primary source assigns any ROM copy to firmware, which
remains outside S16.  It does not model ROM shadowing/relocation, D4 RAM parity
sources or diagnostic latches, IOCHK side effects, board decode timing, memory
wait classes, physical bus behavior, or Model-40 L3.  Those transfer through
the T386 functional and board-timing receivers.

## Coordinator Acceptance

P1 7c2f34f delivered the complete implementation, focused proof, artifact and
evidence.  Coordinator review rejected its acceptance only because PowerShell
had encoded Markdown backticks as control characters in the delivery evidence
and the focused smoke retained a temporary failure print.  P2 9199ea07
removes those delivery defects without changing the accepted mapping contract;
its focused Core, Model-40 and immutable-ROM regressions pass, and the
coordinator rechecked the pushed diff, control-character sweep and branch
state.

The coordinator reviewed the actual S16 source surface: Core contains only a
generic configuration-time overlay registration and ordered checked dispatch;
ordinary immutable ROM registration remains exclusive.  VM contains all Compaq
addresses, D4 backing, diagnostic/control semantics, write protection and reset
state.  The final artifact and 271/271 serial gate are from the reviewed source
graph; composition, ownership, artifact and documentation gates pass.

S16 is accepted as bounded Model-40 functional progress.  It does not claim
firmware execution, full D4 diagnostics/parity, device completeness, board
timing or DeskPro L3 closure.  T386 remains open for the next capability-ledger
receiver.
