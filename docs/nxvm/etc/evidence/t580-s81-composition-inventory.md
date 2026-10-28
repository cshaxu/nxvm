# T580 S81 — PC110 composition/test ownership inventory

`pc110_machine.c` and its header build a second PC110 board solely for
`base_composition_smoke.c` and `display_route_smoke.c`; the real product uses
`mypc110_machine_construction_create` and the X86-NXVM Machine driver.
S81 removes the duplicate only after preserving the following checks:

| Existing check | Owner after consolidation |
| --- | --- |
| PC110 topology, clock, RTC, fixed memory and media binding | Actual production construction test and PC110 plan facts |
| RAM limits, absent/ROM windows, scratch ports, font bank and reset | Actual production composition through existing Driver Debug requests |
| 8042 command deadlines, keyboard/AUX protocol and interrupt | Existing X86-NXVM controller and board tests; real firmware input evidence in S78/S79 |
| F65535 mode setup and captured text/graphics | Actual production composition's existing display provider |
| F65535 CPU-read latch versus non-mutating inspection | Existing F65535 chip contract test |

The real Driver test now checks the 20 MiB boundary, unmapped ranges, the
four-byte high-memory pattern, legacy port access, scratch-port reset, both
font banks and the text and planar display routes. These checks exercise the
same construction and display provider as the product. The controller's
timed reset/ACK and AUX sequences remain covered by the existing chip and
board contracts; the S78/S79 guest tests independently exercise firmware and
pointing input. Those tests do not claim to repeat every old observation at
the identical instruction boundary. Latch-preserving inspection remains in
the F65535 chip contract. No new Core getter or App production access path
was added.

Cached x86 and x64 builds completed. For each architecture, all 11 PC110
unit tests and six artifact/CMOS integration tests passed, as did the four
controller, AUX and display owner tests. The released artifacts were relinked
and the firmware-identity tests checked both variants on both widths:

| Artifact | SHA-256 |
| --- | --- |
| mypc110_x86.exe | a3aaf4b8d397033dbd28699f4eae86313092c4ea53a715d322f07cd7560c9e97 |
| mypc110_x64.exe | 997496ddb91987bc87bb297edafcba5ef73cdd764be021ea72932cad5983b77a |
| mypc110_tft_x86.exe | 6a2f28b0bd37b2d6ef3d8a8fb1a92631ab99791e48733104be272197d8b7308d |
| mypc110_tft_x64.exe | c09fc0137e1aa9f6d1fa42ddf4e4db2a47d4c22991dde01b787e5e329772ba26 |

The full registered unit suite passed on both architectures: 668 NXVM,
Shared and MyRaidenII tests plus 45 MyNES tests per architecture. MyNES was
enabled afterwards in the same cached build trees and its tests were built
without rebuilding its product EXEs. Full integration and desktop/RDP
qualification have not yet been completed. The existing real-guest diagnostic
still references Machine private internals; that separate boundary remains for
subsequent work.
