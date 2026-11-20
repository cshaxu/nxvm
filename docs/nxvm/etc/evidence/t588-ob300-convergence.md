# T588 S1: OB300 identity and finite convergence ledger

Baseline: `cd6477a5c`, 2026-11-19. S1 is a read-only source/asset inventory;
it does not qualify an executable or prove a physical memory decoder.

## Verified input and source boundaries

The owner-supplied `rom_system10_aba_binary_16mb.zip` passes ZIP CRC and
contains one 16,777,216-byte `16_rom_10_aba_common.bin`. Its SHA-256 is
`b98001a30b79f126483f823fd4d7a5ce91a973a5ab8fb35e3d76043b74a8e099`.
The companion `rom_system10_aba_files.zip` also passes CRC and contains
125 files. Both archives were inspected in memory, without extraction or
modification. External source paths and protected bytes are not published.

The owner's wider collection also supplies Operating/Quick Start/Personal
Information guides, Word/Excel guides and two copies of the family service
manual under `manuals/ob300-530`. These local primary references take priority
over assuming documentation is unavailable; their detailed requirements are
inputs to S2/S3, not yet individually verified by this inventory.

`drivecs/ob300` contains `ob300a_system100_aba.zip`,
`ob300a_system101_aba.zip` and `ob300b_system101_abb.zip`. Directory inspection
of the 1.0 backup finds 43 files totaling 977,844 uncompressed bytes, including
`WINDOWS/SYSTEM.INI`, `OBWIN.COM`, swap utilities and supplementary DLLs.
It is a file-level backup, not a sector image proving partition/boot geometry.
Use this version-matched backup to cross-check ROM/C-drive composition and
later construct disposable media; preserve its original archive unchanged.
Driver folders for other models are not evidence of OB300 compatibility.

At dump offset `0x7ffff0`, the candidate far jump targets `F000:E05B`;
`03/27/93` appears at `0x7ffff5`, and `65510` at `0x7e0091`.
These are reproducible content observations, not proof of where a card bank
appears on the guest bus. Separate attribute/CIS memory has not been verified.
The earlier FFS20 capacity observation likewise does not establish decoding.

OBSETUP and the ROM-resident modules together supply the Windows environment.
The extracted file list is not a replacement for the card image. Do not
implement a host-side DOS/Windows loader to bypass guest firmware execution.

The [HP service manual 5965-0244](https://www.elhvb.com/mobokive/edwin/laptops/hp/omnibook/service/ob300-425-430-530_sm.pdf),
printed page 4 (PDF page 9), identifies OB300 as 20 MHz 386SXLV, separately
from the 425/430 486SLC/e. It lists 640 by 480, 16-gray LCD output.
This identifies the product, not every silicon quirk or timing constant.
Only exact sourced constants/formulas qualify as L3; reference models are
L2 and unsupported explicit assumptions L1.

## Current implementation and receiving work

All rows are inventoried, not hardware-qualified. S2 investigates execution
and mapping; S3 freezes each row's implementation/model boundary. Later
receivers are the existing ROM DOS, performance, Windows and product proposals.

| ID | Domain and current evidence | Owner and remaining proof | Receiver |
| --- | --- | --- | --- |
| O01 | X86 Core has 80386 and 80486SX profiles, no explicit 386SXLV | CPU semantics remain X86 Core; establish bus/address/reset and firmware-visible differences before selecting or extending a profile | S2/S3, ROM DOS |
| O02 | Immutable ROM and reset-window alias APIs already exist | App chooses actual card windows; Core owns routing. Execute candidate reset bytes under labeled mapping hypotheses and record first unsupported access | S2, ROM DOS |
| O03 | Common dump and extracted FFS20 files available; attribute memory unverified | App owns card decode, banking and CIS behavior. Distinguish observed firmware requirements from assumptions; preserve whole external image | S2/S3, ROM DOS |
| O04 | Core provides memory providers; masked-span/provider-decline debt remains in TODO | Core owns generic routing; App owns HP RAM/window registers. Determine whether boot needs the unresolved crossing case before proposing Shared changes | S2/S3, ROM DOS |
| O05 | Existing PIC/PIT/RTC/DMA/KBC/UART mechanisms available in X86-NXVM | App owns HP wiring and ports; reuse only verified compatible chip behavior. PC110 composition is not OB300 hardware evidence | S3, ROM DOS |
| O06 | Dump identifies 65510; no dedicated implementation found; existing 65535 is different | Establish C&T register/mode subset and panel connection. Reusable chip behavior belongs in shared chips only with an approved boundary; HP integration stays App | S3, ROM DOS |
| O07 | Standard keyboard/pointer mechanisms exist, HP details unresolved | App owns HP translation/control; trace actual firmware accesses before selecting KBC/mouse wiring | S2/S3, ROM DOS |
| O08 | HP power/control registers not mapped to an existing proven model | App owns HP power and board glue; explicit L1/L2 subset must retain one guest clock and normal reset/lifecycle path | S3, ROM DOS |
| O09 | ROM shell/setup expects writable C and ROM-backed software paths | App selects real storage/controller model; no host filesystem shortcut. Separate disposable test disk from owner media | S3, ROM DOS |
| O10 | OBSETUP plus ROM Windows/Word/Excel are the software target | Guest executes original modules; Shared UI/Session remain generic. Verify installation/persistence and actual application interaction | Windows |
| O11 | Historical CPU versus Machine and PC110 timings show risk, not OB300 speed | Measure fixed retired work and real boot checkpoints on both widths. S3 freezes DOS boot/input-response budgets from S2 evidence; no numeric speed claim yet | S2/S3, performance |
| O12 | Existing Emulator lifecycle, frame and publication mechanisms reusable | App binding/config and artifact identity require design; do not change Lib/Emulator speculatively or redistribute Microsoft ROM payloads | S3, product qualification |

Existing regression owners include
`test/x86-core/machine/machine_immutable_rom_mapping_smoke.c` and
`test/x86-nxvm/board-base/rom_mapping_smoke.c`; they cover generic routing,
not OB300 card decoding. Extend established coverage where appropriate;
new App tests should prove HP wiring rather than duplicate CPU instruction tests.

The [masked-memory debt](../../states/TODO.md),
[CPU/Machine comparison](t582-s27-performance-envelope.md) and
[PC110 stage closure](t582-s40-stage-closure.md) remain explicit inputs.
Different fixture contracts prevent attributing their timing difference to
one function or treating those historical numbers as current OB300 results.

## Design and closure predicates

S1 establishes identity, ownership and a finite checklist. S2 must produce a
bounded actual-executor reset trace, identify the first unsupported route and
measure repeatable work; mapping assumptions must remain visible. S3 must
resolve each row into an evidenced implementation or explicit model and
receiving acceptance test, with numeric performance budgets and public-contract
review needs. Missing manuals alone do not justify indefinite blocking.

T588 cannot close merely because this inventory exists. It also cannot claim
DOS, Windows or application usability before their receiving tasks qualify them.
This S changes documentation only; no source, test, ROM, INI or EXE changes,
no dynamic boot, desktop or RDP validation, and no new build is warranted.

S1 review: proposal/queue/current and all changed-document local links were
checked; NXVM documentation governance and `git diff --check` pass. Changed
text is LF. The review keeps observed ROM offsets separate from address-map
claims, and file backups separate from sector images. No temporary files or
external modifications were created. S1 is delivered for review; S2 is next.

## S3 final dispositions

All O01-O12 inventory rows now map to the explicit owner, model limit and
receiving proof in [the final design](../../proposals/m5-ob300-architecture.md).
O01/O02/O11 have the additional bounded [S2 execution evidence](t588-s2-reset-proof.md);
other hardware behaviors remain implementation receivers, not accepted devices.
No row is dropped. The design gate is complete; the four downstream product
qualification gates remain unexecuted.
