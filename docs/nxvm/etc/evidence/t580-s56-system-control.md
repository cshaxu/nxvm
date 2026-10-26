# T580 S56 - System-control A20 and processor reset

Baseline: clean pushed `b2d7942b7`, accepted S55 (`6964e8280`). The unchanged
TFT BIOS requests reset through 92h after real Setup input, then executes
CLI/HLT because no App route handles it. S55 records both host widths at
the same ABC2 boundary; this is not a guessed CPU, RTC or timer failure.

## Before-change mechanism and sources

Board-Base already installs a 92h route for the PC/AT facade, but its writer
only sets Core's final A20 mask; its readback exposes the effective CPU line,
not the independent fast gate. The KBC callback separately overwrites that
same final mask. Thus merely enabling the current port would still lose the
reset request and retain last-writer-wins A20. The mechanism must be repaired
once at Board, rather than adding a second PC110-specific handler.

The original BIOS at ABAD--ABC1 reads 92h, writes bit 1 set with bit 0 clear,
then writes bit 0 set and halts pending reset. Open-Source-PC110's pinned
live-chipset report records 92h=02h and identifies fast-A20/reset, but its
related SCAMP databook is not an exact VL82C420 specification. Treat the
functional implementation as L2, not a sourced physical pulse duration.

86Box source pin `4fef696a4eead1d55a28d6ac0e5bd2864e5454da` supplies an
independent behavior reference: `src/port_92.c` has separate fast-gate
readback and bit-zero rising-edge reset/rearm; `src/mem/mem.c` combines KBC
and fast gates with OR. File SHA-256 values:
`15CD39C68A7C102AB6E0657A5DF4BDDF760BD06735F08DB48C8A587419806B3C`
and `4FCA48F7DC046844A6C852A5723590BEFD213479322563F0FFCBF8CC376F054A`.
No third-party code is copied or imported. Our existing Core reset request
owns retirement-boundary consumption; no separate pulse timer is introduced.

## Contract and planned change

Append one `lib_bool system_control_port_present` Board config fact. Existing
PC/AT facade decoding remains, while PC110 explicitly selects the same route
without acquiring the facade's unrelated ROM aliases. Both selection forms
resolve to one installer and handler; there is no model-name dispatch.
Reject non-boolean values. No new public operation, enum or callback.

The private Board holds the two real electrical sources: the system-control
bits and KBC A20 output. One helper publishes their OR to Core. Port readback
returns only its own bits; reserved bits are ignored. A bit-zero rising edge
uses the existing processor-only reset request. Repeated one does not create
another edge until cleared. Ordinary full reset clears the fast latch and
resamples the reset KBC output; processor-only reset preserves board state,
RAM and RTC. All CPU reset/time/execution ownership remains unchanged.

Expected production delta is +50--90/-10--25 in Board implementation/headers
and the two App initializers; existing test extensions +70--120/-5--20.
The old instance test currently assumes KBC A20 can be disabled by writing
only port 92h; correct its instruction sequence to release both sources and
assert independent fast-gate readback instead of preserving that wrong
expectation. Extend the existing neutral-controller test for explicit decode,
edge/rearm, retention, cold reset and validation; preserve its absent-device
case. PC110's old UNSUPPORTED assertion for 92h becomes actual port behavior.
No new executable/framework or unrelated formatting is planned.

The receiving scope is the eight PC artifacts and four unpublished PC110
previews. MyNES and MyRaidenII do not link X86-NXVM and remain unchanged.
All other S55/T580 storage, PCIC, video/panel, guest and product gates stay
open. A successful reset does not establish those separate capabilities.

The owner confirms that ERROR and Easy-Setup's blue/cyan background differs
from the normal pink display. This matches S55's retained F65535 colour-path
finding. Preserve this as an explicit display acceptance requirement for both
pages; do not claim that all pixels are simply inverted or introduce a host
colour swap into S56. The existing planar/Attribute/DAC/PEL receiver owns
that complete repair and remains inside T580.

Focused tests initially fail because the new invalid-config case expected
plan allocation to validate Board materialization, and the retention case
wrote a PIC mask before its ICW initialization. Actual source inspection
establishes both contracts. Correct setup at the existing construction
boundary and initialize the PIC before preserving its mask; keep every
reset/A20/retention assertion. Both focused Board cases then pass on both
widths, including actual guest OUT92 before HLT, edge/rearm and reserved bits.
The original App pair separately passes. The first scratch CTest filter
selected only the App pair; the Board cases use their verified registered
names, and that partial selection is not claimed as Board verification.

Strengthen the existing optional Setup replay with one semantic result:
after the two already proved ordinary Enter inputs, it must observe the
actual processor reset and copied reset-vector PC, not merely the earlier
successful loader. Record the existing port transcript's 92h reads/writes
alongside KBC traffic, then stop at that completed reset boundary. Estimated
additional existing-harness delta +15--25/-5--10. No new executor, trace API
or production state. This gates the real system-control request; normal
post-restart Setup, saved configuration and guest boot remain separate.

## Final implementation and actual-change review

One Board helper derives effective A20 from the two physical inputs. Port
92h reads its own latch, masks reserved bits and requests the existing
processor reset only on bit zero's rising edge. Both PC110 constructors select
the existing route. Cold reset clears both inputs before resetting the KBC;
processor-only reset preserves Board/RAM/RTC. The existing Core reset owner
also finalizes active CPU execution, including SMIACT deassertion; the existing
Machine runner consumes RESET_REQUESTED without treating it as product stop.
Neither needs another implementation or interface change.

The actual C/H diff is production +52/-23 (net +29) across five paths and
tests +110/-11 (net +99) across five existing paths: total +162/-34,
net +128. Shared accounts for +141/-24; App accounts for +21/-10. There is
one new private helper, two real input latches and one public configuration
Boolean, no new function/enum, worker, timer, queue or test executable.
The directly affected root A20/fallback check is +9/-1; it retains the old
fallback/raw-RAM bans and now requires independent readback and one effective
mask publication. Two source/test manifests track their actual changed files.

Review covers every Board effective-mask writer, both route selectors, both
App construction paths, cold/processor reset, old port-92 expectations and
all callers of the existing effective-A20 observation. The latter remains an
existing qualification API; no redundant read/write/register-a20 aliases
remain. Lib, Emulator, X86 Core/Debug/IBMPC, MyNES and MyRaidenII are unchanged.

## Real BIOS proof and verification limits

Both host widths run the unchanged TFT BIOS, compare its complete 232,185-byte
Setup loader and deliver the two ordinary Enter inputs. Each now reports
`PC110:SETUP:processor-reset=1 pc=fffffff0 resets=5` and stops after
144,097,170 instructions, with the same four known exceptions and no Core
fault. The previous ABC2 HLT is no longer the terminal boundary. The copied
CPU is at F000:FFF0, CS base FFFF0000, CR0 60000010.

The retirement transcript records a 92h read of 00h and write of 02h; it does
not capture the final reset-triggering OUT 03h. Do not present it as a complete
three-access transcript. The original BIOS instructions, actual reset result
and direct guest OUT92 regression establish the mechanism independently.
This is reset servicing, not proof of normal post-restart Setup, configuration
persistence, selected DOS/Personalware/Windows, live input or panel output.
The native replay bounds remain 660 s x64 and 900 s x86.

| Final verification | x64 | x86 |
| --- | --- | --- |
| Repository units, Types isolated plus remaining suite | 1 + 711 pass, 27.69 + 217.17 s | 1 + 711 pass, 27.44 + 63.64 s |
| Four receiving PC builds and actual adjacent INI identity | 4 pass | 4 pass |
| Four actual profile floppy-boot cases | 4 pass | 4 pass |
| Default control lifecycle and Windows-directory checkpoint | 2 pass | 2 pass |
| PC110 real FDC reset / real boot handoff | 43.47 / 220.82 s, pass | 58.21 / 310.96 s, pass |
| Specialized checks | 123 steps pass | 123 steps pass |
| Original BIOS Setup/reset replay | pass | pass |

The Windows checkpoint is C-drive/Windows-directory listing, not Windows GUI
qualification. Each actual profile boot runs once with immutable owner INI
selecting overlay access. Actual wall times for XT/AT/DeskPro/default are
27.82/43.56/121.39/3.10 s x64 and 35.16/53.19/173.42/3.56 s x86.
The strict compilation check retains all 498 rows, zero deferred; the global
Types gate checks 1,501 tracked code/script files. Manifests and dependency
gates pass. No desktop, RDP, physical panel, DSTN guest or full PC110 product
qualification is claimed by these background checks.

Both stable Ninja/ccache graphs are reused and restored to the default profile.
Receiving builds reuse the already built Shared objects across all four Apps;
they do not configure a fresh graph per machine. Eight admitted PC EXEs are
deployed; four PC110 previews remain unpublished. The
[artifact inventory](t580-s56-artifacts.json) records all twelve hashes, PE
widths and embedded runtime version 0.5.0580. These files have no native PE
version resource; the inventory does not invent one. MyNES/MyRaidenII and
owner INI remain unchanged.

External media hashes still match their owner manifest: the 360K DOS 5.00,
1200K DOS 5.00a disk 1, 1440K DOS 5.00, DeskPro blank HDD and 50 MB Windows
HDD are unchanged. TFT BIOS remains
`232101C88466F311BCC32FBC215A4D7569F695CE19F9C07CA67CE2AEE5232312`.
The comparison-only decoded Setup reference remains
`7128F7C5D4907A8C7F1D7B8C0D64F8617CDA43464F36599D21ED7BBCCAB8FE8B`;
it is never injected into guest memory or committed.

Selected final log SHA-256 records, archived before cleanup:

| Log | SHA-256 |
| --- | --- |
| real-x64 | 677E1827F51D0038EA14ADD53F0497DAEE3A579DA8F2D482CEF3D423EBF516EB |
| real-x86 | FA95B1A73610984415D431C480FDB77E09FBCABC45C8C346EFA959523C6DC163 |
| units-x64 | 64C82FB3516155831047B8A5529608CCD3FFF7A057CB0372B4BBA51909383B7B |
| units-x86 | A50881C489272AC3D9458D2DBEBA5FDC91785555A8586F51D17BCC2EF8FF6432 |
| integration-x64 | B563877BD5C43734004830A1801EAA5A108C5B2908A174D53615B410F3302698 |
| integration-x86 | EC12925B362BB9CCCA95BDD580D6563DBCE450BD8DD7CA6A974FE8F1EF27D028 |
| gates-x64 | 7A4FAA03D5C73D53EDA89D35A7097E9569C994250DB87E85128C2F4177C82B95 |
| gates-x86 | 3F0A8D8BA4A4492D58F66EDE8767FB02AD3421C3A4563E252E3BAA8FCB1002DD |

S56's owned tasks are terminal; native process inspection finds no remaining
S56 job. The official cleaner previews 98 files / 7,013,740 bytes in exactly
`build/t580-s56-control`, including the hash-reviewed comparison reference.
Apply removes those generated files only; both stable caches remain.

## Remaining complete T580 receiver class

System-control/A20/reset is now directly proved. The existing ledger still
requires post-restart normal Setup/configuration persistence, two-slot
SunDisk/CF storage and selected guest, PCIC/card distinction, complete F65535
planar/Attribute/DAC/PEL scanout and inspection, panel/TFT/DSTN identity,
live input/lifecycle and four-artifact product qualification. The owner's
pink-background observation on ERROR and Easy-Setup belongs to that F65535
receiver. A fixed RGBI scanout currently ignores programmed palette values;
this is not a Lib/Window inversion to patch or an accepted display gate.
No original T-level requirement is transferred, erased or called complete.
