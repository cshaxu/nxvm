# M5 T320: VM86 CPL0 Delivery

## S1: VM86 To CPL0 Delivery

T320 S1 implements the bounded 80386 VM86-to-CPL0 entry foundation: protected
32-bit IDT interrupt-gate delivery for `#GP`, `#UD`, `#NM`, and external IRQ0.
It uses the existing TSS32 `SS0:ESP0` selector and existing IDT/PIC routes.

The local outer-gate planner now preserves the VM86 source state, preflights
the full VM86 frame and every stack write, clears VM before publishing CPL0
caches, and places
zero-extended VM86 segment selectors in the four additional frame dwords.
Protected `#UD` is routed to vector 6, while only Intel error-code exceptions
request an error-code frame. No inverse `IRET` transition is changed.

The failure matrix covers invalid/missing/non-present/wrong-type gates,
invalid/non-busy/short TSS states, invalid SS0 forms, and an insufficient new
stack, always by full pre-publication CPU-state comparison.

The direct historical VM86 consumers were reviewed exhaustively: S47/S48
inherit the migrated `PUSHF`/`POPF` and `CLI`/`STI` valid-GP assertion; S49
contains the corresponding valid-GP `HLT` assertion. S50 inherits the CLI/STI
proof, while its native VM86 cases deliberately have no admitted valid
IDT/TSS facility. The native S55 I/O, S62 CLTS, and S63 SMSW/LMSW VM86 branches
also lack that premise and retain their terminal diagnostic matrices. No
instruction behavior or non-VM86 assertion changed.

Detailed form, frame, failure-boundary, and caller-sweep evidence is in
[T320 S1 matrix](../etc/evidence/t320-s1-vm86-delivery-matrix.md).

## S2: CPL0 IRET Return To VM86

T320 S2 adds the bounded Intel 80386 inverse: a CPL0 32-bit `IRET` atomically
returns from the full nine-dword VM86 frame. It preflights and reads every
field before publishing VM, segment caches, EIP, ESP, or CPL, then restores
real-mode-style caches for CS, SS, ES, DS, FS, and GS.

The S2 owner smoke proves direct `IRET` and `67 IRET`, exact FLAGS/cache
properties, low-word selector extraction, continued VM86 execution, and the
short-stack no-publication boundary. The S1 IRQ0 fixture additionally installs
an `IRET` handler and proves the real VM86 delivery-to-return round trip.
`66 IRET` is a legal 16-bit protected-mode return but cannot carry VM in its
popped FLAGS word, so it is not a VM86-return form. Detailed S2 evidence is in
[the S2 matrix](../etc/evidence/t320-s2-vm86-iret-matrix.md).

## Verification And Artifact

- Fresh `mingw-gcc-x64` configure and the exact single current registration
  passed; the focused owner marker was `M5:T320:S1:VM86-DELIVERY:OK`.
- The current developer artifact is
  `build/output/nxvm_0_5_0320.exe`, built from this P1 source tree.
- SHA-256: `4E66566B83900E6AABC9ECA54732E1E5B266809846AFD740D7019BE9669A378A`.
- Runtime identity is `Neko's x86 Virtual Machine`; the CMake target embeds
  build version `0.5.0320`. The ordinary console executable has no
  non-interactive version switch, so its identity is recorded from its emitted
  banner definition rather than leaving a product process running.

## Retained Boundaries

VME/PVI, VM86-origin IRET, task gates/switching, 16-bit VM86 gates, paging,
NMI/PIC redesign, VM86 LGDT/LIDT, and x87/provider work remain outside T320.

## Coordinator Acceptance

The coordinator independently accepted S1 implementation P1 (`afa35a6b`),
its direct-consumer and diagnostic P2 (`bf1628f5`), and packet P3
(`e69fae9e`). S2 P1 (`be84b715`) established the local path but was not
accepted until the consolidated ordinary-mode P2 (`e0ead63b`) supplied the
round-trip, full-cache, selector-word, and dead-path evidence. Fresh
configuration, documentation governance, `git diff --check`, and the complete
198-test current gate were re-run. T320 is closed as the bounded VM86-to-CPL0
delivery foundation.
