# T558 S1 — PC110 Initial Source And Gap Ledger

## Baseline And Evidence Limits

Research date: 2026-10-13. NXVM baseline: `8f00b1bce`, initially clean.
The following initial observations predate the reference refresh; the
superseding research and receiver map are recorded at the end of this ledger.
This is an initial inventory and design delivery, not a completed register,
timing, firmware or compatibility audit. No hardware experiment or PC110 build
was executed. No source, protected firmware or media was imported.

Read-only external references:

- Open-Source-PC110 at `deb665f1b31c946b918f03850813745c406f403c`:
  `Components/README.md`, `Components/U61-VL82C420FC5/README.md`,
  `PCB/Mainboard/README.md`, `datasheets/{top,bottom}/README.md` and file inventory.
  [Repository](https://github.com/ahmadexp/Open-Source-PC110).
  Component labels and photographs are hardware research leads; reconstructed
  PCB files are not IBM originals. This revision explicitly calls its
  schematics incomplete/placeholders. Its README also imposes noncommercial
  and open-source conditions: do not copy or derive implementation under MIT
  without a separate license review.
- PC110-EMU committed reference `81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c`:
  [repository](https://github.com/ahmadexp/PC110-EMU), README and
  `Sources/PC110Core/pc110_core.c`. The local checkout has modifications in four
  tracked files; initial worktree observations were checked against `git show
  HEAD:<path>` before retaining the conclusions here. Committed README says it
  repairs Personaware startup scripts in memory; source retains placeholder
  I/O and BIOS/DOS handling. It is a behavioral research lead, not an accepted
  hardware oracle, source import or proof that NXVM can boot PC110.
- Newer online reconstruction material was discovered, but not pinned or
  reconciled to this local revision. Do not mix its asserted gate-array
  responsibilities into the frozen hardware contract.

The external reconstruction contains candidate PDFs named `4-486SX-33.pdf`,
`3-VL82C420.pdf`, `6-RB5C396.pdf`, `7-28F200BX-TB.PDF`, plus standard peripheral
documents. Filename presence does not prove original authorship, completeness,
edition, exact device applicability or an accurate register map. The next
research batch must inspect originals and record edition/pages/hash externally
under the source policy; no PDF contents are claimed verified in S1.

No PC110-named asset was found by the filename scan of the owner asset tree.
This does not prove that no useful asset exists under another name. Dumps in
external reference repositories are not automatically owner-approved build
inputs. System BIOS, font data, controller firmware, optional flash and guest
media require independent role/size/hash/provenance and lawful-use review.

## Confirmed Current-Code Findings

- `src/core/chips/cpu/cpu_interface.h` exposes DEFAULT, 8086, 8088, 80186,
  80286 and 80386 only. `cpu.c` likewise has no 486 profile selection. The
  historical wish to retain 486 is not an implemented 486 product contract.
- Existing PIC/PIT/DMA/RTC/KBC/FDC/video mechanisms live in `core/chips`;
  execution lives in `core/x86`, wiring in `core/board-*`, adaptation in
  `core/machine`, and PC-family product policy in `core/product`.
- Inventory and identifier searches find no dedicated VL82C420, F65535,
  RB5C396/PCMCIA or PC110 power model in Core or the four PC Apps. Standard
  peripheral compatibility is reuse potential, not proof of PC110 register
  compatibility or wiring.
- Shared `src/x86` is product/debug/assembler tooling, not the CPU device
  implementation. The old proposal's `x86/chips`, `x86/core`, `ibmpc/board-*`
  and Common paths were stale and have been corrected.

## Frozen Initial Coverage Domains

Every domain below is currently **research/implementation pending**, not
accepted. Candidate identities are not yet a frozen motherboard revision.
Each domain must be expanded into finite register/function/reset/timing cases
before its implementation S; no domain may disappear merely because POST runs.

| Domain | Evidence and current gap | Receiver and required proof |
| --- | --- | --- |
| Board identity | Exact board revision, factory population and options not frozen | App composition; reconcile original IBM docs with observed board labels |
| CPU | 486SX-33 is a research lead; no current 486 enum/implementation contract | Core CPU; exact stepping/features/reset/exception/instruction/system/timing ledger, no renamed 386 |
| Chipset/memory | U61 VL82C420FC5 observed in component inventory; private mapping/config/refresh/power behavior unqualified | PC110-local chipset initially; reset maps, RAM sizing, ROM/shadow decode and bus behavior |
| Standard PC peripherals | Existing PIC/PIT/DMA/RTC mechanisms, actual integrated variant and routing unknown | Core mechanisms plus App wiring; register/reset/IRQ/DMA/clock equivalence proof |
| Video/LCD | F65535 research lead; current planar/EGA support is not this controller | Qualified chip mechanism plus App panel wiring; extended registers, memory/banks, text/graphics/panel timing |
| Storage/PCMCIA | RB5C396 lead; boot-media topology and onboard flash mapping unresolved | App/controller owner; socket/windows/card/reset/interrupt and real firmware sector-transfer tests |
| Keyboard | U67 M38813-family controller in board inventory; existing AT KBC is not an MCU implementation | App input/controller boundary; documented commands, scan/make/break/reset behavior or explicit approved model |
| Pointer | U75 D17137AGT identified by reconstruction | App pointer wiring; protocol and IRQ/register evidence, not assumed PS/2 identity |
| Power/status/custom logic | U6 power MCU, U21 Bowman and U35 Pluto leads; responsibilities not frozen | PC110-local owners; reset, status LCD, suspend/resume and wake/event semantics |
| Audio/serial/parallel/modem/dock | Population and required coverage still unqualified | Inventory each option; implement or obtain explicit exclusion, never return fake success |
| Firmware/configuration/media | No admitted role/hash manifest or CMOS seed model | App build slots and external assets; reset vectors, mappings, protection and persistent-state semantics |
| Product/host integration | Existing Core Machine, PC Product and Emulator route available | app-mypc110; own lowercase INI/release identity, lifecycle/debug/input/display/media and failure teardown |

Permitted dispositions: direct proof accepted; non-applicable with evidence;
owner-approved exclusion with named receiver; or pending/blocked with exact
source, asset or implementation prerequisite. T closure requires no unowned
pending domain, plus dual-width repository units and admitted real guest
integration, deployed artifacts and owner acceptance. A timeout or later BIOS
checkpoint is containment/diagnosis, not coverage of unexamined behavior.

## Bounded Follow-up Plan

1. Reconcile original manuals, exact board/CPU population and lawful asset
   roles; choose a finite reset/POST/setup/DOS corpus with observable checkpoints.
2. Qualify and implement the selected 486 contract in Core. Split instructions,
   system/reset behavior and timing if needed; retain all existing CPU regressions.
3. Implement chipset/memory and standard peripheral wiring from its complete
   affected contract, not one BIOS port at a time.
4. Implement display/LCD; separately storage/PCMCIA; separately keyboard/pointer;
   separately power/status and remaining required interfaces.
5. Compose the fifth App through the existing driver/product/host path; verify
   startup rollback, reset, pause/resume, stop and teardown.
6. Qualify the finite real-firmware/guest corpus and both host widths, deploy
   verified pairs, audit the full ledger and await acceptance.

These are batch boundaries, not preallocated S identifiers. S1 does not
authorize Shared changes or missing-hardware fallbacks. Do not block safe
manual research merely because a future release needs owner-supplied assets.

## S1 Quality And Verification Boundary

Architecture review uses the architecture-governance skill: chip state has one
owner; board assembly connects it; Core owns execution time; existing Emulator
owns host lifecycle/UI. Do not add a second runtime, a generic device framework
or BIOS/DOS interception. PC110-only logic stays App-local unless an actual
independently reusable mechanism warrants a Core owner.

Production/test/ABI/EXE changes: zero. Research-only verification checks source
pins, code interfaces, document links, queue ordering and actual documentation
diff. Existing T557 tests are historical baseline evidence, not tests rerun by
S1. Original-PDF inspection, actual PC110 ROM execution, hardware comparison,
dual-width PC110 builds and desktop/RDP qualification remain unexecuted.

Documentation governance for NXVM, changed-document local link resolution and
`git diff --check` passed. Actual-change review confirms only NXVM documents
change; two historical receiver links are repaired without rewriting their
historical findings. T557 closure reuses its unchanged accepted proofs, not a
new runtime qualification. S1's initial inventory is ready for review; the
T-level implementation and original-manual research gates remain open.

## Reference Refresh And Multi-T Receiver Map

The owner authorized updating external research checkouts, then requested a
multi-T queue plan. Verified upstream main revisions on 2026-10-13:

- Open-Source-PC110: `02ccf2e8e19b298a2cf04c5d430278eb087cc368`.
- PC110-EMU: `81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c`.
- PC110-QEMU: `bae390ebf21665ec97860d28e26ec46f1fe8f4e9`.

The owner explicitly discarded the local EMU experiments; that checkout is
now clean. The duplicate upstream checkout was removed. The hardware-reference
checkout retains an unrelated untracked RAM-module ZIP; it is not evidence.
No reference source or firmware was copied into NXVM.

Updated evidence changes the research picture, not the hardware qualification:

- `Discovery/Service-Manual/readme.md` is explicitly an unofficial reconstructed
  manual, not IBM's original. Chipset documentation still lists unresolved
  register semantics. Its repository license is now explicit CC BY-NC 4.0;
  QEMU models/tools state GPL-2.0-or-later. Neither is an MIT implementation
  import approval.
- Selected PDF opening pages were inspected: Intel Embedded Intel486 SX
  272769-004 (August 2004); C&T 65535 preliminary publication PO27.3 revision
  1.0 (1995); Ricoh controller application manual EA-028-9804. These establish
  useful leads, not complete variant/register review. The later embedded CPU
  document cannot alone freeze the PC110 BGA silicon contract.
- `Discovery/Live-Dump/README.md` reproduces CPUID bytes `2B 04 00 00`, i.e.
  `0x042B`; other text writes `0x0432B`. Resolve against original evidence before
  selecting CPU identity. This is not permission to assume CPUID on every 486.
- Latest reconstructed `0x4F` RTC-index attribution conflicts with the QEMU
  configuration-latch model. Keep both as research inputs; do not copy either
  interpretation without reproducing the relevant access/reset behavior.
- QEMU `qemu/target-i386/pc110post.c` explicitly bypasses BIOS loops by guest
  instruction address; `05-seg-helper-loose-pm.patch` relaxes segmentation;
  `scripts/run-realbios.sh` enables these and describes removing EMM386.
  EMU likewise contains segment fallbacks and guest-specific repairs. Their
  boot claims do not qualify NXVM hardware semantics.
- Current Core ATA PIO/IDENTIFY is reusable candidate functionality. Video
  snapshot capacity is 720 by 512, but the implemented mode list does not
  establish F65535 640 by 480 scanout. WBINVD, CPUID and RSM handlers currently
  take the undefined-opcode path; names alone are not 486 support.

The original domain inventory remains intact. Its complete receiver mapping is:

| Existing domain | Bounded T receiver |
| --- | --- |
| Board identity and original source/asset contract | Current T558 research/contract freeze |
| CPU | 486 instruction task, then 486 system/reset/timing task |
| Chipset/memory and standard peripherals | Board/chipset/firmware task |
| Video/LCD and font data window | F65535 display/font task |
| Storage/PCMCIA and flash device behavior | Storage/flash task |
| Keyboard, pointer, power/status and associated custom logic | Input/power task; decode/memory ownership stays in board task |
| Audio/serial/parallel/modem/dock options | Audio/remaining-peripherals task; every exclusion requires explicit owner disposition |
| Firmware/configuration/media | T558 freezes roles; board embeds/maps ROM; storage owns device access; firmware-boot proves execution |
| Product/host integration | Board establishes the sole composition; firmware-boot completes real connections; product qualification closes the claim |

The [programme proposal](../../proposals/m5-pc110-evidence-and-implementation.md)
links all nine unnumbered implementation receivers. This supersedes the earlier
plan to carry every implementation batch inside T558. T558 is still open and
does not claim the hardware baseline is already fully frozen. PC110 releases
embed owner-supplied external ROMs under the owner's explicit authorization;
raw ROMs, media and generated byte sources do not enter the source tree.

This planning revision changes only NXVM documents. Architecture-governance
review keeps chip mechanisms, App wiring, the existing Core driver and Shared
host components separate; it adds no runtime framework. No build, EXE refresh
or dynamic test is needed for unchanged executable inputs. Documentation
governance, changed-link resolution, diff whitespace and actual proposal/scope
review are the applicable verification; later implementation gates remain open.
