# M5 T589: OmniBook 300 basic hardware

Admitted 2026-11-19 following [T588](m5-ob300-design.md). Follow the
[OB300 architecture](m5-ob300-architecture.md) and PC110's component ownership.
Qualify individual device contracts with synthetic guest work before board
wiring, original firmware or product integration.

| S | Scope and exit evidence |
| --- | --- |
| 1 | Establish the Am386SXLV implementation contract, existing coverage and missing CPU behavior. Verify the current x86/x64 baseline. |
| 2 | Implement the ordinary Am386SXLV CPU selection on the existing executor. Verify inherited instructions, reset and 24-bit external address decoding. |
| 3 | Implement and verify 16-bit external bus transactions and two-byte prefetch. |
| 4 | Implement and verify Am386SXLV management entry, memory space and state restoration, including the documented instruction and I/O behavior. |
| 5 | Identify the display silicon from physical/documentary evidence, reconcile the ROM's 65510 identifier with the owner's 65536 lead, and deliver the required register, VRAM and frame behavior. |
| 6 | Implement or reuse the OB keyboard and pointing device protocols. Verify make/break, mouse movement/buttons, controller handshakes and reset. |
| 7 | Qualify basic PIC/PIT/DMA/RTC and the CPU/display/input contracts. Record precise retained limits and board receiver requirements. |

T closes when required device behaviors are implemented and independently
verified on both host widths, reusable device code has one owner, and every
board signal needed by the next T has a declared receiving contract. Physical
accuracy claims require source evidence. Keep a bounded L1/L2 model where
exact timing is unavailable. System Card mapping, HP port decoding and full
system boot belong to later stages. Shared interface and delivery obligations
apply to actual shared changes.
