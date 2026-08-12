# M5 T320: VM86 CPL0 Delivery

## Implementation P1

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

## Verification And Artifact

- Fresh `mingw-gcc-x64` configure and the exact single current registration
  passed; the focused owner marker was `M5:T320:S1:VM86-DELIVERY:OK`.
- The current developer artifact is
  `build/output/nxvm_0_5_0320.exe`, built from this P1 source tree.
- SHA-256: `4A25D30E0C588F887048B265C761DD57C8E20D7C8C456C3A39B0B6544458DE09`.
- Runtime identity is `Neko's x86 Virtual Machine`; the CMake target embeds
  build version `0.5.0320`. The ordinary console executable has no
  non-interactive version switch, so its identity is recorded from its emitted
  banner definition rather than leaving a product process running.

## Retained Boundaries

CPL0 `IRET` return to VM86 is T320 S2. VME/PVI, task gates/switching, 16-bit
VM86 gates, paging, NMI/PIC redesign, VM86 LGDT/LIDT, and x87/provider work
remain outside T320 S1.
