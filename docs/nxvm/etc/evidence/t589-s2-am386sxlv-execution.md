# T589 S2: Am386SXLV ordinary execution

Baseline: `61c3daca1`. The [AMD Am386SX/SXL/SXLV datasheet](https://www.amd.com/content/dam/amd/en/documents/archived-tech-docs/datasheets/21020.pdf),
21020 Rev A, distinguishes the SX 24-bit physical bus from the 386's
unchanged logical address space. The S1 contract records the separate
16-bit transaction and system-management batches.

The appended CPU profile selects the existing 386 instruction and paging
paths. The CPU owner applies the 24-bit limit at completed memory bus accesses
and splits a span at external address rollover. A20 remains a board memory
signal downstream of that limit. The reset-specific route uses the physical
FFFFF0h address, with architectural CS:EIP and the Core's existing reset
provenance retained. Tool-directed linear memory and descriptor timing
inspection use the physical result. Raw Machine physical inspection and DMA
retain their own address contracts.

Instruction cost uses the inherited 386 model as a compatibility estimate.
The new profile rejects verified physical timing publication. Two-byte
prefetch, 16-bit electrical transfer timing and AMD SMM are later hardware
batches. AMD 21020 Rev A, printed page 28, specifies DH=23h at reset for the
SX family. The diagnostic uses DL=0 as the modeled revision. The actual
OB300 stepping is unverified. HP board selection and original-ROM execution
are downstream. No product currently selects this profile.

Existing test owners exercise the new profile with a synthetic reset jump
through the low 24-bit reset alias, an ordinary 32-bit store in both A20
states, and a linear read/write crossing the high ROM to low RAM rollover.
The existing protected-mode paging path is reused with high bits in CR3 and
page entries. It checks retained CR3 bits, page-walk effects, guest write,
and reset control state. Both cached Release widths passed the focused batch.

The coordinator caught an inherited DX reset signature before closure.
The corrected candidate asserts DX=2300h at reset and reruns the gates.
The previous publication and any partial aggregate are superseded. The
Ninja trees retain completed object files.

The first complete-unit attempt on x64 reached the 300-second deadline.
Its partial output also identified two real fixture failures. The V30
negative cases and neutral Machine configuration test treated enum value 8
as invalid before that value was assigned to Am386SXLV. Those probes now use
the existing out-of-range convention 0xff. The product source and verified
EXEs were unchanged by that test repair. The retry keeps the same deadline
and runs eight test jobs on this qualification host.
The complete retry passed 716/716 units on each width, in 186.59 seconds
on x64 and 294.60 seconds on x86. The reserved 300-second budget was kept.

## S3 handoff

The AMD source distinguishes the physical two-byte fetch unit from the
processor's instruction stream and states that BHE/BLE select lanes on the
16-bit bus. The existing Core refill still requests a byte range. Route that
range through the existing bus cycles, preserving page and segment
bounds, fault ordering and decoder behavior. Queue depth is a distinct
hardware question. Do not shrink the decode window to two bytes.

For memory and I/O, qualify the emitted address/width sequence for aligned
and odd word/dword transactions, including the odd dword's byte/word/byte
decomposition. Preserve full operand validation before port side effects,
the one existing transfer/completion owner, and fault cancellation after
a partial transfer. Account for completed bus cycles exactly once. The
existing address-span, prefetch and external-cycle fixtures are the first
regression candidates. AMD pin behavior alone does not give an exact
OB300 board wait-state count.

## Refreshed publication

The corrected candidate published all fourteen receiving EXEs. Existing
product versions were retained. Every deployed file matches the publisher
SHA-256 and its expected PE machine. Both full unit aggregates passed.
The new CPU profile is not currently selected by a runnable product, and no
external-media boot qualification was performed in S2.

Source manifest SHA-256: 15d57cffe37b8a00cbedc0847c249f4acc676f96daeb42e253580a472dd11cc4

| App | Host | File | SHA-256 |
| --- | --- | --- | --- |
| my5160 | x64 | my5160_0_5_0582_x64.exe | 4f325b2081b57b466dd3d8de86fd89a7b13a94893ef11656126b3e63be160744 |
| my5160 | x86 | my5160_0_5_0582_x86.exe | 91a4699b7ea9e095f0c9b74ebecd9b4a51d8d459fb7fc03a6a4aeabb7f812b07 |
| my5170 | x64 | my5170_0_5_0582_x64.exe | 1ee347b4555bcdcca12410c4dca6023225906384166bd06827533ceff5d9ee55 |
| my5170 | x86 | my5170_0_5_0582_x86.exe | d57efe215d1a15266e875c4c508d8ea7667ebd3b269e5e747a1978feef8f04fe |
| mydeskpro386 | x64 | mydeskpro386_0_5_0582_x64.exe | 2099c799119bb3ca4414e676e856a7d6561a31570789475882df300843cceb91 |
| mydeskpro386 | x86 | mydeskpro386_0_5_0582_x86.exe | 2ebd829ec354fb8d2e938145b55ef55e335e7e1da820badaaa0fec036af05d91 |
| mynes | x64 | mynes_0_0_0044_x64.exe | f8da1cc828d14b7eec892c132e21a765c21d65f09ca9230411a68da575f313d1 |
| mynes | x86 | mynes_0_0_0044_x86.exe | 5af91ff77288c9f6ea0ba4d37cd25e0ec51b0a869255788c697437ca6cd5bd54 |
| mypc110 | x64 | mypc110_x64.exe | 2d2a51b9107d34ae6e95fff36818ae1e6f9d9e33956e975ae49714b7d981a8d3 |
| mypc110 | x86 | mypc110_x86.exe | 1729b9da6b629a7e1ee90423e74840e9f9c77ac230d95e0e4ceee2b3ffd6f9c4 |
| myraidenii | x64 | myraidenii_0_0_0007_x64.exe | 8f4df63a897a14b877e72b133a8e0eb5bdb74e127dc06b918dbf220e6df17cdb |
| myraidenii | x86 | myraidenii_0_0_0007_x86.exe | 068839a7f1b379979a2ee3a2283ba1af062d1768e4d49cb54ad609d2cfedd337 |
| nxvm | x64 | nxvm_0_5_0582_x64.exe | d75d97e1876d90acd856bfb3305f1f5fea7d653cf4821063fc90b75cbf02a69e |
| nxvm | x86 | nxvm_0_5_0582_x86.exe | b05a87e3ea78a2143369b53f9c1b44e88af413ee458efb67aba4af6691eb46f9 |
