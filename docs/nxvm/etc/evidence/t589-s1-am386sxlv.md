# T589 S1: Am386SXLV implementation contract

HP service manual 5965-0244 gives 20 MHz and the SXLV family for OB300.
The [AMD datasheet 21020, Rev A](https://www.amd.com/content/dam/amd/en/documents/archived-tech-docs/datasheets/21020.pdf)
specifies the external bus, management cycles and SX/DX differences.

| Boundary | Baseline and implementation contract | Direct verification |
| --- | --- | --- |
| Architectural execution | Reuse the existing 80386 decode, execution, privilege and paging. Keep logical addresses and register operands 32 bits where the 386 permits them. | Existing 386 instruction and paging tests, then one differential case for each new selected behavior. |
| Reset and external address | Core currently uses a hidden reset CS base of FFFF0000 for 80386. Verify the SXLV's external FFFFF0 fetch separately from the internal execution address, and truncate normal external addresses to 24 bits at the single CPU transaction boundary. The mutable board A20 mask has a different owner and cannot substitute for a fixed CPU address width. | Reset fetch, far transfer out of reset, A20 both states, address truncation after paging and accesses crossing the external address limit. |
| Data bus and prefetch | AMD specifies a 16-bit bus and two-byte instruction prefetch units. Keep the architectural byte/word/dword operation intact while accounting for actual byte lanes and aligned bus transfers. Audit existing cycle and data callback owners together before changing either. | Aligned and odd starts for word/dword memory and I/O, two-byte prefetch and page splits, with old-profile regression. |
| System management | Current cpu.c SMI admission and cpu_instructions.c SMM entry require 80486SX. AMD specifies a separate memory transaction space, state locations at 60000h–600CBh and 60100h–60127h, SMM reset fetch and RES3 restoration. I/O retains the ordinary port space. Use Core's single interruption boundary with an SXLV-specific save/restore format. | Exercise SMI, separate memory, NMI priority, restoration, UMOV and I/O instruction break against the documented contract. Do not use an ordinary 486 save image as evidence. |

Deliver address/reset and transaction width first. Admit SMM as a separate
batch when the transaction contract is established. Chip selection and
observable display/input tests are subsequent hardware batches. The ROM's
65510 identifier is firmware evidence. Confirm the fitted display part and
the 65536 lead before freezing the device profile.

The existing cached Release targets had no build work. Both host widths
passed 5/5: real-mode 386 address, immutable ROM mapping, CPU profile gate,
80386 paging and external time trace. Log SHA-256:
`74d0606edef5caaeb16c7867059e6c9962213fc27658e3545d286aad6a530a63`.
These are current baseline regressions. No AMD profile, desktop, board
or real firmware test is covered by this S. No production code or EXE changed.
