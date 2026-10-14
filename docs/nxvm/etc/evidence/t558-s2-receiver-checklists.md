# T558 S2 Implementation Receiver Checklists

Working audit at NXVM feadb8235. The nine receiver inventories below are
expanded by the concrete contract rows; unresolved admission facts remain
explicit, so this is not a blanket claim of implementation readiness.
External repositories and hashes are pinned by the S1 ledger and
[S2 availability record](t558-s2-source-availability.md). Each source must be
classified as original, reference observation, model or unsuitable bypass.
No register value is qualified merely because a boot path reads it.

## 1. 486SX Instructions

- Owner: Core chips/cpu; preserve existing families and decoder style.
- Required source: Intel family programmer manual instruction entries and
  architecture differences; selected silicon's feature documents. Original
  manuals are now archived with hashes in the S2 availability record. Chapter
  26 instruction entries and appendix D feature determination are the concrete
  reading anchors; full form/exception inspection remains open. The local
  embedded SX datasheet is variant evidence, not the whole specification.
- Checklist: every existing 386 form reused by 486; added integer forms;
  CPUID/ID-bit eligibility; cache/TLB operations; prefix and LOCK legality;
  all execution modes, operand/address sizes, memory faults and restartability;
  selected absent-FPU behavior. Partition legal, illegal and variant-optional
  forms before implementing. Do not turn the undefined-opcode RSM/WBINVD/CPUID
  placeholders into unconditional success.
- Proof: table-driven execution against frozen form/exception expectations,
  including memory-side effects and flags, plus retained CPU regressions.

## 2. 486SX System, Reset And Timing

- Owner: Core CPU and its single executor/bus boundary.
- Required source: Intel hardware/developer manual reset, bus, cache, SMM,
  paging and timing sections: Intel 273021-001 chapters 8--10 and appendix D.
  Table D-2 identifies family 4/model 2 as SX, distinct from model 4 SL;
  exact PC110 silicon applicability and feature bits remain open.
- Checklist: reset hidden CS/base; CR0/flags/reserved bits; alignment checks;
  paging protection/A-D behavior; debug and interrupt priority; shutdown,
  NMI/SMI/RSM and warm reset; cache invalidation, DMA visibility, bus widths,
  wait/HOLD/HLT; timing for every legal form and exceptional path.
- Proof: real fetch at reset alias, mode transitions and save/restore state,
  finite timing ledger with explicit L2/L3 rather than QEMU icount equivalence.

## 3. Board, Chipset And Firmware Mapping

- Owners: independent chip mechanism in Core; PC110 decode/wiring in App.
- Sources: hardware-reference Chipset 13h--13k, actual three config text dumps,
  RAM-Module live study, board netlist; compare QEMU scamp_read/scamp_write but
  reject its unqualified EC/ED MCU assignment. VL82C420 PDF is an article.
- Checklist: revision/population; cold versus warm reset; four config windows
  and unlock/relock; bank geometry/alias tests; ROM/shadow read and write routes;
  A20; PIC/PIT/DMA/RTC clocks and IRQs; keyboard/SIO/PCIC decode; partial
  construction teardown. Inventory opaque registers explicitly.
- Proof: isolated mapping and reset tests plus real guest accesses to the same
  routes; no guest-IP dispatch. Resolve EC/ED, 4Fh and firmware address windows
  before freezing the corresponding owner. BIOS offsets 3DBDEh/3DBEBh were
  independently confirmed as EC/ED indexed access, not their register semantics.
  AT helper limitations are recorded
  in the S1 ownership audit.

## 4. Keyboard, Pointer And Power

- Sources: hardware-reference Pluto, PSU-MB-M38 and Trackpoint notes; M38813
  original datasheet candidate; EMU KBD_MCU/POWER_MCU notes only as models.
- Checklist: KBC command/data phases, OBF/IBF/AUX and IRQ1/12; scan translation,
  make/break, repeat and special keys; PS/2 pointer packet/status/reporting;
  inking-pad distinction; A20/reset outputs; power command/reply, suspend/wake,
  status LCD and battery/adapter states. Guest power state is not host pause.
- Proof: actual KVM-to-guest input path and guest command responses, reset and
  pending-event ordering. Decide bounded external-protocol model versus MCU
  execution from firmware validity and original evidence, not loader success.

## 5. F65535 Display And Font ROM

- Sources: C&T 65535 March 1994 Revision 2.0 register manual (combined PDF
  opens with a different PO27.3 product sheet); hardware-reference display and
  font studies; QEMU fontrom_io_read/write and fontrom_window_read comparison;
  EMU f65535_vga_read/write and planar memory helpers are partial references.
- Checklist: VGA compatibility and extension banks; reset/unlock; VRAM size,
  banks, planes, latches and DAC; text/font/cursor; timing and retrace; panel
  versus CRT settings; font-window bank/enable/read-only semantics; graphics
  and return-to-text. Reject guest-specific font-metric repair routines.
- Proof: register/memory patterns through the actual Core capture and Machine
  frame path, then Window presentation. Original timing/register sections must
  be inspected before qualifying values; a screenshot is not that proof.

## 6. PCMCIA, Flash And Storage

- Sources: Ricoh EA-028-9804 printed pp18--27 onward; SMSC FDC37C665IR IDE
  p116 and config p120; hardware-reference Live-Dump 14a and BIOS-Flash notes;
  existing Core HDC/FDC/media interfaces. Do not conflate PCIC and onboard ATA.
- Checklist: socket identity/power/detect; attribute/common/I/O windows and
  IRQ/DMA; ATA master/slave selection, IDENTIFY/CHS/LBA/PIO and errors; firmware
  flash command/protection/reset; dock floppy presence; insertion/removal and
  storage mode behavior; programmed state must not overwrite immutable masters.
- Proof: actual guest-sector transfer on each selected device path, boundary
  and removal failures, absent devices and readonly behavior. Slot-zero floppy
  and second-disk limitations require one-path Core adaptation, not fake devices.

## 7. Audio And Auxiliary Devices

- Sources: ESSPROBE assembly/observations; ES488 reconstruction; Yamaha YM3812
  original manual candidate; SMSC UART/infrared/parallel/power chapters at
  printed pp75/89/90/111; dock/modem reconstruction for remaining population.
- Checklist: DSP reset/version, command FIFO, sample clock/DMA/IRQ; OPL timers
  and output; speaker path; UART FIFO/interrupt/baud; parallel handshakes and
  modes; IrDA; modem and dock presence/power. External optional cards are not
  the onboard audio device. Each unsupported item needs explicit disposition.
- Proof: deterministic PCM/IRQ/register tests, not physical speaker volume;
  guest and device integration without a second audio scheduler.

## 8. Real Firmware And Guest Boot

The storage and auxiliary receivers also consume the concrete contracts below;
this section's guest checkpoints do not substitute for their direct tests.

- Sources: hashed BIOS/font candidates; genuine guest firmware execution;
  QEMU run-realbios/POST and EMU boot tools only for locating observations.
  POST completers, DOS interception, extracted Setup entry and patched boot
  scripts are excluded from success evidence.
- Checkpoints: reset fetch; chipset memory initialization; basic peripherals;
  firmware video initialization; keyboard/POST completion; F1 Setup and return;
  firmware-selected boot sector; DOS prompt with functioning keyboard/disk;
  DOS/V font and Personaware path; reset/reboot and retained-media integrity.
- Freeze precise guest version, hashes and positive observations before runs.
  Windows compatibility is a separately specified checkpoint, not inferred
  from DOS or a reference screenshot. Budget expiry is never a checkpoint.

## 9. Product And Guest Qualification

- Sources: actual Core product startup/factory and four App bindings; Emulator
  Machine/Session/UI contracts; current repository test/artifact authorities.
- Checklist: original DSTN and upgraded TFT build-fixed identities and one
  lowercase mypc110.ini; embedded firmware, with no runtime model/BIOS loader;
  RAM/media/display configuration validation; banner/help/debug/hotkeys;
  lifecycle and target changes; failures/cleanup; x86/x64 PE/release identity;
  full admitted guest matrix and preservation of existing four PC Apps/MyNES.
- Proof: production route tests, full required suites and correct assets;
  no alternative runtime or product-only test substitute for chip ownership.

## Detailed Contract Ledger

### Inspected Storage And Super-I/O Contracts

These rows supplement receivers 3, 6 and 7. Original-manual identities/hashes
are in the availability record; page numbers are printed pages, not viewer
indices. They are requirements, not claims of implemented chip support.

| ID | Evidence and behavior | Owner and required proof |
| --- | --- | --- |
| SIO-1 | SMSC FDC37C665IR p120: two successive 55h writes to 3F0h enter configuration; intervening write to another address prevents entry; AAh exits. CR0--CRF use index/data 3F0h/3F1h. | Core chip owns configuration; App owns decode wiring. Test incomplete/interrupted unlock, locked access and exit through real bus I/O. |
| SIO-2 | SMSC p19: outside configuration, 3F0h/3F1h reads reach FDC status A/B; configuration suppresses those reads. IDE AT decode is 1F0h--1F7h/3F6h/3F7h and can be relocated. | One decoder owns routing, not overlapping always-on handlers. Test mode-dependent routing and relocation; review shared 3F7h bit behavior before implementation. |
| SIO-3 | SMSC pp6/122: power-up establishes configuration defaults; RESET generally preserves them. Page 124 explicitly allows hard/power-up reset to release CR1 bit7's configuration lock. | Board reset reason uses the existing reset route. Test cold defaults, ordinary preservation and the documented lock exception; no blanket register zeroing or unconditional preserve-all rule. |
| PCIC-1 | Ricoh mapping chapter: five memory/two I/O windows per slot; memory uses 4-KiB boundaries and signed card offsets, with independent common/attribute, wait and data-size controls. | Core Ricoh owns translation; App owns sockets. Test boundaries, offsets, disabled windows and attribute/common distinction. Resolve overlap priority from register details before coding. |
| PCIC-2 | Ricoh p27: index 00h identity is 83h; index 01h reports slot/power/ready/WP inputs. WP indication alone does not block writes; window protection controls that. | Test immutable identity, empty/unpowered sockets, transitions and independent write protection; no constant ready/present byte. |
| PCIC-3 | Ricoh pp24--25: PCMCIA-ATA is selected per socket with index 1Fh bit0. | Test the explicit mode separately from onboard ATA; controller capability does not establish PC110 CF wiring. |
| PCIC-4 | Ricoh p66: either CD input changing latches detect-change; interrupt, resume and removal configuration-reset controls are distinct. | Test insertion/removal, acknowledge, disabled/enabled IRQ and selected register reset; retain one media owner. |
| ATA-1 | Core chips/hdc already provides ATA PIO, LBA28 capability and unit callbacks; board-base/hdc.c `hdc_media()` maps master/slave IDs. Machine preparation rejects fixed_disk_image[1]. | Reuse chip/board path; lift upper-layer restriction only with real two-device composition. Test selection during pending transfer, absent slave, unit isolation, CHS/LBA limits, reset and readonly failures; retain legacy personalities. |
| ATA-2 | Live-Dump 14a reports internal flash master and CF slave; Storage section 1 contradicts it. Reconstructed J11 nets corroborate IRQ14/FDC_IDE7 and U42 routes, but CSEL straps do not independently settle device selection. | Board/storage receiver reconciles selected-board straps/mode and firmware/device-selection observations before freezing topology. Test each unit selected/absent and shared-channel reset; do not infer wiring from Ricoh capability or guest drive letters. |
| FL-1 | Intel 290448-005 p15 section 3.1.2.2: top-boot x8 array has 128-KiB and 96-KiB main blocks, two 8-KiB parameter blocks and 16-KiB boot block. | Core chip owns array/command state; App owns bank/decode/power pins. Test each block boundary and isolation using byte addresses, not the 28F200 word-address map. Immutable embedded input initializes disposable chip state; external master never changes. |
| FL-2 | Intel pp15/18: ID and status commands do not require high programming VPP; x8 top-boot ID is 89h/7Ch. RP# protects boot-block mutation separately. | Test ID/status with low programming VPP, accepted versus electrically gated command cycles, protected boot versus main blocks and reset. Reject the reconstruction's claim that low VPP alone suppresses all command writes. Board gate remains independently unresolved. |
| FL-3 | Intel pp18--21, command table 4 and status table 5: setup/data or setup/confirm phases, read-mode selection, sticky failure bits and operation-triggered VPP status. | Cover both program-setup opcodes, erase confirmation/cancel, busy read-status, clear-status, return-to-array and invalid sequences. Programming clears bits only; errors persist until cleared. Test low-VPP failure without array corruption; no unconditional successful write. |
| FL-4 | Intel pp20--22: erase suspend/resume is distinct from nonsuspendable byte programming; RP# can terminate operation. | Use existing guest scheduler for ready/busy transitions. Freeze documented timing/model grade before implementation; cover suspend reads outside erased block, resume, reset and interrupted-operation disposition. No host sleep, second worker or generic recovery state machine. |

SMSC p122's CRE default-versus-bit-column inconsistency was confirmed visually:
01h versus 82h. The pinned 86Box 665 model uses 02h, not independent PC110
silicon proof. Resolve applicable revision/identity before assigning an exact
reset constant. Check RB5C396-specific exceptions in the
Ricoh family manual. Full timing/UART/FDC/ECP review remains required; these
rows do not qualify those uninspected details.

### Inspected Display And Input Contracts

| ID | Evidence and required behavior | Owner and proof gate |
| --- | --- | --- |
| VID-1 | C&T Revision 2.0 p16: extension index/data always accessible at 3D6h/3D7h; XR15 groups protect standard registers, p91. | Core chip owns registers; test each protection group's affected bits and unaffected neighbors, reset and index/data access. No blanket VGA-extension unlock from another chip. |
| VID-2 | C&T p74: ninth-dot line graphics, split-screen panning and VSYNC/XR60 blinking are guest-defined. Panel timing is independent from CRT timing, p16. | Reuse Core video clock/capture; test programmed timing and text-to-graphics representation, cursor phase and reset. Window consumes ordinary copied frames, Console only true text; no Shared raster extension. |
| VID-3 | QEMU fontrom model supplies a concrete 1160h--1163h/DE000h reference but does not enforce enable or reset. Hardware-reference Chipset 13e/Bowman font sections provide observation context. | App owns Bowman font decode; immutable bytes through Core ROM mapping. Test bank endpoints, enable/disable, reset, ignored writes and overlap with RAM/shadow after those contracts are evidenced. Do not copy QEMU priority 10000 as board semantics. |
| VID-4 | C&T original supports 512-KiB/1-MiB arrangements; reconstruction includes different fitted DRAM. | App freezes original board memory/panel identity; Core mechanism accepts validated capacity. Prove alias/bank boundaries and panel/CRT modes, not a screenshot or recreation BOM. |
| IN-1 | Pluto section 6c describes M38813 DBB command/data channels and KBC OBF/IBF/AUX behavior; keyboard image top-aligned vectors independently agree. | Reuse Core kbc8042/keyboard mechanisms only for matching protocol. PC110 wiring owns IRQ/A20/reset/SMI; test command/data phase, reply ordering, reset with queued input and real KVM make/break route. |
| IN-2 | Keyboard study has three scan-set tables but unresolved physical key/Fn mapping; Trackpoint reports standard PS/2 responses. Core ps2mouse interface explicitly says incomplete three-byte AUX model. | Audit each required command against Core implementation. Test reporting enable, status/reset, movement/buttons and backpressure. Inking pad remains a distinct protocol; do not call mouse support complete input. |
| PM-1 | PSU study proposes a truncated power ROM mapping; reference emulator exposes synthetic firmware metadata rather than executing MCU code. | Power receiver first resolves vectors/peripherals or specifies an evidenced external-protocol model. No fake vectors, always-ready mailbox or checksum registers claimed as real hardware. |
| PM-2 | Pluto's observed 15E8h/15ECh mailbox and 35EAh/35EBh bank must be reconciled with newer Chipset evidence excluding EC/ED as power owner. | App board glue owns decode; chip model owns protocol. Enumerate request/reply/busy/error/reset and suspend/wake state; test guest power separately from Emulator pause/stop, without another lifecycle controller. |

### Inspected CPU Delta And Timing Contracts

The Intel family programmer manual section 21.3 is the finite starting delta
from the existing 386, not a requirement to replace the interpreter. Its
chapter 26 is the complete form inventory; later SL-enhanced features come
from the selected-silicon supplement, not an unconditional family switch.

| ID | Original anchor and requirement | Current-code receiver and proof |
| --- | --- | --- |
| CPU-1 | PRM 21.3.3 and chapter 26: BSWAP, XADD and CMPXCHG are new application instructions. | cpu_instructions.c existing tables; no named implementations found in inspected Core. Enumerate opcode/width/register-memory forms, aliasing operands, flags, LOCK legality, read/write faults and restart state. Retained pre-486 profiles must still reject 486-only forms. |
| CPU-2 | PRM 21.3.3, INVD 26-171, INVLPG 26-172 and WBINVD 26-281: distinct cache/TLB operations and privilege contracts. | WBINVD currently calls UndefinedOpcode; INVD/INVLPG have no named implementations. Test privileged/illegal encodings and real memory-translation invalidation. If no cache data model is needed, document why observable behavior remains correct; never equate all three to success-only no-ops. |
| CPU-3 | PRM 21.3.3 changes MOV test-register access and cache/TLB test registers. | Audit 0F24/0F26 dispatch and test-register representation explicitly. Direct tests cover legal indices, reserved values, CPL and side effects; do not omit test registers merely because firmware has not reached them. |
| CPU-4 | PRM 21.3.1--2: EFLAGS.AC with CR0.AM controls vector 17 alignment checks. | cpu.h already defines AC access, which does not prove checking exists. Audit each logical memory access, stack/string and fault precedence; test modes/CPL/AC/AM combinations and unchanged state on failure. |
| CPU-5 | PRM 21.3.4--5 adds CR0 NE/WP/AM/NW/CD and CR3/page-entry PCD/PWT. | Audit cpu.c and control-register instructions, page walk and reset masks. Test supervisor write protection, cache attributes and absent-FPU behavior; preserve the old profiles' reserved-bit rules. |
| CPU-6 | PRM 21.3.6: descriptor accessed-bit locked read/write occurs only when that bit was clear, unlike 386. | Descriptor-load memory path must select family behavior. Test already-set descriptors in readonly memory, clear descriptors, bus effects and precise faults; register-result equality alone is insufficient. |
| CPU-7 | Intel 273021-001 appendix D: CPUID/ID are feature-qualified; chapter 8 covers applicable SMM. Intel 272769-004 pp23--24 supplies a concrete SX candidate: maximum leaf 1, model 2, EDX=2 (VME, no FPU), reset ID clear and separate reset signature. Later package applicability still needs reconciliation with the observed 042B PC110. | CPUID and RSM currently raise undefined opcode. Freeze mask and SMM applicability; prove VME execution if advertised, reset ID/signature, reserved leaves and SMM entry/exit. Do not infer PSE or implement only a feature bit. No guest-address special cases. |
| CPU-8 | PRM 26.2.2.3 clocks assume cache/TLB hits, aligned operands, simple EA, no exception/contention/write-buffer delay; Appendix E defines penalties. | cpu_timing*.c must add a selected-486 ledger, not reuse 386 values or QEMU icount. Separate nominal instruction cost, evidenced penalties, external bus waits and interrupt/fault cost; record accuracy grade for every form. |
| CPU-9 | Intel developer chapters 8--10 supply reset, SMM, bus and low-power mechanisms; PRM 23.8 states real/V86 compatibility with 386. | Core x86 executor remains sole timeline owner. Test reset fetch alias/hidden CS, paging/mode changes, HLT/HOLD/IRQ/NMI/SMI priority and resume, DMA visibility and shutdown without a second execution loop. |

Profile admission also audits comparisons rather than merely appending an enum:
`cpu_interface.h` currently ends at 80386, cpu.c contains exact-80386 branches,
and cpu_instructions.c has both lower-bound comparisons and a maximum-profile
validation rejecting values above 80386. Every such site needs a recorded
retain/generalize/family-specific decision, including decoder probes and
timing selection; changing all equality checks to inequalities is not valid.

### 486 Instruction Form Acceptance Matrix

Original anchors below are printed PRM pages, inspected in the archived 1992
manual. Nominal clocks retain its chapter 26 assumptions; system timing still
owns penalties and board/bus behavior. These are acceptance rows, not copied
implementation or a claim that the complete inherited 386 set is re-audited.

| Family / original anchor | Finite forms and key assertions | Negative / precision proof |
| --- | --- | --- |
| BSWAP, 26-35 | Eight 32-bit registers, 0F C8--CF; 11223344h becomes 44332211h; flags unchanged; nominal 1 clock. Check decoding consumes no ModR/M byte. | Pre-486 rejection, prefix legality and instruction length. The manual declares the 16-bit result undefined: do not invent a normative 16-bit byte-swap result or silently turn it into a documented #UD. Record selected deterministic model separately if exercised. |
| CMPXCHG, 26-62--63 | 0F B0/B1 with 8/16/32-bit operands, register/memory destinations, both address sizes, equal/unequal comparisons and accumulator/source/destination alias cases. Flags follow accumulator minus original destination. Nominal register 6; memory equal 7, unequal 10. | Segment write permission, page/stack/limit faults, alignment contract and LOCK memory versus register legality. Observe bus/write behavior as well as final values; define failed-comparison memory access from applicable evidence rather than assuming no write requirement. No partial architectural commit on fault. |
| XADD, 26-282 | 0F C0/C1, 8/16/32 bits, both address sizes, register/memory and same-register/high-byte aliases; source receives old destination and destination receives sum, with ADD flags. Nominal register/memory 3/4. | Carry/overflow/zero boundaries, access faults, alignment and LOCK legality. Audit operand commit order for aliases, not only distinct-register examples. The 1992 #NM text needs reconciliation as noted in source availability; do not copy it into the integer path without corroboration. |
| INVD / WBINVD, 26-171/281 | 0F08/0F09; real/protected/V86 modes and CPL matrix; flags unchanged. Distinguish discard from write-back signaling. Nominal 4/5, not a bound on external completion. | Privilege #GP, illegal prefixes and old-family rejection. System receiver proves visibility and any external bus effects; instruction decoding alone cannot close this row. |
| INVLPG, 26-172 | 0F01 /7 memory-address encoding, 16/32-bit effective addresses and segment selection; invalidates selected translation. Nominal 12 for a TLB hit only. | Register operand #UD, protected nonzero-CPL/V86 privilege behavior, old-family rejection. Address calculation is not an ordinary data load; avoid manufacturing data reads/page faults. Hit/miss and translation-state behavior require system proof. |

For each row the instruction task records actual decoder entry, direct test,
source grade and remaining system handoff. Optional CPUID and RSM are separate
CPU-7 rows, not extra universally supported instructions inferred from this
1992 base-family table. Tests must not use the implementation under test to
calculate their own expected flags or cycles.

### Board And Qualification Receivers

| ID | Evidence / production anchor | Contract and proof |
| --- | --- | --- |
| BRD-1 | Chipset 13j/13k, scamp_config.txt, block2_config.txt, eced_config.txt, block2dump.asm; BIOS indexed-access routines independently inspected. | Core independent configuration/memory model, App addresses/wiring. Test 24/25 unlock via FC23/F023/C023/0023 reads and relock, 74/76 indexing, EC/ED gate, RAM geometry/aliases and warm-reset preservation. Observed running-register dumps are not cold-reset defaults. |
| BRD-2 | RAM-Module eced-dram-regs-live.md retracts older memory-size interpretations; current Core AT preparation assumes an existing fixed ROM organization. | Freeze actual populated memory and permitted module combinations; reject unsupported capacities rather than silently rounding. BIOS flash, shadow RAM and font ROM are separate read/write owners. Test CPU reset alias and low aliases, disabled decode, readonly master bytes and writable shadow independently. |
| BRD-3 | Existing core/board-base PIC/PIT/DMA/RTC, hdc/fdc and Core Machine driver provide composed routing. | One guest scheduler, one device instance per fitted mechanism and one teardown owner. Prove IRQ cascade, DMA requests/terminal count, RTC/NMI mask, KBC A20/reset, display and SMI connections; staged absent devices remain visibly unqualified. |
| BOOT-1 | Hashed BIOS candidate reset entry plus actual Core CPU fetch path. | Cold reset must fetch embedded bytes at the real alias, initialize RAM and devices through guest I/O, then reach firmware POST completion. Evidence is CPU/bus observations plus framebuffer and input, not host-generated POST text. |
| BOOT-2 | Genuine BIOS Setup path; historical decompressed body and unpacker are available as pinned Git objects in the S2 availability record. The unpacker conflates END, truncation and invalid-code termination; reference run-realbios.sh injects POST support and extracted Setup. Neither proves guest decoding. | Observe guest loader relocation, flash-bank reads, destination writes and successful termination before comparing decompressed bytes. Enter Setup using guest F1 through KVM/keyboard, change an admitted setting, exit and reset. Record expected persistent state and image hashes; do not boot a substitute Setup floppy or inject its body. |
| BOOT-3 | Real firmware disk path, selected owner-supplied DOS/DOS-V media and font ROM. | Boot sector is fetched through ATA/FDC, shell accepts a typed command, directory/read/write round-trip works on a disposable overlay. Validate Japanese font output and installed drivers without host INT13/DOS services or removed EMM386. |
| BOOT-4 | Owner-supplied Personaware and explicitly selected Windows corpus. | Fix exact version/image hashes/configuration and expected shell/application/input/storage milestones before execution. Personaware must use its guest driver path. Windows desktop alone does not prove suspend/audio/storage; each admitted capability has its own positive checkpoint. Missing media blocks that qualification, not permission to substitute a modified reference disk. |
| QUAL-1 | core/product/startup_interface.h vm_app_run(name, ini_name, binding); four Apps supply fixed bindings and explicit INI identity. | MyPC110 uses the same entry/INI grammar/Emulator lifecycle. Test stopped start, pause/resume, stop, reset-to-paused, debug access lease, title/target and mouse ownership; no PC110 command-provider fork. |
| QUAL-2 | Current App CMake artifact declarations and test/core/setup plus App unit/integration split; owner-selected two compiled PC110 variants. | Publish mypc110_x86.exe, mypc110_x64.exe, mypc110_tft_x86.exe and mypc110_tft_x64.exe in assets/mypc110, with embedded externally supplied ROMs, internal version, PE identity and matching published hashes. Verify both panel/firmware bindings without cross-variant fallback. Chip tests live under Core, machine-specific wiring/guest tests under App; existing four-PC and MyNES regressions retain their owners. |
| QUAL-3 | Programme feature ledger and source policy. | Every required device/guest row ends with proof or explicit owner-approved disposition; missing originals or passing mock tests cannot silently erase a row. Preserve media/INI masters and report unexecuted desktop/RDP checks separately. |

The inspected external archive filename inventory currently has no PC110 or
Personaware/DOS-V-named media candidate. This is a discovery result, not proof
that the owner has no such media elsewhere. QEMU's disk README explicitly
requires owner-supplied copyrighted data and describes modified EMM386/volume
geometry variants; those variants cannot become NXVM's stock-guest baseline.
The firmware-boot receiver must record actual approved input identity before
running BOOT-3/4. This research task does not acquire or alter protected media.

### Audio And Auxiliary Finite Receivers

| ID | Source and limitation | Implementation / verification requirement |
| --- | --- | --- |
| AUD-1 | ESSPROBE.ASM + reported results: reset base+6, read data base+Ah, read availability base+Eh, E1 version and E7 ID; not a full DSP manual. | Core independent DSP owns command/reply and sample state; App owns configured decode/IRQ/DMA. Test reset edge/delay, readiness, response FIFO and command parameters with actual bus reads. Reject the reference placeholder returning AA from the status port. |
| AUD-2 | ES488 reconstruction and ESSPROBE identify SB2-class behavior; pinned 86Box snd_sb_dsp.c `sb_exec_command`/`sb_dsp_reset` provide concrete SB-compatible comparison paths, not exact ES488 timing. | Before PCM implementation, freeze supported command forms against documented SB behavior and observed ES488 differences. Required checklist: direct DAC, single/auto-init DMA, sample time constant, block count/terminal count, pause/resume/reset, IRQ acknowledgment, ADC disposition and error behavior. No speaker-only completion or fixed successful replies. |
| AUD-3 | Yamaha LSI-2138123 pp3--6, visually reviewed: two timers, reset, address/data/status, two operators per channel, melody/rhythm and waveform control. Pinned 86Box vendored ymfm_opl.cpp supplies a synthesis comparison entry. | Core OPL2 owns synthesis/timers; App owns clock/decode and mixing. Test timer status/masks/reset, register groups, key-on/envelopes, rhythm and deterministic PCM vectors. Distinguish functional accuracy from bit-exact output; review synthesis reference before choosing a numeric model. No code import authorized. |
| AUD-4 | Existing Core PIT/gate path, Yamaha/ESS board reconstruction and existing Emulator audio delivery contract. | Mix speaker/FM/PCM once on guest time before host delivery. Verify silence, clipping, partial acceptance, reset/stop and no replay. Host mute or physical audibility is never a pass/fail oracle; no second worker or Lib backend. |
| AUX-1 | SMSC original UART/IR/parallel/power chapters pp75/89/90/111 and configuration p120; Pluto dock/COM2-mux observations. | Core owns UART/FIFO/baud/interrupt and parallel/IR protocol; App owns connectors/mux/power. Cover disabled/decode, loopback/line status, FIFO/IRQ, timeout, reset and mode transitions. Dock absence must withdraw routing, not leave successful phantom devices. |
| AUX-2 | Pluto and modem/dock reconstruction; protocol and external endpoints not yet fully evidenced. | Enumerate modem, inking, infrared, dock floppy/serial/parallel and analog controls separately. Choose implementation proof or seek explicit disposition for each; no implied exclusion from a successful DOS boot. Keep host backend choice outside chip logic. |

## Bounded Admission Decisions

### One Implementation Owner Per Cross-Task Contract

The nine tasks are delivery stages, not nine independent implementations.
The following handoffs resolve overlap in the inventories above. A consumer
tests its wiring and consequences, not a copied register/state implementation.

| Cross-task surface | First owner and deliverable | Later consumer / exit restriction |
| --- | --- | --- |
| CPU-1 and CPU-3 instruction forms | Instruction receiver owns decode, operand/flag/privilege rules, test-register instruction semantics and form tests. | System receiver adds the corresponding cache/TLB effects where applicable and uses the same instruction path. |
| CPU-2 cache/TLB operations | Instruction receiver owns encodings and privilege/fault entry; system receiver owns translation/cache visibility and invalidation effects. | The instruction task must name effects still unqualified; it cannot pass them as successful no-ops. Only the system receiver admits the profile for board use. |
| CPU-4--6 memory/system delta | System receiver owns alignment checking, CR0/page protections/cache attributes and descriptor accessed-bit bus behavior. | Instruction receiver inventories affected forms and preserves exception entry; it does not build a second memory checker. System tests exercise the real instruction path. |
| CPU-7 selected feature behavior | Instruction receiver owns CPUID/ID eligibility and leaf encoding from the frozen feature record; system receiver owns advertised CR4 capabilities and SMM entry/state/RSM. | Feature values and legal RSM behavior are reconciled before board admission. No claim that decoding RSM alone implements SMM. |
| CPU-8--9 timing and reset | System receiver owns the complete selected-family timing/reset/execution contract. | Device/board tasks consume the existing single timeline and signal interfaces, not another CPU loop. |
| SMSC configuration and decode | Board receiver establishes one configuration owner and mode-dependent routing, including SIO-1--3 and its reset reasons. | Storage adds ATA/FDC endpoints; audio/auxiliary adds UART/parallel/IR endpoints. Neither duplicates unlock/config state. Unconnected endpoints remain explicitly unqualified until their task. |
| Firmware array versus board mapping | Board receiver owns CPU alias/shadow/bank/decode and pin wiring; storage receiver owns Intel flash command/array/busy/protection behavior FL-1--4. | A readonly reset mapping is a staged dependency, not completed writable flash. There is one final array owner; shadow RAM is separate. |
| Firmware input preparation versus boot acceptance | Board receiver's initial asset-binding batch owns admitted external input identities, slot sizes/mapping and any separately approved offline panel restoration. | Display consumes the frozen two panel pairings; boot receiver executes and qualifies them. Do not postpone preparing the board's required inputs until the later boot task, or interpret preparatory byte checks as successful boot. No ROM mutation is admitted by this planning row. |
| Keyboard, pointing and inking | Input/power receiver owns IN-1--2 and the inking protocol. | Auxiliary inventory checks this delivered coverage but does not implement another pointer/inking path. App supplies physical routing; neutral KVM input remains unchanged. |
| Dock and power state | Input/power receiver owns guest power requests and suspend/wake; board owns presence/decode wiring. | Storage owns dock FDC transfers; auxiliary owns dock serial/parallel behavior. One presence/power fact drives the endpoint routing; no per-task competing dock manager. |
| Firmware and product qualification | Boot receiver owns positive reset/POST/Setup/guest milestones on admitted images. | Final qualification reuses those results on the unchanged baseline and adds product/lifecycle/release proof; changed dependencies invalidate affected evidence. It does not replace missing chip tests with a boot screenshot. |

This partition does not freeze a new public interface. At each implementation
admission, inspect current interfaces and name the minimal changed fields or
callbacks, their owner and failure semantics before code changes. Cross-task
handoff records must link both the delivered direct test and the still-required
composed proof; an interface stub is never evidence that a consumer is wired.

These decisions are research gates, not additional implementation mechanisms.
An affected row cannot be closed merely by accepting a reference emulator's
default. Unaffected, documented chip work can be planned independently; this
does not authorize publishing a partially evidenced PC110.

| Decision | Responsible receiver | Concrete resolution and acceptance evidence |
| --- | --- | --- |
| Exact SX variant and feature mask | 486 instructions/system | Reconcile the quoted 042B identity with a complete CPUID/ID-toggle capture and applicable Intel silicon documentation. Freeze reset signature, CPUID leaves, CR4 features and SMM eligibility separately. The manuals already support the base 486 instruction delta; unknown optional features remain unadmitted, not silently absent. |
| Original board and RAM population | Board | Identify a stock board revision and populated parts, separating recreation BOM from original observations. Cross-check module geometry against EC/ED captures and firmware sizing accesses. Record allowable INI capacities and exact aliases; no rounded RAM-size fallback. |
| Private chipset windows and reset values | Board | Trace the cited BIOS indexed access/unlock routines and compare actual configuration dumps. Classify each reset value separately from post-POST values. Resolve port 4F read semantics and early flash-bank selection; unsupported hypotheses require a sourced model decision before implementation. |
| Stock and modified video firmware | Display/firmware | Owner selects both as compiled variants. Current candidate matches all 18 TFT values. The author's bidirectional patch is source-confirmed: assess unchanged stock input or explicitly admitted offline restoration with exact-site, integrity and byte-round-trip proof; preserve master and derived hashes/provenance. Independently qualify both. The in-system flasher is not validated and is not imported. No runtime model/firmware selection. |
| Font enable/reset and video population | Display | Inspect the controller's applicable register sections and board decode/firmware accesses. Confirm stock 512-KiB candidate against the selected board, and resolve Bowman font enable/reset/overlap independently of QEMU's permissive callback. Direct boundary tests must be derived from the resulting contracts. |
| Keyboard/power implementation model | Input/power | Keyboard vectors support its candidate alignment; power ROM origin/vector coverage does not. Choose MCU execution only after format/peripherals are proved, or specify a sourced external-protocol model with enumerated commands, replies and timing. Missing power vectors cannot be fabricated; Fn, inking and suspend/wake remain explicit coverage rows. |
| ATA topology and writable flash | Storage/board | Corroborate reported onboard master/CF slave with wiring or firmware/device observations. Intel 290448-005 now establishes chip behavior (FL-1--4); resolve board WE/RP/decode separately. Low programming VPP alone does not inhibit ID/status commands. Do not add a special case for floppy boot. External ROM masters stay immutable. |
| ESS and auxiliary behavior | Audio/auxiliary | Map each required DSP command to original SB/ESS evidence or an explicitly graded reference model. The named 86Box routines are comparison anchors, not exact ES488 qualification. Resolve every modem/IR/inking/dock row with implementation proof or owner-approved disposition; successful speaker output does not close them. |
| Guest/release inputs | Firmware/qualification | Identify approved original firmware and guest media with hashes, versions and positive checkpoints before boot qualification. Existing QEMU modified disks/extracted Setup are not substitutes. Missing media blocks only its qualification, not unrelated chip research. |

For each resolution, retain source identity and section/function, inferred
versus measured status, the selected rule, rejected alternative and the direct
plus composed test that can falsify the rule. If evidence remains absent,
report the exact required capture/manual/owner decision rather than implementing
an always-ready stub. This matrix adds finite next actions; it does not claim
those actions have already succeeded.

## Final Research Reconciliation

The nine proposals are all linked in Queue and now consume the rows above.
This review distinguishes an evidence-backed work package from permission to
implement every uncertain detail. Reading a whole repository or counting green
documentation checks is not a substitute for resolving the specific gates.

| Receiver | Evidence and checklist present | Still required before affected implementation/qualification |
| --- | --- | --- |
| 486 instructions | Original PRM delta plus six-family form matrix, source conflicts and real Core decoder/profile audit. | Resolve selected optional feature identity and the XADD exception discrepancy; expand inherited-form coverage using current CPU tests at admission. Do not silently advertise unimplemented system effects. |
| 486 system | Original developer/PRM reset, system and timing anchors; CPU-2--9 handoffs. | Select precise silicon applicability, CPUID mask/SMM behavior and per-form timing grades, with no 386 timing fallback claimed as 486. |
| Board | Original chip references where available; pinned reconstructed configuration/ROM/net evidence; BRD/SIO rows and existing board reuse. | Freeze original board/module population, private register/decode rules, SMSC revision conflict and firmware/reset aliases. Known post-POST dumps cannot fill unknown power-on defaults. |
| Input/power | Real firmware-vector inspection, Pluto/MCU/Trackpoint studies, existing KBC/mouse code, IN/PM rows. | Choose evidenced protocol model versus MCU execution; enumerate actual supported power/inking/Fn commands and reset/wake behavior. Partial MCU images do not qualify execution. |
| Display | Original C&T register authority, panel byte comparison, font reference-code audit, VID rows. | Qualify selected VRAM population and Bowman font decode/reset; admit exact panel/firmware pairing and offline preparation separately. |
| Storage | Original Intel/Ricoh/SMSC documents, actual net inspection and existing Core ATA/FDC code, PCIC/ATA/FL rows. | Resolve fitted topology/straps, flash board gates, socket/window priority and media identities. Core two-unit capability alone does not prove App wiring. |
| Audio/auxiliary | Original Yamaha/SMSC manuals, ESS recorded probe and pinned SB/OPL reference functions, AUD/AUX rows. | Freeze ESS command/timing model and endpoint scope for modem/IR/dock; decide every unsupported feature explicitly. No speaker-only success. |
| Firmware boot | Hashed candidate slots, source-confirmed bidirectional patch, concrete reset/POST/Setup/DOS milestones BOOT-1--4. | Admit complete external input set, integrity/derivation and exact guest corpus; run the real guest paths after hardware is implemented. |
| Product qualification | Actual startup/INI/factory interfaces, four fixed artifact names, QUAL-1--3 and existing App ownership review. | Complete all predecessor proof, release tests and owner acceptance; no PC110 binaries are currently produced or claimed valid. |

All nine have a named owner, source entry, finite behavior checklist and direct
plus composed proof requirement. The right column remains an explicit
admission gate, not a silent exclusion or a claim of completed hardware freeze.
The source/checklist objective is satisfied; the programme's stronger hardware-
freeze gate remains open. T558 is not closed and no implementation is admitted
by this research reconciliation.

### Facts Frozen Versus Decisions Still Gated

- Freeze now: existing Core/App ownership and retained four-App paths; one
  PC110 implementation with two build-fixed panel/firmware pairings; four
  owner-selected executable names and one runtime INI; external immutable ROM
  masters; the pinned research revisions, inspected original-manual identities
  and candidate hashes; no BIOS interception, second executor or Shared change.
- Freeze as evidence, not hardware defaults: the historical 042B capture,
  candidate's 18 TFT values, source-confirmed reverse table, reconstructed nets
  and post-POST register dumps. Preserve their source/measurement limitations.
- Block the affected implementation choice: each decision in Bounded Admission
  Decisions above. Its receiver must record the selected rule and falsifiable
  direct/composed test before implementing it. A whole receiver need not guess
  unknown behavior to start separately evidenced work, but its stated exit
  cannot pass with an unresolved required row.
- Block release qualification: unprepared or mismatched firmware pairings,
  missing approved guest media, unimplemented hardware and unexecuted guest
  checkpoints. None is counted as passed by this documentation delivery.

### S2 Requirement-To-Evidence Review

| Requirement | Inspected evidence and disposition |
| --- | --- |
| Every later task has implementation sources | Nine proposal links were read and reconciled with the source availability record and concrete contract rows. CPU, display, Ricoh/SMSC/flash and OPL have identified originals; private board/input/ESS domains have pinned observations or named reference code with explicit limitations. No repository name alone is an implementation specification. |
| Every later task has a finite checklist | The nine inventories and CPU/SIO/PCIC/ATA/FL/VID/IN/PM/BRD/AUD/AUX/BOOT/QUAL rows identify behavior, failure/reset obligations, owner and proof. Full instruction-form expansion belongs to the admitted CPU batch using the cited manual, not BIOS first-failure discovery. |
| Current NXVM architecture is actually considered | Rechecked CPU profile/placeholder entry points, preparation.c second-disk rejection, machine_devices.c first-floppy construction, board-base HDC unit mapping and Product startup interface at feadb8235. The S1 ownership audit supplies the four-App boundary findings and their receivers. No source change is claimed. |
| Sources can be located again | Rechecked all three primary external HEADs, the eight archived manual hashes, and historical flash/decompression object identities. Source record separates current checkout, Git-history objects, online tool revisions and author-reported captures. |
| Overlap and ordering do not create duplicate implementations | Cross-task handoff table assigns CPU decode/system effects, SMSC configuration, flash array/mapping, firmware preparation, inking and dock state once. Board input preparation precedes dependent display/boot qualification. |
| Exact baseline is justified or has bounded resolution | Frozen facts and evidence classes are listed above; the nine decision rows name unresolved facts, responsible receiver and concrete resolution/proof. These do not authorize undocumented constants or complete T558 hardware freeze. |
| Preservation and verification are honest | Repository change scope is NXVM documents only. No source/test/Shared/ROM/media/INI/EXE changes or runtime claims. Documentation structure, local-link review and diff checks support the documents, not PC110 execution. |

Verification on 2026-10-13: NXVM documentation governance and `git diff --check`
pass; file-target and heading-fragment checks pass for all 15 changed documents.
Actual-diff review removed duplicated research paragraphs and reconciled the
previously stale capture/decompression availability statement. The three main
reference HEADs and eight archived manual hashes still match this record.
No production inputs changed, so the existing ten EXEs remain untouched;
no build, runtime, physical-PC110, desktop or RDP qualification was performed.
