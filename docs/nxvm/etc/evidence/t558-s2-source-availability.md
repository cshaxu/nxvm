# T558 S2 Source Availability And Conflicts

## Inspected Baseline

NXVM feadb8235. External pinned revisions remain unchanged from S1:
Open-Source-PC110 02ccf2e8e19b298a2cf04c5d430278eb087cc368;
PC110-EMU 81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c;
PC110-QEMU bae390ebf21665ec97860d28e26ec46f1fe8f4e9.
This is the reviewed research-source record, not a fully qualified hardware
contract. The receiver checklist records the bounded unresolved decisions.

## Original Capture Availability

`git ls-tree -r --name-only HEAD Discovery/Live-Dump` in the hardware-reference
repository lists only README.md and probe_lib.py. Referenced `raw/cpuid.txt`,
port JSON, ROM-shadow dumps and ATA observations are not present in that
committed tree or the inspected checkout. The initial inspection therefore
classified these as author-reported observations. The owner's later pointer
led to local Git-history inspection: the deleted capture objects are available
at `0a7f6c8583c3f9c8c0b7f9fac58d55d57f341967` (parent of deletion e660599).
Their presence in history supersedes a blanket unavailable classification;
each historical capture still requires content and provenance review. This is
inspection of someone else's recorded measurement, not a new NXVM bench test.

The README section 11 reproduces bytes `2B 04 00 00`, which encode 0x042B;
its 0x0432B spelling is inconsistent. This establishes what the quotation says,
not that the exact fitted silicon/CPUID feature has been independently proved.
CPU instruction/system receivers must reconcile manufacturer applicability
before selecting the implemented CPU feature contract.

The historical `raw/cpuid.txt` has now been read directly (Git blob
`69379ebbf3dbbf5e47465bea082f93eae5b8b12a`). It includes the probe bytes and
DEBUG transcript: EFLAGS.ID toggle result 00200000h, GenuineIntel and leaf-1
EAX 0000042Bh. The probe stores that EAX but not full leaf-1 ECX/EDX, so this
corroborates identity/CPUID availability without supplying a complete feature
mask. DEBUG's 16-bit DX display is not the full EDX capability word.

Section 2 calls the boot CF a PCMCIA/CF path, while section 14a explicitly
corrects an earlier interpretation: internal flash on primary ATA master, CF
on primary ATA slave, external PCMCIA holding sound/SCSI. Do not freeze a
CF-through-PCIC design from the introductory summary. Section 14a remains a
reported observation until raw capture or independent board/firmware evidence
corroborates the topology. This affects board and storage receivers directly.

## Storage Topology Cross-Check Against Reconstructed Nets

At the pinned hardware-reference revision, `Discovery/Storage/readme.md`
section 1 still routes internal CF through Ricoh, contradicting the explicit
correction in `Discovery/Live-Dump/README.md` section 14a. Neither prose
statement alone freezes wiring. The following were read directly from
`PCB/Mainboard/Storage.kicad_sch` and its supplied
`Fab/PC110.kicad_pcb_netlist.ipc`, not inferred from a PCIC feature list:

- J11 is the CF connector; pin 24 belongs to IRQ14 and pin 11 to FDC_IDE7.
  IRQ14 also reaches U42 pin46 and U21 pin75; FDC_IDE7 reaches U42 pin13
  and U22 pin20. CF data/address nets use the board SD/SA bus names.
- The schematic identifies U42 as SC414281PU4 and U41 as SDLB-4. U41 is
  not itself the complete host ATA interface; do not infer ATA pins from its
  flash-package name. J11 CE1#/CE2# nets connect U42 pins37/39 respectively.
- J11 CSEL# pin28 connects to R145 pin2, with R145 pin1 at ground. The
  schematic's U42 CSEL pin23 is also grounded in the supplied netlist.
  These observations do not independently prove the reported fixed
  master/slave assignment; mode selection, fitted straps and controller-side
  routing need reconciliation with the actual selected board.

This corroborates a board ATA/IRQ14 route for internal CF, not a reason to
send every CF access through Ricoh windows. The files are reconstructed board
evidence, not an original IBM netlist or a new continuity measurement. Retain
primary-channel internal-master/CF-slave as the author-observed candidate;
verify selection and absence behavior before freezing it. No diagram/source
was imported and no card/ROM was modified.

The storage study's claim that EDD/IDENTIFY are absent because literal
comparisons/opcodes were not found is also insufficient by itself: dispatch
tables, indirect values and compressed firmware can evade such a search.
Treat its listed command sites as positive investigation anchors. Qualify
CHS/EDD/IDENTIFY support by actual decoded dispatch and guest observations,
not a whole-image negative text/byte search.

## Candidate Firmware Bytes

Read-only hashes of local PC110-EMU `Roms/` candidates on 2026-10-13:

| Candidate | Bytes | SHA-256 |
| --- | --- | --- |
| pc110_bios.bin | 262144 | 232101c88466f311bcc32fbc215a4d7569f695ce19f9c07ca67ce2aee5232312 |
| MSM538032E@SOP44.BIN | 1048576 | 9829fdb8281c12022dc3b77686044ed1a5213ab526ce4329f2841cd64171784c |
| M38223E4HP@QFP80.BIN | 16254 | 96c6e37cfa52f30b303db70c2036cbf21e6e1bb638c5eb11343ab161db3c9cc0 |
| M38813E4HP@QFP64.bin | 16255 | b29761a2fd39abd9c9419ca73b03beb6c41bc52102e2a7429d1f023f82a2a2b8 |

The system BIOS hash agrees with that checkout's BIOS_INSPECTION.md. That
record describes FRU 39H4551, 11/08/95, embedded C&T video BIOS and reset jump
F000:E05B. Its mapping interpretation still needs board proof; an image
inspection is not evidence that the machine decodes the entire image at reset.
The two MCU file lengths are not 16384. Do not silently pad/truncate them or
treat a permissive reference loader as proof of a complete silicon ROM image.
Input/power research must determine format, address origin, reserved region
and vector coverage before direct firmware execution can be selected.

Availability in a reference checkout is not approval of a release asset set.
No candidate was copied; approved master placement remains the external
nxvm-assets archive. Hashes identify research inputs, not distribution rights.

### Historical BIOS Search After Owner Pointer

The owner recalls firmware in the PC110 repositories. Search included ignored
working files, committed trees and local Git history, not only current paths.
Open-Source-PC110 deletion `80b28abdfc2d0ceda5213b8a192a1560ba27b581`
removed `Components/Flash` on 2026-08-06. Its parent's
`Components/Flash/E28F002BXT/E28F002BXT@TSOP40.BIN` has Git blob
`38e19c64b0182232a5d96d7ce67b452e17b37f5b`, identical to the current EMU
candidate. The historical README records the same SHA-256. This locates the
remembered complete dump but does not locate a distinct stock-DSTN image:
the already inspected TFT table identity applies to both names.

The deleted live video shadow is also available: at capture revision
`0a7f6c8583c3f9c8c0b7f9fac58d55d57f341967`,
`Discovery/Live-Dump/raw/vbios_C0000.bin` is 32768 bytes, Git blob
`e5eeb2085fbaedbf697dd67874648460c35e78d4`, SHA-256
`45f6eabcd0ab436877abddcab61e353a08d165bdc247fa6dfb73ad22e7b86d0e`.
Read-only inspection in memory finds all 18 TFT values again. This shadow
is neither a full system flash image nor the separately reported later stock
DSTN capture. No external files were restored, patched or imported.
Current LBA-patched images explicitly advertise modified/unvalidated firmware
and are not stock substitutes. Further stock search must identify a genuinely
different original input, not rename one of these matching TFT candidates.

## Reference-Code Suitability

PC110-EMU `Profiles/ibm_pc110.json` explicitly labels chipset, F65535 and ESS
controllers placeholders. Its 20 MiB RAM and 512 KiB VRAM are model choices,
not frozen factory facts. The Core source and firmware helpers must be audited
per capability, not adopted as a monolithic machine specification.

PC110-QEMU `qemu/hw-misc/pc110-fontrom.c` gives a concrete bank/window model:
1 MiB, 8 KiB banks, ports 1160h--1163h, DE000h window. Its comment explicitly
calls port 1161h a scratch register with unknown real function. Retain the
bank/window implementation as a comparison candidate, not an exact unknown
register contract. GPL source is reference-only; no copying is admitted.

The existing S1 exclusion of QEMU POST instruction-address patches, relaxed
segmentation and alternative SeaBIOS boot remains in force. Their successful
screenshots cannot discharge real-PC110 reset/POST proof.

## Manual Identity Findings

Read the opening text/contents of these existing external PDFs; register tables
and timing diagrams still need full relevant-page inspection. No new manual
copy or derived text file was placed in the repository.

- `datasheets/bottom/3-VL82C420.pdf` is a Microprocessor Report article,
  "VLSI Integrates 486SL Power Management", volume 7 number 9, July 12, 1993,
  not a VLSI programming datasheet. It is useful architectural corroboration
  of SCAMP IV, not authority for private register reset values. Its related
  82C144/82C146 diagram must not be copied as the PC110's actual population.
- `datasheets/top/5-FDC37C665IR.PDF` is an SMSC component document with an
  applicable part identity. Contents identify FDC registers at printed page
  20, commands 44/47, UART 75, infrared 89, parallel 90, power 111, IDE 116,
  configuration 120 and timing 137. These are concrete receiver reading
  anchors, not a claim those sections have all been verified yet.
- `datasheets/bottom/6-RB5C396.pdf` is Ricoh application manual EA-028-9804,
  covering RF5C296/RF5C396L/RB5C396/RF5C396. The notice page says June 1995;
  record both rather than deriving an edition date solely from the number.
  Contents place address mapping/power at printed pages 18--21, indexed access
  22, multiple sockets 23, PCMCIA-ATA 24, DMA 25 and register descriptions 27.
  The shared manual's chip-specific columns must be checked for RB5C396.

Read-only SHA-256 identities in the same order:

- VL82C420 article: `435abef5d813811a3ec09151d305b5f417443d30af7c6379b92302e707ffda59`.
- SMSC manual: `2105112cd5ee41e377bf667d408796573f244180a4f3e47b9b7c714a07be3258`.
- Ricoh manual: `7eb037b495d7ed3f4334a1cdc6c6582c8fa9ceadf0fd680ebadec2575b60c7ec`.

### SMSC Configuration Table And Reset Audit

The identical SMSC manual is archived externally as
`nxvm-assets/manuals-nxvm/smsc-fdc37c665ir.pdf`. The existing Ricoh, C&T and
Yamaha originals are also archived there as `ricoh-rb5c396-ea-028-9804.pdf`,
`ct-f65535-register-reference-1994.pdf` and
`yamaha-ym3812-lsi-2138123-1992.pdf`; their recorded hashes identify the
copies, not newly acquired editions.

Printed page 122 was rendered and visually inspected on 2026-10-13. Table 48
really lists CRE default 01h while the same row's DB7--DB0 cells encode 82h.
This is not OCR column displacement. The inspected following register tables
do not resolve that discrepancy. In the pinned 86Box reference,
`src/sio/sio_fdc37c6xx.c:fdc37c6xx_reset()` assigns 02h for chip IDs >=65h
and 01h for the earlier modeled parts. That is a third reference-model value,
not evidence to choose 02h for the PC110's 665IR revision. Freeze CRD/CRE
identity from applicable silicon documentation or a read-only actual-unit
configuration capture before claiming exact identity/reset qualification.

Page 122 says configuration defaults are established at power-up and are not
affected by RESET. However page 124's CR1 bit7 description specifically says
the configuration lock, once cleared, can be set by hard reset or power-up.
Do not generalize the former sentence to an unconditional preserve-all reset
implementation, or copy a reference initialization routine as ordinary-reset
semantics. The board/SIO contract must distinguish power initialization,
ordinary reset and this documented lock exception; test locked access before
and after the corresponding reset. This finding narrows SIO-3 without adding
a new reset controller or inventing a reset value for CRE.

## Later-Chapter Reconciliation

The missing `Live-Dump/raw` files do not mean every capture is missing.
`Discovery/Chipset/{scamp_config,block2_config,eced_config}.txt` are present,
as is `block2dump.asm`. The EC/ED text explicitly identifies chipset config,
not a power-MCU mailbox. `Discovery/Chipset/readme.md` sections 13j and 13k
supersede several earlier claims. Section 13k and
`Discovery/RAM-Module/eced-dram-regs-live.md` report module-dependent EC/ED
02h/03h values; the older claim that these cannot be geometry is withdrawn.
These are inspectable reference observations, not measurements made by NXVM.

The two emulator models still implement EC/ED as MCU state. QEMU's
`mcu_read/mcu_write` and EMU's firmware metadata map are therefore unsuitable
as the authority for this port pair. They also cannot establish the real power
controller interface. The board and input/power tasks must reconcile the
actual BIOS accesses and newer capture context before assigning port owners.

Port 4Fh has at least three conflicting descriptions: extended RTC index in
the service reconstruction, configuration latch in QEMU, and I/O delay in
chipset section 13j.8. This is a specific board research item, not an excuse
to combine all three behaviors. The reference supplies BIOS offsets for
independent analysis; the current evidence does not settle arbitrary reads.

`Sources/PC110Firmware/POWER_MCU_FIRMWARE.md` says the status LCD is modeled,
not obtained by executing MCU firmware; high indexed values expose firmware
banner/checksum metadata. `KBD_MCU_FIRMWARE.md` likewise describes a compatibility
model. Their smoke tests prove that model, not real MCU instructions or protocol.

Audio has a useful later probe: `Software/ESSPROBE/ESSPROBE.ASM` with its README
reports DSP reset/version/ESS ID and mixer reads. It is a concrete reference
probe for SB2-class ES488 behavior, not a complete audio datasheet. Pointer
research `Discovery/Trackpoint/readme.md` final live-host section reports PS/2
ID/status responses, while the actual mask ROM remains unavailable. Model the
documented external protocol only after reviewing this distinction; do not
claim a recovered MCU.

## Acquired Intel Originals

The actual local archive is `nxvm-assets/manuals-nxvm/cpu`. On 2026-10-13,
two originals were acquired there from the Bitsavers Intel 80486 collection;
no manual or extracted text was added to this repository:

- `intel-486-family-programmers-reference-1992.pdf`, 44,868,764 bytes,
  SHA-256 `d86207aaebd9b10464f2058efc61559e6141d3b072db100e89bfe17c8187e143`;
  source `https://bitsavers.trailing-edge.com/components/intel/80486/1992_Intel_486_Programmers_Reference_Manual.pdf`.
  The scan has later literature-order front matter, so the URL's year is not
  independent edition proof. Inspected contents identify instruction chapter
  26: BSWAP 26-35, CMPXCHG 26-62, INVD 26-171, WBINVD 26-281 and XADD
  26-282; alignment check is section 9.9.16. These anchors select required
  reading, not yet full opcode/timing acceptance.
- `intel-486-embedded-developers-273021-001-1997.pdf`, Intel order
  273021-001, October 1997; SHA-256
  `be6242fd0a0b0663c27aa578c6bb7a3994db21f1ec31887c2ea440d011b82bd3`;
  source `https://bitsavers.trailing-edge.com/components/intel/80486/273021-001_Embedded_Intel486_Processor_Family_Developers_Manual_199710.pdf`.
  Inspected contents locate SMM in chapter 8, hardware/reset in chapter 9,
  bus operation in chapter 10 and feature determination in appendix D.

Appendix D, printed D-3, table D-2 explicitly assigns family 4/model 2 to
Intel486 SX. It separately lists model 4 as Intel486 SL and marks that part
as not implementing CPUID. Thus the quoted 0x042B is consistent with SX,
not evidence to select the distinct SL processor merely because the machine
has power-management features. This does not independently validate the
reported capture or identify package, stepping errata or feature bits.
The manual also requires CPUID qualification for CR4 VME/PVI/PSE features:
do not enable every feature described by this multi-variant manual.

## Independent Firmware Access Check

Read-only 16-bit disassembly of the hashed BIOS candidate confirms the
reference offsets: file 3DBDEh selects AL at port ECh and reads AL at EDh;
3DBEBh selects AL at ECh and writes AH at EDh. The following pair uses
3F0h/3F1h. This confirms executable indexed-access routines in the actual
candidate, not merely a reference emulator's port declaration.
It does not establish EC/ED register meanings, unlock behavior, reset values
or whether a particular caller executes on a specific board revision.
Call-site and RAM-sizing analysis remain necessary; no firmware bytes or
reconstructed source have been imported.

## Additional Source Review

### Audio And Chipset Reference Limits

An additional read-only behavior reference was inspected:
`86box-source` at `4fef696a4eead1d55a28d6ac0e5bd2864e5454da`, with clean
tracked/untracked status at inspection. The project COPYING is GPLv2; no
source import or transliteration is approved.

- `src/sound/snd_sb_dsp.c::sb_exec_command()` has concrete single-cycle
  8-bit DMA (14h), auto-init (1Ch), time constant (40h), block size (48h),
  pause/continue (D0h/D4h), version (E1h) and ESS query (E7h) branches;
  `sb_dsp_reset()` is the reset entry. These are useful behavioral comparison
  points for a separately specified SB-compatible DSP. No ES488-specific
  model was found by the inspected symbol search; later ESS extensions must
  not be assumed to apply. The file also labels some decoding tables as
  extrapolation/guesswork, so it is not an original timing authority.
- `src/sound/ymfm/ymfm_opl.cpp` and the matching headers provide a concrete
  OPL synthesis reference; that vendored file has a BSD-3-Clause notice,
  distinct from the surrounding project's GPL. It remains reference-only
  in this task; any later import requires a separate unit-level license and
  architecture review rather than silently bringing in C++ or a new runtime.
- `src/cpu/cpu_table.c` distinguishes i486SX from i486SX-S (SL-enhanced),
  with a 33-MHz enhanced model reporting 042Ah. `cpu.c::cpu_CPUID()` returns
  VME for that model, and its family setup enables CR4/VME. This provides an
  explicit implementation comparison for CPU-7, not proof that the PC110's
  reported 042Bh exposes the same bits. A standard 486SL processor and an
  SL-enhanced 486SX must not be conflated.

The scanned Yamaha original `datasheets/top/7-YM3812.pdf` was visually
inspected on pages 1--6. Cover identity: YM3812 Catalog, LSI-2138123,
April 1992. SHA-256:
`35c4a73c59a480419305644482bfe7edecedd016d9e92301c2c2e7fe97f4ae49`.
There is no extracted text layer. Page 3 defines address/data/status access,
two timer IRQs and reset; pages 4--5 identify operator/envelope/rhythm and
register groups; page 6 supplies electrical timing including reset duration.
This is useful direct register/timing authority, not by itself a bit-exact
digital synthesis specification. Detailed synthesis needs a separately reviewed
reference implementation or further original material.

ESSPROBE's actual assembly distinguishes reset at base+6, response data at
base+Ah and status at base+Eh. Its README reports AA, version 02.01 and ESS
ID 48 82 on the author's machine. By contrast, PC110-EMU `ess_read()` returns
AA at 22Eh and `ess_write()` only traces; these are explicitly placeholders.
Neither that code nor QEMU's speaker-only route can qualify ES488 PCM/DMA.
The reconstruction's broad SB-Pro wording must not override the later
SB2-class observation. All-FF mixer reads alone also do not prove every
possible chip mode lacks mixer hardware.

`Components/U61-VL82C420FC5/VL82C420_Technical_Reference.pdf` explicitly calls
itself reconstructed. Its patent-derived multiplexed-bus diagrams describe a
related architecture, not a manufacturer PC110 register manual. Companion
VL82C144/146 and FC4 package assumptions cannot be imported into the fitted
FC5/Ricoh/Bowman board without independent evidence. The original patent is
available for bus research, but does not settle private register reset values.

### Display And MCU Follow-up

The C&T file `datasheets/bottom/2-CHIPS65535.pdf` has SHA-256
`acb95e9bd56786227b6b9201b3bff317adbd224d1a587fbf8db515fb5a648a91`.
It combines a July 1995 PO27.3 Revision 1.0 introductory sheet with the March
1994 register manual marked Revision 2.0. Register citations must use the
latter identity, not the opening sheet's revision. Inspected printed p16
states extension registers are always accessible at 3D6h/3D7h; XR15 protects
groups of standard VGA registers, not a guessed global extension unlock.
Printed p91 specifies those groups and reset-unprotected behavior. Printed
p74 ties attribute blinking to VSYNC and XR60, and defines ninth-dot/panning
behavior. These must use the existing guest video timeline, not host timers.

The reconstruction's `Discovery/65535/readme.md` is about a recreated board
and includes a larger fitted DRAM and conflicting MCU role descriptions.
Its component population is not sufficient to select original PC110 VRAM.
Original controller documentation describes 512-KiB/1-MiB configurations.
The same reconstruction's section 6 explicitly distinguishes the original
512-KiB population from its recreated 2-MiB part. This supports a stock
512-KiB design candidate, not a manufacturer-verified population or permission
to inherit the recreation's larger memory. Confirm the selected original board
and controller configuration before freezing alias/bank tests.

### Candidate BIOS Panel Identity: Independent Byte Check

The pinned `Discovery/65535/readme.md` section 6c.1 reports 18 distinguishing
video-BIOS offsets for a stock DSTN unit versus a TFT conversion. It explicitly
limits the stock capture to one unit; a different BIOS revision can be unknown.
On 2026-10-13, a read-only check of the independently hashed EMU candidate
`pc110_bios.bin` (SHA-256
`232101c88466f311bcc32fbc215a4d7569f695ce19f9c07ca67ce2aee5232312`)
compared those locations at flash offset `0x20000 + video_offset`.
All 18 match the documented TFT column; none match its DSTN column.
The sampled offsets are 01CB, 0A9A, 0A9C, 0AA0, 0AA4, 0AA6, 0AB4, 0AB8,
0AE6, 0AE8, 0AEA, 0AF6, 0AF7, 0AFA, 0AFC, 0AFE, 0B00 and 0B04.

This proves the candidate's table identity against that reference, not that
the author's missing live capture has been independently reproduced. It is
not a qualified stock-DSTN release input. Do not pair this ROM with stock
panel timing and attribute resulting differences to the F65535 model.
The owner subsequently selected both original DSTN and upgraded TFT as
separate compiled models over the same PC110 implementation. The briefly
proposed INI model/firmware selection was superseded; all ROMs are build-embedded.
An unchanged original dump and a documented panel-restored image are different
provenance classes. The latter is now a concrete preparation candidate, as
confirmed below; absence of a separate original dump is not absence of a
restoration route. Neither class is qualified by table matching alone.
No ROM bytes were changed or imported during this check.

### Bidirectional Panel Patch: Source Confirmed

The owner's pointer was verified against actual source, not just menu text.
At the pinned Open-Source-PC110 revision, `Mods/BIOS-Multi-Patcher/PCPATCH.ASM`
has `.pre_dstn` load the machine's `PANEL.SAV`, falling back to `dstn_val` if
unavailable. `.dstn` and `.tft` both call `write18` with the corresponding
18-byte set and the same offset table. The stock table is attributed to a
2026-07-27 unmodified-unit capture in Discovery/65535 section 6c.1. This
supersedes Mods/README's stale statement that the reverse patch is pending.
It does not establish that all other bytes in an arbitrary input are stock.

Read-only GitHub inspection on 2026-10-13 also pinned the separate tools:

- [PS2TUI d9fb80c](https://github.com/ahmadexp/PS2TUI/tree/d9fb80cbb17240bcee04bd77bff56681509756be):
  `PS2TUI.ASM` implements both panel directions. Its reverse path requires
  `C:\\PANEL.SAV`; the stock table is used by diagnosis, not an automatic
  restore fallback. Do not attribute Multi-Patcher's fallback to this version.
- [PS2GUI 039cc717](https://github.com/ahmadexp/PS2GUI/tree/039cc7172750d8bf6ed21a8e8a1d29a64883217f):
  README and `FLASH-ISSUE-1.md` explicitly disable all five flash actions.
  The report identifies unbounded polling, unowned staging, omitted boundary
  bytes and incomplete validation in the older common flasher. Diagnosis
  remains available. These are author-reported/reproduced host-test findings,
  not a new NXVM hardware experiment; no flasher was run here.

The board receiver's asset-binding batch should evaluate offline panel restoration independently
of these unsafe in-system flash paths. Before adopting a derived input:

1. Pin the complete source image and patch table revision; reject unexpected
   size, mixed panel values or an unreviewed firmware revision.
2. Confirm all 18 input sites, their surrounding register-write tables and
   option-ROM integrity rules. An inverse table alone is not boot proof.
3. Verify the only differences are the declared panel/linear-start bytes,
   preserve every other byte, and prove reverse-then-forward byte equality.
4. Record original and derived hashes and derivation provenance externally;
   label the result panel-restored, not a pristine factory dump. Obtain the
   task's explicit preparation admission before creating a release input.
5. Qualify reset, POST, display modes and guest boot for each compiled pairing
   through ordinary emulated hardware, without executing or importing the
   third-party flasher or adding a runtime patch path.

The 18-site set includes offset 01CB, the separate linear-start/256-colour
change; it is not solely LCD timing. PS2GUI's report raises an aperture/RAM
overlap hypothesis for expanded RAM. The display/board receiver must review
XR08/XR0B decode against its selected RAM population rather than assume a
panel conversion also qualifies a 32-MB memory configuration.

A read-only arithmetic check adds an explicit integrity question for that
receiver. The candidate's video header at flash 20000h declares 36352 bytes.
That raw span sums to 75h modulo 256 before any change; replacing only the
18 table values would change the sum by 99h, yielding 0Eh. The reference
`write18` only writes table sites; no checksum update was found on this path.
This is not evidence that the currently booted image is invalid: the actual
firmware's copy/initialization/validation path has not yet been established,
and a raw embedded span need not be the final checked shadow. Do not silently
"fix" a checksum byte or claim the restored candidate boot-qualified. Inspect
the real firmware path during preparation and preserve this baseline. This
probe computed sums only; it created no patched image and ran no guest code.

Inspection of QEMU `fontrom_window_read()` shows it ignores the stored
`enable` field. It also masks bank/address on every read and has no device
reset callback. These are reference-model limitations, not hardware proof.
The font receiver must establish enable/reset/bank-boundary semantics from
observations and firmware accesses instead of reproducing those omissions.

Read-only inspection of the keyboard candidate's final bytes, aligned to
C081h--FFFFh, yields FFFCh=C208h and FFFEh=E49Eh, agreeing with
`Discovery/Pluto/readme.md` section 6c. This materially supports that mapping,
but does not prove all MCU peripheral behavior. The power candidate differs:
PSU-MB-M38 section 2.2 proposes base C000h and missing final 130 bytes;
blind top alignment would instead use C082h. Neither file length nor the
keyboard's valid mapping resolves the power image. Direct power-MCU execution
requires independent address/vector evidence; do not invent padding/vectors.

Pluto still contains an older EC/ED power-telemetry interpretation contradicted
by later Chipset 13j/13k. Its 15E8h/15ECh command mailbox and 35EAh/35EBh
indexed-bank observations are separate evidence. Do not create two power
routes to accommodate contradictory documentation. Keyboard matrix and Fn
behavior also retain explicit unresolved items in the reconstruction.

## Flash Evidence Availability And Limits

Read `Discovery/BIOS-Flash/readme.md` sections 7.1--9 and the opening of its
actual `vpatch.asm`. The latter identifies a 2021 TFT updater, not factory
firmware. No updater was executed. Its code is reference-only.
The document's links to `Components/Flash/E28F002BXT` and
`region0_decompressed.bin` do not resolve in the pinned hardware checkout,
but the parent of deletion `80b28abdfc2d0ceda5213b8a192a1560ba27b581`
retains them. Under `Components/Flash/E28F002BXT/BIOS Disassembly/Region 0/`,
both `region0_decompressed.bin` and `region0_lzw_out.bin` identify the same
143,335-byte blob `5da6f6730bba1bab3d6b6022427164d1b26bd230`.
The accompanying analysis is blob `d9acde6d8f155e671701fff56405bda7b3f52c1b`;
the offline unpacker is `b9342a1aaace11ee323217018d085bed09df753e`.
Thus the evidence is historically available, not missing. None was restored
or imported into NXVM.

Read-only inspection of that unpacker reveals a qualification limit: exhausted
input returns the same value as an explicit LZW END code; an invalid code
breaks the loop; and the output budget ends the loop. All three can return a
partial byte string without a distinct failure result. Its reported output
length or UI strings therefore do not independently prove complete decoding.
The analysis identifies a loader at flash offset 4Ah, relocation to 9000:0,
decoder entry A3h and stream start 2BFh. Treat these as reference checkpoints
to corroborate against the actual firmware bytes and guest execution, not host
injection addresses. Boot qualification must observe real loader execution,
source-bank transitions, termination and destination writes before comparing
the historically supplied output. No offline unpacker is a runtime substitute.

Section 7.2 explicitly leaves two board gates unresolved: U59 VPP and
Pluto_BIOS_WR_EN. Failed read-ID/program observations do not distinguish them.
Do not implement a boot-drive-dependent magic enable or assert a successful
program operation from the updater's writes alone. The fitted part is reported
as Intel 28F002BXT. The existing `7-28F200BX-TB.PDF` has now been inspected:
its title explicitly covers both 28F200BX-T/B and 28F002BX-T/B, with section 3
for the x8-only part. It is Intel order 290448-005, November 1995, SHA-256
`2cbda8e8a856ee0d94c803bfceee92301f00b3a1caf7f74be472dbd1fe082089`.
The unchanged manual was archived on 2026-10-13 as
`nxvm-assets/manuals-nxvm/intel-28f002bx-290448-005-1995.pdf`.
Original provenance is the pinned hardware repository's datasheet copy;
the title, part coverage and order number were read directly, not inferred
from its filename.

Printed p15, section 4.0 (also visually inspected), contradicts the study's
section 7.1 assertion that low VPP inhibits all command writes: without high
programming voltage the chip still supports Read Array, Read/Clear Status and
Intelligent Identifier. Printed p18 specifies the x8 identity (89h manufacturer,
7Ch top-boot device) and distinguishes VPP programming requirements from RP#
boot-block protection. Thus a read-ID returning array data is not explained
by low VPP alone under the documented chip contract. Board decode, WE#/RP#
and actual selected part still need independent evidence; do not claim which
one caused the reported observation.

Printed pp18--19 define command phases and read-state selection; p21 describes
sticky error bits, operation-triggered VPP detection and programming that only
clears bits. The chip's command behavior is sufficiently sourced for a finite
implementation checklist. It does not resolve PC110's physical write gate.

Section 9 distinguishes physical flash banks from post-boot CPU windows.
Its runtime sweeps cannot prove the absence of an early-boot decode, nor can
the chip dump itself prove that decode. Board work must inspect the real guest
loader's accesses and bank-select nets. Never pre-decompress Setup into host
RAM as a replacement for guest execution. Flash programming qualification
must use disposable emulated state, never external firmware masters.

## CPU Instruction Evidence Precision

### SX Feature-Mask Candidate From An Original Datasheet

The existing reference file `datasheets/bottom/4-486SX-33.pdf` is Intel
order 272769-004, August 2004, Embedded Intel486 SX Processor. An identical
copy was archived on 2026-10-13 as
`nxvm-assets/manuals-nxvm/cpu/intel-486sx-272769-004-2004.pdf`, SHA-256
`a35a0a428168ed45428dd886876238424a228b8db0583ee74a1815a88bf421cd`.
Sections 4.1--4.2, printed pp23--24, give an actual feature contract rather
than merely saying "486":

- CPUID 0F A2: leaf 0 returns maximum leaf 1 and GenuineIntel; nominal
  clocks are 9 for leaf 0 and 14 for leaf 1. Inputs above 1 are undefined,
  not a specified modern zero-filled extended-leaf interface.
- Leaf 1 has family 4/model 2, stepping-dependent low nibble. EDX is
  00000002h: VME present, FPU absent, all bits 31--2 zero. EBX/ECX are
  reserved, not a documented topology or cache-information interface.
- ID starts clear on RESET/SRESET and is software-toggleable. Reset EDX
  contains the family/model/stepping signature, not the CPUID feature mask.

The p24 table was visually checked. This is consistent with the historical
PC110 capture's EAX 042Bh and displayed low DX 0002h, but the datasheet covers
later 5-V PGA/PQFP packages, not direct proof for the PC110's RIOS-marked BGA.
It provides a defensible candidate to reconcile, not permission to advertise
all Intel486 embedded-family features. In particular, do not infer PSE or an
FPU from the family manual, confuse CPUID.VME with the position of CR4.VME,
or use a 486SL model-4 identity. Remaining silicon applicability and SMM
details stay in the CPU feature gate; no exact-PC110 claim is made here.

CPU instruction/system tests should now explicitly separate CPUID leaf output,
reset signature, reset ID bit and reserved/unsupported leaves, plus actual
VME execution support if this contract is selected. Reporting VME while only
implementing its feature bit would fail CPU-system closure.

### Base Instruction Source Conflicts

The archived Intel 1992 family PRM was inspected at printed 26-35,
26-62--63, 26-171--172 and 26-281--282 for the six base 486 additions.
BSWAP and XADD pages were rendered to verify suspect extracted text.
BSWAP's printed table says `0F C8/r`, not the OCR's `CB/r`; pinned 86Box
`src/cpu/386_ops.h` independently dispatches C8--CF to eight registers.
Do not interpret this old table notation as an extra ModR/M byte. The same
page explicitly leaves a 16-bit result undefined, not specified as #UD.

XADD's printed exception paragraph really includes #NM on CR0.EM/TS; this is
not OCR invention. It is a suspect integer-instruction exception and requires
errata/later Intel specification or selected-silicon corroboration before it
becomes a requirement. The pinned 86Box `x86_ops_atomic.h:opXADD_b_a16()`
contains no explicit FPU availability check; this is comparison evidence, not
manufacturer authority. Keep this discrepancy visible rather than deriving a
new floating-point dependency from the isolated sentence.

The same manual's section 9.9.7, printed p9-16 (PDF page 236), was rendered
and inspected separately. It defines #NM for ESC with EM, or ESC/WAIT with
TS (WAIT also requires MP), not integer XADD. This is an internal original-
source contradiction, not merely disagreement with 86Box. The defensible
implementation proposal is to keep XADD independent of EM/TS and test all
four bit combinations, while separately retaining the SX ESC/WAIT rules.
Record that resolution as an explicit source-conflict decision at instruction
admission, not an alleged published erratum or a new selected-silicon test.
The nearby general advice to set MP on a 486 also needs the SX-specific
qualification in section 19.2.3; do not generalize DX FPU policy to SX.

The same reference's `opCMPXCHG_b_a16()` charges memory forms 10 clocks for
both outcomes, whereas the original table distinguishes 7 for equal and 10
for unequal (register 6). Use the original timing assumptions and branch
conditions in the NXVM ledger, not a copied emulator constant. Its XADD
handler also commits destination then source, so alias cases must be checked
independently against the instruction's ordered semantics. No third-party
handler was copied, compiled or declared a complete CPU oracle.

The companion receiver checklist now contains a finite acceptance matrix for
these forms, including widths, address modes, aliasing, privilege, unchanged
flags, old-family rejection, fault atomicity and nominal timing. Optional
CPUID/SMM and full inherited-386 behavior retain their separate source gates.

## Research Delivery Boundary

The reviewed sources, conflicts and original-manual identities above feed all
nine [receiver checklists](t558-s2-receiver-checklists.md). Each receiver has
specific source sections/functions, an implementation owner and direct plus
composed proof requirements. The final reconciliation there distinguishes
established facts from precisely bounded admission prerequisites; it does not
claim every register, timing value or firmware input already qualified.

In particular, optional CPU feature applicability, private chipset reset/decode,
SMSC revision, MCU execution versus protocol modeling, selected video memory,
font decode, storage straps, ESS model and guest-media identity remain visible
decisions. The relevant implementation batch must resolve its decision using
the cited source/capture path or seek an explicitly graded model approval.
It may not erase it through a passing boot screenshot, successful stub or
unreported product exclusion. T558 hardware-freeze acceptance is separate
from this source/checklist research delivery.
