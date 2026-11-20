# T590 S2 OmniBook 300 board construction

The board plan uses the Am386SXLV profile, 2 MiB base RAM, 20 MHz nominal
guest time axis and existing cascaded PIC, 8254 PIT, dual DMA, 8042 and
RTC owners. The SCAMP-compatible oscillator-to-PIT model uses a nominal
1,193,182 Hz. IRQ0, IRQ1/12, DMA cascade and RTC IRQ8 are supplied by the
shared board services. The initial A20 qualification covers the existing
port-92 and KBC wiring. Port EEh and its electrical combination with the
other A20 sources still need proof against the original board path.

## App and shared responsibilities

`ob300_chipset.c` owns the App's FBh/F9h configuration gate, readable
ECh index latch, and EDh dispatch. Implemented indexed registers are
the modeled SCAMP II identity, MISCSET and SMMCTL. SMMCTL reads its
active bit from Core's board signal. Its write path masks read-only
bits and honors the management lock. EFh requests the existing CPU-only
reset when the VLSI special functions are enabled. Unknown indices
on the enabled route return UNSUPPORTED, leaving missing board behavior
visible during original firmware qualification.

Core already owns SMIADS cycles and an optional memory-provider pair.
The X86-NXVM Board Plan previously forwarded only the management-active
signal. The board profile now forwards the existing read and write pairs,
requires both or neither, and leaves the old unsupported result for
profiles without a pair. Lib, Emulator and X86 Core need no new API.
The App owns management window and reset image contents. The test uses
the optional 32 KiB firmware input with the normal Core immutable image
and reset alias. Its reserved management backing is distinct from
ordinary RAM.

## Scoped verification and limits

The MyOB300 board smoke passed on x64 and x86. It exercises configuration
lock/unlock and read-only bits, device port round trips, A20, and a real
guest instruction sequence entering SMM, reading and writing a management
byte, and returning with RES3. It checks that the same ordinary RAM
address remains unchanged. A synthetic reset image and preconfigured
SMMCTL provide isolated test inputs.

Current management storage implements the BASIZ window lengths, with a
private backing for the selected range. BASIZ target placement, shared
shadow visibility, and SMIADS traffic outside the qualified windows are
open. The [original reset trace](t590-s1-ob300-reset-decode.md) also found
an indexed initialization loop whose 23 indices include values absent
from the currently identified SCAMP II map. The next firmware probe must
classify that batch, rather than retaining arbitrary index writes as
successful chip behavior. Display, physical input routing, System Card
windows and original-ROM POST remain later board and machine stages.

## Original reset replay

A read-only probe passed the same 32 KiB original BIOS span from S1 to
the App board plan and stepped the guest one instruction at a time.
Both host widths left the reset path, reached the PIC initialization
routine at FA65Fh, and stopped at FDAF0h on step 84. The board declined
the output and Core reported `LIB_STATUS_INTERNAL_ERROR` (status 6).
The stopped instruction writes EDh, following
the unlocked selection of index 2Ch. The next routine instructions
select 2Fh and start a search at F000:7FF0 for an eight-byte marker
beginning with BIOS0. All eight bytes match the bytes at card offset
7F7FF0h, immediately below the supplied reset-ROM span. This supports
the candidate upper 64 KiB map from card offset 7F0000h into the F000
segment. It does not establish which other pages either index controls
or how their values select them. Table 64 of the VL82C316 manual leaves
2Ch and 2Fh unassigned. Carry this pair and the identified marker to
the System Card routing batch, where the decoded bytes can be tested
together, rather than guessing register effects.

The 23-entry loop at F8E05h was not reached. Its static table is
still relevant to chipset qualification, but this replay establishes
no cold-path ordering for that loop. The prefix and stop were identical
on x64 and x86. Probe output SHA-256:
`5968d28cca1db422db7cc36887ee5382955dfeb46f4e4bb5abadcda6a2cc583e`.

The specialized gate target passed on both host widths. The x86 unit
aggregate passed 717/717 in 256.33 seconds. The x64 aggregate reached
its 300-second total deadline with 329 passing results and one per-test
Lib boundary timeout. That test passed alone in 11.82 seconds. Four
more batches passed the remaining 387 registrations, for 717/717
verified on x64 without changing the test budgets or the built sources.
The original-ROM replay above is the only external-media run in S2.

| Verification record | SHA-256 |
| --- | --- |
| x86 unit aggregate | `aa2e852b024e48e01e419f7117f5fc5e42b8f00ba211f2e3e91676bbd39db66c` |
| x64 unit aggregate | `691939bc9a348b92413661f5443227ebdb85d06e199e2eeb6092963dba1e8843` |
| Unit inventory reconciliation | `a67da0ce40996e61ca8399f9c2ee4a12ad08e5557f3d44d71e427ec0caf1d3f3` |
| X86-NXVM production manifest | `59860acbd307a81263ce98ad27fedb8cbcf2c472ec34b8382269fcdabef44b5e` |

## Display and input handoff

X86-NXVM Machine construction already accepts an injected display
snapshot provider with the generic board display disabled. MyPC110
demonstrates this path with the 65535 variant. S3 can retain the disabled
generic topology, give the App profile one C&T 65510 instance and use
Core port and memory-device routes. The App provider captures from
that same instance and can be injected by the later machine receiver.
OmniBook's aperture and port decode must follow its firmware evidence,
including indexed word writes and declined accesses when the legacy
aperture is disabled.

The 8042 topology already connects guest ports 60h and 64h, IRQ1/12 and
auxiliary reports. The next board test should drive the controller
through those ports and confirm a guest can observe the resulting bytes
and IRQs. Use an App-owned command extension only if the OmniBook
firmware accesses a command the existing controller cannot service.

## Published receivers

The cached release publisher deployed both widths for all seven existing
products. A separate check matched each published hash to its deployed
file, checked the x86 or x64 PE machine value and found exactly two EXEs
per receiving product. Existing version identities were retained. The
App board smoke is an internal test and is outside this release set.

Publication log SHA-256: `8392e505fbe56689cb024008f2b97716e18b186e9a722945e97fc1e345082151`.

| Product | Host | File | SHA-256 |
| --- | --- | --- | --- |
| nxvm | x86 | nxvm_0_5_0582_x86.exe | `4ac3c204e592fac6b20b7ac0385298a551cd2b222dc64a507d7277a36fa2d9a6` |
| my5160 | x86 | my5160_0_5_0582_x86.exe | `86a5ee5a23ad504cd09babfa6d9b5f8357672304689f3ba163712606ee8d2bae` |
| my5170 | x86 | my5170_0_5_0582_x86.exe | `ce84c95965dc80b580c904cc68c00059f9eef33430e1c75dcb59189a47a6fd22` |
| mydeskpro386 | x86 | mydeskpro386_0_5_0582_x86.exe | `a36f37fd8b629e049259aab4795560c4a553cf44da84845c349ef128cdcb0aa3` |
| mypc110 | x86 | mypc110_x86.exe | `8433afbf6b0142886c79141d88927d8b9cbc253d8be2cbfb542d76b0af30ee78` |
| mynes | x86 | mynes_0_0_0044_x86.exe | `0cd9291713a56d65d1ab7e24f36bf3b7a8aa0be4b7ae4d8c785e9da9d8f08fcb` |
| myraidenii | x86 | myraidenii_0_0_0007_x86.exe | `3006b32c0a5452f9b8ef3e75d2840c7890b6b52cafccdde4034429f95ae86f81` |
| nxvm | x64 | nxvm_0_5_0582_x64.exe | `1ebb505166e495a4d57e16d7bba2df0972b31edecc42590836c74d50050e8469` |
| my5160 | x64 | my5160_0_5_0582_x64.exe | `9625cf82b20e6c830478e739becfa5a0f7b9c8c1da4ab61e7c1ac211a6b27a0d` |
| my5170 | x64 | my5170_0_5_0582_x64.exe | `993805716a50373ced9df6da055c2a3ee1ddd1aee875075f010dc81a6cc54f18` |
| mydeskpro386 | x64 | mydeskpro386_0_5_0582_x64.exe | `f1637eea0e020393ad9f69dc383124fa699e473eb405e7f40d5bb46a9a48cd2d` |
| mypc110 | x64 | mypc110_x64.exe | `37a0f2a4c7b6784bd4e9e305305bb36136b54ae9cf62a9f90ea50314bd2c0cee` |
| mynes | x64 | mynes_0_0_0044_x64.exe | `d7b1070ee751e84b3194c68b9bbda6652be8fe76d7db88a5f58ca22aa84ee772` |
| myraidenii | x64 | myraidenii_0_0_0007_x64.exe | `22c7d5292f999e77638f2243337e6f578fcfb7cb84e11e7a4694372094d9a053` |
