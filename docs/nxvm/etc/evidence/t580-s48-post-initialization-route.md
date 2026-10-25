# T580 S48 — Post-initialization route reconciliation

## Scope and baseline

The frozen baseline is `9fb5d7d33`, after accepted S47. This audit consumes
the existing qualification ledger's complete post-initialization receiver
batch, not a new product-completion predicate. It changes no production code,
public contract, firmware, INI or media. A temporary mode of the existing real
firmware integration harness records copied retirement/I/O observations through
normal Core execution. It is removed before delivery.

## Static receiver reconciliation

The admitted, unchanged TFT image contains separate mechanisms which must not
be conflated into a single POST or boot claim:

| Receiver | Contract and current boundary |
| --- | --- |
| Cold shadow and SMI initialization | Accepted S47 supplies CPU-only SMRAM, guest copies, physical SMIACT, actual ASMI service and RSM. This audit inherits that proof, not a full power-management model. |
| Staged CPU resets | Existing processor-reset route preserves RAM and peripherals. CMOS shutdown selection is firmware-owned; no host warm-signature write is admitted. |
| Post-initialization wait/service | F000:A8FD prepares several distinct memory/port predicates; A9CB calls DD3E, which requests vendor function 84 through the configured ASMI port. A functioning first ASMI is not proof of every later request or wait predicate. |
| Lower Flash/Setup image | F000:DD5D copies its trampoline into ordinary RAM and executes it. The trampoline saves ROMSET and E/F access registers, makes E/F external, toggles port 98 bit 2, invalidates cache, copies 128 KiB into 10000–2FFFF, toggles back and restores configuration. The caller checks the copied option header/checksum before invoking its loader. Current App has no port-98 owner and always selects upper Flash banks. |
| Setup entry | F000:8184 invokes that image-loader path. A source header, successful copy or reference's host decompressor is not proof that the real Setup executes. |
| Boot entry | F000:52BA writes zero to 8B, then the actual INT 19h is at F000:52BD. Entering INT 19h still does not prove selected device discovery, sector transfer or guest execution. |
| Product release | The diagnostic bypasses host monitor/UI only to exercise the same guest machine. It does not qualify live panel, input, lifecycle or publication. Existing owner INI has no selected boot medium and is preserved. |

### Source grades and exclusions

These firmware control sequences are observed/static evidence for a bounded
L2 receiver design, not an original chipset register specification or physical
timing formula. No recovered source or binary excerpt is committed.

The read-only Open-Source-PC110 `Discovery/BIOS-Flash/readme.md`, SHA-256
`A4A1885B2036FCA5AC6062583B74B45A3350F8EC65DADB8D535606DD14C7E123`,
records the four physical banks, the runtime upper-bank view and the
unavailable lower runtime banks. Its general boot-only description does not
define the complete cold decoder. The actual guest trampoline supplies the
more specific port-98 toggle/copy/restore requirement; it does not authorize
making all external C/D accesses permanent ROM aliases.

PC110-QEMU's `qemu/target-i386/pc110post.c`, SHA-256
`CE8C8E23064DDC29A1109C9C93BAA2C17E50148B5E66841E8B8E16494CCB85A0`,
contains explicit firmware redirects, synthetic warm signatures, Setup
handoff and boot-sector loading. PC110-EMU likewise has a host Setup-image
decompressor. These are research leads, not evidence that their underlying
hardware paths satisfy this project's real-firmware contract. None is imported.

## Design consequences

The Flash receiver belongs to the existing App firmware/configuration owner:
one port-98 latch drives the same E/F external read/fetch decoder; shadow access
continues to be independently register-selected. Both transitions invalidate
the existing Core memory view. Guest code performs every copy, checksum and
decompression. No second loader, BIOS service, frame renderer or clock is needed.
Reset value, supported control bits, high reset view and processor-only reset
retention must be stated and tested together before implementation. This is
not a claim that the missing Flash route causes the currently observed wait.

The later wait receiver must first establish the actual predicate, saved CPU
registers, ASMI admission and pending-input behavior together. It must not be
repaired by changing the guest memory flag, returning an invented successful
service or simply increasing an instruction ceiling.

## Actual route observations

The unchanged x64 image runs 200,000,155 instructions, with three staged
processor resets at 11,438,872, 11,860,908 and 11,931,579 instructions. It ends
on its containment, not a successful boot: status OK, reason BUDGET, PC 38000,
no terminal Core fault and no INT 19h. The run lasts about 521 seconds, below
the 600-second containment. Four delivered GP exceptions belong to the
firmware's existing protected-memory probes; this is not an exception-free
POST claim. Warm signature remains 0000; copied C header is 55-AA-60-EB.

An x86 replay contained at 32,000,027 instructions agrees on all three exact
reset counts, timer delta 166C, no terminal fault and no INT 19h. It terminates
in the same ASMI-entry phase, AX=5380/BX=8424. The copied port ring repeats
ASMI write 15EE, source B7=20, acknowledgement 5F and FA=1 rearm. Consequently
the later stall is not explained by absence of SMI entry or acknowledgement.

The first x64 sampler's read/write display labels were inverted because the
temporary ring stored retirement-direction values but the old printer expected
trace-event values. The x86 and final service samplers correct the printer;
PC/port/value data is unchanged. No conclusion relies on the inverted words.

### Predicate and downstream device reconciliation

One short x64 replay stops at the first actual RSM after reset three, at
11,933,595 instructions. It copies the existing observation's CPU facts at the
dispatcher, vendor function and internal service, rather than guessing from
the last PC:

| Point | Copied fact |
| --- | --- |
| 3DB6C / 3A774 | Firmware actually dispatches its installed vendor handler; DS=3000, ES=3800. |
| 3A77B | Saved request AX=5380 is read correctly. |
| 3A790 / 3A7DF | Function 84 selects internal handler 3002 with original BX=8424. |
| 3B002 / 38882 | The vendor handler calls its own internal AH=04 service. |
| 38A64 | The service's configuration read gives AH=0C, taking its existing bit-2 immediate-FF branch. No invented PIC polling or service result is added. |
| 3DBC8 | Actual RSM returns AX=53FF, DS=9FC0 and ES=0040. |

The preceding real guest I/O resets the floppy controller: it writes DOR=08,
clears its own completion byte at 40:3E, then writes DOR=0C. Its FC93E wait
requests INT 15h AX=9001; the subsequent wait tests that same IRQ-completion
flag, while issuing the observed ASMI service. The App does not configure an
FDC at all: its construction has zero floppy slots, its plan configures only
HDC, and no FDC ports/IRQ6/DMA2 route is installed. Thus the actual firmware
consumer now demonstrates a missing controller receiver, independently of
the separate lower-Flash gap. Returning a successful service or setting
40:3E in the host would hide, not repair, that omission.

The hardware reference identifies the actual controller as U22 SMC
FDC37C665IR, with the drive in the dock, **not** a Pluto-emulated controller.
Source: Open-Source-PC110 `Discovery/Pluto/readme.md`, SHA-256
`691DE9101FD0A1AC12E6E9AB9BEB9ABDF7A2507501E208814E322EDAC3E38533`.
Its chipset record separately identifies IRQ6. This supports a bounded L2
standard-compatible controller wiring receiver, not a claim that the existing
8272A model implements every Super-I/O configuration register or dock signal.

## Complete batch disposition and next implementation

The reset/SMI-entry/registered-vendor/RSM members have direct observations;
the required floppy reset/IRQ-completion receiver is absent, lower Flash
toggle/copy/restore is absent, and Setup/INT19/sector/guest/product remain
unqualified. Neither a larger budget nor the references' host boot shortcuts
can advance those members. All stay mapped to the existing T580 ledger.

The next cohesive receiver should wire the **existing** standard FDC route at
3F2/3F4/3F5/3F7 with IRQ6/DMA2 through the existing Board plan. It must prove
DOR assertion/release, reset completion, PIC delivery, guest acknowledgement
and an empty-drive disposition together, without inserting media or equating
an absent external drive with an absent integrated controller. Reuse the App
composition fixture and the existing real BIOS harness. No new Shared API,
controller, event queue or BIOS helper is needed. Audit the SMC-specific
configuration endpoints before claiming their coverage; do not enable them
merely because another PC personality happens to have them.

Port-98 Flash selection is a distinct next receiver with the design above;
it is not silently repaired as part of the FDC proof. Boot-device discovery,
selected lawful medium, actual sector handoff and live product qualification
remain later finite predicates under the original T completion standard.

## Verification and cleanup

Both temporary samplers compile with existing strict App flags in the original
x64/x86 Ninja/ccache trees. All temporary C changes are removed; restored
diagnostic executables rebuild from the exact committed source. Retained
production/test delta is +0/-0; no helper, API, state, test target or manifest
change remains. A documentation-only audit does not refresh published EXEs.
The restored existing App units pass 11/11 on each width (x64 7.85 seconds,
x86 1.53 seconds); documentation governance and whitespace checks pass.
The preceding S47 full-unit qualification remains baseline evidence, not a
new full-repository test run. No desktop, RDP, full POST or guest qualification
is claimed.

The following task-owned logs have their required facts copied above and are
hashed before removal after all owned processes have exited:

| Log | Bytes | SHA-256 |
| --- | ---: | --- |
| x64-route.log | 8614 | `4C116ACFD1834EA0FE36AFBCFB6B87630D04D118759B32BFD4DADD220035FF5E` |
| x86-route.log | 8813 | `D8EEFBCA40814AA100587D32E273EE11B084A8C557596DC97419BD6AEDB8A2B8` |
| x64-service.log | 10139 | `CEB5FE34628E7D96B98AAA46549307D24852EECD6A8FC53ACB05F6A4668C5B4B` |

The real cleanup tool previews and removes exactly those three logs (27,566
bytes). Stable Ninja objects and repository ccache remain intact; all owner
inputs and published artifacts remain unchanged.
