# T555 S4 Storage Source Ledger

## Baseline And Method

Admission baseline is clean `476f9fea5`, with accepted runnable S3
`2ad955473`. This record consumes the complete S1 storage batch. Historical
T507/T510/T511 findings are leads only; original source, current owner and
receiving tests must be reconciled before accepting a row. No production
repair or runtime qualification is claimed by this initial inventory.

The actual external archive is `nxvm-assets/manuals-nxvm`, not the historical
`manuals` path. Originals are read-only and are not copied into this repository.
On 2026-10-11 the following original SHA-256 values were recomputed:

| Original | SHA-256 |
| --- | --- |
| Intel 8272A, 1982 | C03A1FABE42FE43BB47FBD84042840F7B78B1DDD03E8A117FC7059C3453CC398 |
| ATA-3 revision 7b, January 1997 | FC9D1C42B0B3EF916C790BAFED05E96D9EBA3B221C0E0D3E914E6F948DDD0237 |
| IBM 5170 Technical Reference, March 1984 | F6CB4A270E0F49A79940FA1EE1084CA419AF15B463C870D652EA74CED011EA0D |
| IBM Options/Adapters volume 2, April 1984 | B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F |
| Compaq DeskPro 386 Technical Reference volume 2, September 1986 | E523F1ACF81B7223D94835FB9BEDDEF0738BA6B1C5E1C740028FDCF3DA79357F |

## Frozen Coverage Batch

Every row remains unaccepted until its original pages, complete current
implementation and actual direct/board receiver have been checked. A row may
then be directly proven, repaired and proven, explicitly excluded by the
selected logical-media contract, or transferred with its complete named
receiver. Confirmed in-scope defects must be repaired; a boot checkpoint
does not dispose them. Timing models never become original-source values
merely because a controller accepts configured ticks.

| Row | Coverage and required contexts | Current owner / receiver family | Initial disposition |
| --- | --- | --- | --- |
| F01 | All fifteen command grammars, lengths, invalid/reserved flags and result lengths | chips/fdc8272; contract/records | Pending original/current reconciliation |
| F02 | Specify SRT/HLT/HUT, DMA selection and selected clock conversion | chip plus board-base/fdc | Pending |
| F03 | Sense Drive Status sampled pins, head/unit and result | chip; contract/causes | Pending |
| F04 | Seek/Recalibrate overlap, PCN, 77-step limit, SIS and ready causes | chip; causes | Pending |
| F05 | Read/Write and deleted-data commands, MT/SK/EOT/TC and status | chip; records | Pending |
| F06 | Read Track selected drive, starting ID, full-track limits, NDMA and marks | chip; records | Candidate restrictions require original/media-boundary proof |
| F07 | Read ID, unavailable IDs and actual track/head | chip; records | Pending |
| F08 | Format IDs/fill/partial input/write protection and failure | chip/media; records | Pending |
| F09 | Scan Equal/Low/High, STP, comparisons, deleted marks and termination | chip; records | Pending |
| F10 | MSR/RQM/DIO/CB/NDMA, byte cadence, overrun, DMA/DACK/TC | chip; contract/causes and board DMA | Pending |
| F11 | Reset/service gate/result acknowledgement and ready-poll ordering | chip/board; causes/topology | Pending |
| F12 | Four-App ports, DOR, change line, DMA2, IRQ6, media geometry/errors, deadlines | board-base/fdc, Machine providers, App fixed inputs | Pending |
| H01 | ATA PIO read/write/identify, count zero, CHS/LBA28, error/result phases | chips/hdc; taskfile/contract | Pending |
| H02 | ATA status/alternate-status IRQ acknowledgement, nIEN, SRST and write admission | chip/board-base/hdc; taskfile/board | Pending |
| H03 | IBM WD1003 command variants, restore/seek/verify/format/parameters/diagnostics | chip; contract/taskfile | Candidate simplified command execution requires proof |
| H04 | IBM head extension, ports, reset, IRQ14 and selected mechanical/timing bounds | chip/board/App; board HDC | Pending |
| H05 | Compaq WD command grammar, two-drive selection, verify/parameters/diagnostics | chip; Compaq board and dual-drive | Pending |
| H06 | Compaq shared FDC port, control/status acknowledgement and timing | chip/board/App; topology/Compaq | Pending |
| H07 | Xebec DCB/initialize/response/sense grammar, range/count and errors | chip; contract | Pending |
| H08 | Xebec selection/reset/mask, DMA3/TC, IRQ5 and pending service | chip/board/App; contract/board | Pending |
| M01 | Sole media registry/provider, direct/readonly/overlay, replacement/removal errors | board-base/media and machine/media; provider/close-failure | Pending; Lib implementation read-only |
| T01 | All four HDC and FDC timing inputs, overflow, due/advance and scheduler consumers | chip/board/App existing owners | Pending; no model upgraded without source |

## Initial Current-Code Findings

Read FDC command-length, dispatch and transfer admission, and HDC command
classification/dispatch at the baseline. FDC Read Track currently requires
opcode 42h, DMA, unit zero, sector one, 512-byte sectors and EOT equal to the
track size. This is an observed restriction, not yet a confirmed repair.
HDC verify loads a sector before completing, and initialize parameters
completes without a separate geometry update. Their required semantics must
be checked against each selected personality, not inferred from ATA alone.

No source/test/asset change or dynamic test was performed in this inventory.
The S4 packet is active; full original-page and current-path audit remains.

## Original-Page Reconciliation, First Pass

On 2026-10-11 visually reviewed Intel PDF pages 9/11 (printed 6-232/6-234)
and Compaq volume 2 PDF pages 126/129 (printed 9-11/9-14). Renderings remain
ignored under `build/t555-manual-review/` for immediate S4 work. This review
does not copy source from another emulator or claim dynamic reproduction.

### Confirmed Missing Functions Requiring S4 Repair

- **F02:** Intel 6-234 specifies HLT between head load and read/write and HUT
  between execution end and unload, with clock-scaled durations. Current
  chip stores `hlt/hut` only; complete FDC-source scan shows no execution of
  either timer. Board-Base FDC and Machine media have no timer substitute.
  Existing causes test checks preservation of those values, not their effect.
  Exact zero encodings and absent electrical head-load output still require
  source reconciliation; they must not be invented during repair.
- **F10:** Intel 6-227/6-234 requires an interrupt for each non-DMA byte;
  read/write access acknowledges it. Current `publish_due_ndma_byte` only
  clears its gate, initial execution only asserts DMA requests, and non-DMA
  accesses have no IRQ acknowledgement. The logical byte gate cannot be
  accepted as a complete IRQ protocol. Current source also contains no
  overrun deadline/status path. Intel 6-231/6-233 specifies 27 microseconds
  FM / 13 microseconds MFM for Read/Scan; 6-232 specifies 31/15 for Write.
  Cadence, operation-specific service limit and selected drive data rate are
  distinct facts and must not be conflated in repair.
- **F06:** Intel 6-232 starts at the index and reads EOT sectors, without
  multi-track or skip. Current admission requires drive zero, DMA and EOT
  equal to the entire track. Neither drive-zero nor DMA-only is justified by
  the command contract. Logical 512-byte and absent rotation/CRC boundaries
  remain separate; relaxing those model boundaries is not implied.
- **H05:** Compaq 9-11 requires per-drive programmed maximum head and sectors
  per track after Initialize Drive Parameters, defaulting to four heads and
  seventeen sectors. Current 91h dispatch just completes; CHS resolution and
  advancement always query backing geometry. No programmed per-drive
  translation is used. Existing command contract test checks completion only.
- **H05:** Compaq 9-14 defines Verify as Read Sectors without host payload.
  Current Verify loads one sector and completes irrespective of sector count.
  Multi-sector error and last-address behavior therefore remain unimplemented.
  IBM WD1003 must be checked separately before sharing this repair.

### Additional Candidates, Not Yet Final Dispositions

- Intel Format result describes R+1, while current formatting records the last
  supplied R. Current completion and TC rules need complete table comparison.
- Intel reset retains the last Specify command. Current reset preserves
  SRT/HLT/HUT but clears ND with the remaining data. Verify original table and
  selected board contract before concluding whether the DMA selection is
  also required to persist.
- Current Read/Write bounded transfer, MT progression and normal EOT status
  need comparison with Intel Tables 7/8; existing end-of-command tests cannot
  be used as an independent source oracle.
- Compaq Format/Long/ECC/buffer commands need logical-media versus controller
  function classification. Original 9-14 is not permission to silently treat
  an unsupported physical command as successfully completed.

Production/test edits remain deferred until the complete 22-row batch has
been reconciled and the complete owner-local repair design and expected
diff are reported. No ABI/model downgrade is approved by this evidence.

## HDC Original-Page Reconciliation, Second Pass

Read IBM Options/Adapters PDF pages 356--365 (AT adapter printed pages
3--12), Xebec PDF pages 308--317 (printed pages 7--16), and ATA-3 PDF pages
25/118 (printed pages 11/104). Visually inspected the IBM AT PIO page 362,
ATA interrupt page 25 and Xebec command/initialize pages 311/313. These
are original hardware/protocol sources, not another emulator's behavior.

### Confirmed Command And Interrupt Defects

- **H03/H05:** IBM AT adapter page 10 independently requires Set Parameters
  to retain the selected drive's maximum heads and sectors per track for
  multi-sector track/cylinder crossing. It also defines Verify as Read with
  no host data and one completion/error interrupt. Therefore the previously
  observed no-op 91h and one-sector Verify are defects for IBM as well as
  Compaq; the repair must retain each personality's command masks/defaults.
- **H04:** IBM AT adapter page 9 explicitly excludes an interrupt before
  the first write sector and after the last read sector. Current
  `x86_hdc_begin_write()` raises IRQ for that first buffer, and
  `x86_hdc_next_read_sector()` calls IRQ-producing `x86_hdc_complete()`
  after the final read. Both are protocol defects, not extra harmless
  notifications. Subsequent block IRQs and write completion IRQ remain
  required. Compaq interrupt rules still need their own reconciliation.
- **H02:** ATA-3 page 11 independently excludes the first write data block
  from the pending-interrupt condition. The same unconditional initial
  write IRQ is therefore wrong for ATA. The separate ATA read-completion
  protocol must be checked before applying IBM's final-read rule to ATA.
- **H01:** ATA-3 section 7.38 lists both Write Sectors 30h and its no-retry
  variant 31h as mandatory. Current ATA dispatch accepts only 30h. The
  logical-media retry/error boundary does not justify rejecting the no-retry
  command. Read 21h requires the corresponding original section check.
- **H07:** Xebec's defined-command predicate accepts Test Ready, Recalibrate,
  Verify, Seek, Initialize, buffer and diagnostic commands, but current
  dispatch handles only Sense and Read/Write; every other defined command
  unconditionally returns drive-not-ready sense 04h. Initialize collects
  eight bytes but never applies them. IBM XT adapter pages 10--13 explicitly
  define these controller functions. Presence, logical seek/verify, parameter
  storage and sector-buffer operations cannot all be excluded as absent
  physical ECC/rotation. Each defined command needs a separate disposition
  and direct receiver, not a blanket drive-not-ready fallback.

### Verification Gaps And Remaining Classification

Existing chip contract coverage for 91h proves status/completion only; it
does not prove programmed crossing geometry. Xebec contract tests cover
Read/Write DMA, cancellation and record bounds, not the omitted command
functions. Existing task-file IRQ assertions must be reconciled against
the original block/completion rules rather than preserved as source truth.

The IBM AT source names Format as a 512-byte interleave-table host transfer
and Read/Write Long as data plus four ECC bytes. These differ from ordinary
512-byte records and require explicit media-model classification; accepting
a Long opcode as a short transfer is not a proven implementation. No
production repair, new ABI, dynamic qualification or whole-batch acceptance
is claimed by this second pass.

## FDC And Media Reconciliation, Third Pass

Read Intel PDF pages 7--12, including printed 6-230/6-231 transfer and TC
rules, 6-232 Format and 6-233/6-234 Scan/Specify. Re-read complete transfer,
format, dispatch, reset and due-consumer paths, the Board-Base FDC timing/
port wiring, and Machine media replacement/removal receivers.

### Additional Confirmed Defects

- **F05:** Intel's MT operation spans the two sides of one cylinder.
  Current `start_transfer()` budgets only the initial head's sectors, and
  `advance_position()` never switches head at EOT. `complete_dma_data()`
  changes result CHR after TC, but does not implement the second side's data
  transfer. Result-only MT coverage cannot prove the missing payload path.
  Read/Write/Scan progression and SK accounting must be reconciled together.
- **F08:** Intel 6-232 explicitly requires Format result R+1 after the last
  formatted ID. Current `format_byte()` records that ID's unincremented R.
  This is a result-contract defect even with the retained flat 512-byte
  medium boundary; physical gap/interleave emulation is a separate question.
- **H01:** ATA-3 section 7.18 confirms mandatory read variants 20h/21h;
  section 7.19 confirms Verify 40h/41h, count zero = 256, no DRQ and one
  completion interrupt. Current ATA personality rejects 21h and both Verify
  variants. Figure 10 (PDF 124--125) ends after the final data transfer,
  independently confirming that the extra final-read IRQ is also incorrect
  for ATA, not just IBM.
- **H01/H02:** Identify unconditionally sets word 49's LBA capability bit,
  even when `config.lba28_supported` disables LBA command addressing. The
  public configuration already owns this capability; reporting and command
  admission must agree without adding another capability mirror.

### Retained Media Ownership And Source Boundaries

- **M01:** FDD/HDD insertion opens the requested existing Lib medium mode
  and uses the sole provider/registry path. Candidate validation precedes
  replacement. Removal returns Storage's close status after consuming the
  lease; `close_failure_smoke` directly asserts both returned I/O error and
  absent/null media, and replacement asserts new payload remains installed
  when the old lease close reports failure. This is current source/test
  coverage, not a fresh dynamic pass. Lib storage remains read-only.
- **F12/T01:** Board FDC installs configured ports once, receives DMA2/IRQ6
  bindings, and supplies step/reset/byte ticks to the chip's existing due
  path. Its 31/15 inputs currently serve as byte cadence, while rate 250
  supplies zero. This cannot prove operation-specific overrun timing or all
  three documented data rates. Exact clock scaling and the byte/service
  distinction remain required timing reconciliation before repair.
- **H01:** ATA-3's mandatory-command table includes more than the selected
  three-command implementation. Diagnostic, Initialize, Seek, Set Features,
  Multiple and DMA require explicit selected-controller/capability
  dispositions; this ledger does not declare full ATA-3 compliance from
  Read/Write/Identify success. Unsupported DMA wiring is not permission to
  omit unrelated PIO/non-data functions.
- **H05:** Compaq Identify's detailed table is explicitly ESDI parameter
  information (9-16). The selected 40-MB WD/ST506 personality must not borrow
  that ESDI identification layout without source proof. The initial concern
  about Read/Write Buffer was corrected by the original command-table note
  during the selected-model reconciliation below: the 40-MB controller
  explicitly does not support all three commands.

The complete batch is still not accepted. Remaining original-clock/zero
encoding, Compaq control/interrupt, Xebec completion and selected capability
questions must be disposed before the production repair design is frozen.

## Fixed-Disk Control And Shared-Port Reconciliation

IBM AT adapter printed page 15 (PDF 368) defines 3F6h bit 2 as fixed-disk
reset, bit 1 as interrupt disable, and bit 3 as enabling head-select 3
instead of reduced write current. Printed page 5 makes Drive/Head's low
four bits the head number. Compaq 9-10 (PDF 125, visually reviewed) uses
the same reset/interrupt/head-extension control positions and defines
3F7h as active-low drive selection and inverted four-bit head selection.

- **H04:** Current IBM control write retains only bit 3, so it drops reset
  and interrupt-disable. `irq_pending()` consults `device_control`, which
  this branch does not set. These control functions are confirmed missing.
- **H04/H06:** Head extension is an enable, not a forced head-number bit.
  Current IBM `current_head()` ORs control bit 3 into the low three head
  bits, forcing head 8 even when Drive/Head requests head 0. Current Compaq
  uses all four head bits irrespective of the extension-enable bit. Both
  must use the command head with the selected control gate, while preserving
  each protocol's register presentation and per-drive parameters.
- **H06:** Current Compaq Drive Address returns `drive_head & 1fh`, which
  does not encode the documented active-low two-drive and complemented-head
  outputs. Board composition already combines its response with FDC DIR
  through one wired-OR read route at the configured shared port. Preserve
  that owner/wiring; repair the HDC contribution rather than adding another
  shared-port path. Write Gate and the reserved multipurpose-controller bit
  need explicit selected-output treatment, not copying arbitrary command
  bits into their positions.

These defects are source/current-path confirmed. Existing topology or head
extension tests must be reviewed for wrong polarity/forced-bit oracles.
No dynamic test or production change is claimed by this reconciliation.

## FDC Reset And Status Reconciliation

Intel's RESET pin description (PDF 2, printed 6-225) says reset does not
clear the last Specify command; its ND bit is part of that command, not a
board policy. Current reset preserves SRT/HLT/HUT but clears NDMA through
the whole-data reset. **F11 requires preserving all Specify values.**
The NEC original was inspected as a comparison only (SHA-256
E8A37F1242D4E26B400A524519B0B2EFEA8A68B83B4770E7A081BB82F6EDFEBC);
it is not substituted for Intel's timing authority.

Intel Table 3 (PDF 4) says NDM is set during non-DMA execution and clears
when execution ends. Current `msr()` drops NDM along with RQM while a
non-DMA byte gate is pending, despite still being in execution. **F10
requires independent phase/direction and byte-readiness bits.** Per-byte
IRQ acknowledgement likewise must not consume a pending seek/READY cause.

Intel Table 12 defines ST0 HD as the head at interrupt. Current result
construction always copies HD from command byte 1. **F05 requires the
actual selected side after MT progression to be reflected in terminal
status**, not just in the separate result H byte.

HLT/HUT zero encodings are not defined by the inspected Intel or NEC timer
descriptions. Nonzero duration execution is a confirmed missing function;
zero must remain an explicitly classified model boundary until a primary
encoding source exists. No zero encoding may be silently presented as L3.

## Selected-Model Correction And Repair Boundaries

Read Compaq PDF pages 121--128 and visually reviewed page 124, printed 9-9.
This step checks the command table's notes against the selected 40-MB
personality, rather than treating every command described in chapter 9 as
applicable to that controller.

- **H05 retained exclusion:** note 4 explicitly says the 40-MB drive does
  not support Identify, Read Sector Buffer or Write Sector Buffer. Their
  present rejection is correct for the selected personality. Add a direct
  rejection receiver; do not implement them using the ESDI table. This
  supersedes the earlier buffer-function candidate, not an implemented fix.
- **H06/T01 confirmed repair:** Compaq's step-rate table specifies code 14
  as 3.2 microseconds and code 15 as 16 microseconds. Board-Base currently
  fills codes 1--15 using the IBM half-millisecond formula, producing 7 and
  7.5 milliseconds for those two codes. Retain the existing sixteen-entry
  timing input, select the original personality-specific values at the board
  owner and use checked ceiling conversion. No new HDC public interface is
  needed for that correction. The chip's current step-selector helper also
  exits for every non-IBM personality, so Compaq never consumes even its
  supplied table. Correct both the board input and its actual receiver;
  changing the unused input alone is not a repair. Compaq Recalibrate explicitly uses its low
  command bits for subsequent implied seeks, not for the recalibration
  movement itself; command selection and timing application must remain
  distinct.
- **F01 confirmed repair:** Intel Table 4 specifies fixed zero opcode bits
  for Specify, Sense, Seek and Recalibrate, and independent permitted flags
  for data, ID and Format commands. Current command-length/dispatch masks
  every opcode to five bits, accepting otherwise invalid aliases. Validate
  the complete opcode at the same command admission owner. Read Track's SK
  bit is shown in the table but the narrative says it has no effect; do not
  turn that ignored documented flag into a reserved-bit rejection.
- **F07 confirmed repair:** Read ID returns the first correct ID on the
  selected cylinder (Table 4). The flat-record model has no rotational ID
  cursor, but the current path carries a previous command's R unbounded by
  the selected track. Retain explicit first-logical-ID modelling and return
  a valid selected-track ID, not stale result state. Raw ID/CRC absence
  remains outside the flat-record contract.
- **F09 confirmed repair:** Intel 6-233 defines byte comparison with FF as
  the largest value, and Table 10 defines equality versus inequality.
  Current host FF bypasses comparison and preserves Scan Hit for unequal
  disk bytes. The independent receiver must include disk 00/FE/FF against
  host FF for all three scan commands; FF is not a wildcard in this Intel
  contract.
- **F11 confirmed repair:** Intel's polling feature (6-230) starts after
  power-up RESET, between commands and Seek steps; it does not require
  Specify. Current reset clears `ready_poll_enabled`, and only Specify
  enables it. Preserve the sole caller-owned READY sampling route but remove
  this incorrect Specify prerequisite, with a reset-before-Specify receiver.

### Planned Owner-Local Change Surface

The estimates below are planning ranges, not actual diff or qualification.
They cover the complete storage repair, not only an initial IRQ patch.

| Owner and existing paths | Responsibility | Estimated additions / deletions |
| --- | --- | --- |
| `core/chips/fdc8272/fdc.c`, `fdc.h` | Full command admission, MT/SK/Scan/Format/ID results, NDMA IRQ/MSR/reset, head timing, overrun and checked deadlines | 240--380 / 130--220 |
| `core/chips/fdc8272/fdc8272_interface.h` | Only a separately reported timing-input need, if approved; no new observer or execution API | 4--12 / 0--3 |
| `core/board-base/fdc.c` | Byte cadence versus operation-specific service limits and selected clock/rate conversion at the sole board owner | 35--65 / 15--35 |
| `core/chips/hdc/hdc.c`, `hdc.h` | Per-drive parameters, multi-sector Verify, command variants, PIO IRQ, controls/head addressing, selected Xebec functions and deadlines | 230--380 / 90--160 |
| `core/board-base/hdc.c` | Original IBM/Compaq step-code conversion using the existing timing table | 15--30 / 5--15 |
| Existing FDC/HDC chip tests and fixtures | Source-independent command/result/failure/count/timing matrices; replace incorrect IRQ oracles | 650--1000 / 60--130 |
| Existing Board-Base FDC/HDC tests and fixtures | Actual ports, shared-port polarity, DMA/PIC and guest-time receiver proof | 220--350 / 25--65 |

Machine media ownership and its close-failure implementation are retained;
their existing tests must be rerun, not rewritten into another backend.
No App-specific BIOS response, second media route, Shared-six change or
MyNES change is included. Core manifests and the eight affected PC artifacts
are delivery obligations after the complete batch is implemented and verified.

### Remaining Design Prerequisites, Not Accepted Dispositions

The batch still requires explicit reconciliation of Compaq PIO notification
rules, FDC post-reset READY polling and terminal EOT/TC ordering, ATA selected
non-data/Multiple capabilities and Xebec physical-command exclusions. These
are not accepted merely because the plan names their owners. The public FDC
timing input presently supplies only two byte durations; it cannot separately
express FM/MFM Read/Scan and Write service windows while those fields retain
byte-cadence meaning. Report the exact input extension before touching that
header. HLT/HUT can use the existing Specify step-clock ratio and private
state; they do not justify a new public head-load API.

No production or test implementation, dynamic replay or artifact refresh is
claimed by this design reconciliation. S4 remains active and its entire
22-row batch remains subject to direct implementation/receiver proof.

## Coordinator Batch Disposition Before Implementation

The following matrix reconciles all twenty-two members into a repair,
retention or explicit research prerequisite. It is an implementation plan,
not acceptance. Source-incomplete members stay in S4 and prevent its closure;
they do not prevent a source-confirmed owner-local repair in the same batch.
No partial implementation P is eligible for delivery.

| Row | Required disposition and next proof |
| --- | --- |
| F01 | Repair full-opcode admission; preserve fifteen command lengths/results and prove the original permitted/ignored flags. |
| F02 | Repair nonzero head-load/unload execution using the existing clock ratio; retain explicitly non-physical zero encoding and absent electrical output boundaries. |
| F03 | Retain sampled ST3 pins/head/unit; rerun its independent direct receiver and selected board pin tests. |
| F04 | Retain per-unit seek/recalibrate/SIS and 77-step limit; include deadline arithmetic and simultaneous cause regressions in F11/T01. |
| F05 | Repair MT payload/progression and actual terminal head identity; resolve EOT versus TC ordering against Tables 7/8 before final acceptance. |
| F06 | Repair unjustified drive-zero/DMA/full-track restrictions; retain explicit 512-byte/raw-rotation/CRC model boundaries. |
| F07 | Repair stale ID R; prove selected track/head and unavailable-ID result within the flat-record model. |
| F08 | Repair Format R+1, retain sole mark/fill provider, prove incomplete IDs, TC, protection and provider failure. |
| F09 | Repair FF comparison and MT/SK progression together; retain Table 10 status semantics and prove STP/EOT termination. |
| F10 | Repair MSR and per-byte NDMA IRQ/ack plus overrun; exact independent timing-input need must be reported before interface edits. |
| F11 | Repair Specify-preserving reset and post-reset READY enable; prove cause-preserving byte/result/SIS acknowledgements and service gating. |
| F12 | Retain one port/DOR/DIR/DMA2/IRQ6/provider route; receiving tests must prove corrected chip behavior through all four fixed inputs. |
| H01 | Repair ATA variants, Verify and Identify reporting; classify the selected PIO capability set explicitly rather than claiming full ATA-3. |
| H02 | Repair source-confirmed PIO IRQ boundaries, preserve status/alternate acknowledgement and correct reset/nIEN; prove admission and cancellation. |
| H03 | Repair IBM parameters/Verify; dispose Format and Long individually using logical versus raw-media source boundaries, not successful short-transfer aliases. |
| H04 | Repair reset/nIEN and gated head bit; prove one IRQ14/control/data port receiver and original step table. |
| H05 | Repair Compaq per-drive parameters/Verify; retain source-required Identify/Buffer rejection for the 40-MB model and prove it directly. |
| H06 | Repair inverted Drive Address and step-code receiver/conversion; preserve shared FDC port. PIO notification authority remains a named source prerequisite. |
| H07 | Repair Xebec logical ready/recalibrate/verify/seek/initialize/buffer/diagnostic functions; classify Format/ECC/Long individually without drive-not-ready blanket fallback. |
| H08 | Retain one mask/select/reset/DMA3/IRQ5 route; prove every repaired Xebec command's actual response, TC/cancellation and service receiver. |
| M01 | Retain current sole Lib lease/provider route and implemented close-error semantics; rerun replacement/removal/mode regressions. |
| T01 | Repair checked monotonic deadline/conversion arithmetic and Compaq step inputs; retain command/service/mechanical model origins explicitly and prove scheduler receiving routes. |

The first internal implementation work changes existing HDC completion,
command predicates, control/head decoding and Identify reporting, plus their
existing chip tests. Planned production delta is approximately +25/-30 and
test delta +90/-5, excluding later required parameter/Verify/Xebec work.
Compaq-specific PIO notifications are unchanged until their original-source
prerequisite is resolved. No public API or model downgrade is part of this
internal work. The complete S4 implementation, verification and artifacts
remain one delivery obligation.

## First Internal HDC Implementation And Receiving Proof

This is partial worktree evidence, not S4 acceptance or a runnable artifact
qualification. IBM/ATA first-write DRQ no longer generates an initial IRQ;
their final PIO-in block does not generate a second completion IRQ. Existing
status acknowledgement, nIEN and SRST share the sole control path. IBM and
Compaq head bit 3 now obey the control-register enable rather than forcing a
head or bypassing the gate; address advancement no longer rewrites that
enable. The duplicate private fixed-disk-register field is removed while the
existing copied observation is derived from control. ATA 21h/31h variants and
Identify capability/count isolation use their existing command path.

The taskfile receiver covers both IBM and ATA two-block read/write IRQ
boundaries, masking/unmasking, reset and a failed read followed by Identify.
The existing contract receiver additionally uses a 16-head medium with exact
record admission: head 0 with enable, head 8 without/with enable, and a
two-record head 7 to head 8 crossing. Both WD personalities must deliver the
correct payload and preserve the enable bit. This rejects the old IBM forced
head and Compaq ungated-head interpretations, not just a copied-register
oracle.

Both original Release Ninja trees rebuilt the changed contract and all HDC
receivers with the persistent ccache directory and eight jobs. The focused
`ctest -L unit -R hdc --output-on-failure` selection passes 8/8 on x64 and 8/8
on x86, including three chip, three Board-Base and two App receiving tests.
Source/test Core manifests and Core dependency corpus pass. These results
do not qualify the remaining parameter, Verify, Xebec, FDC or timing members;
Compaq PIO notification remains unchanged pending source reconciliation.

At this checkpoint `git diff --numstat` counts HDC production +22/-29
(net -7), existing HDC tests +147/-4 (net +143), excluding manifests and
documentation. No public interface, Shared-six, MyNES, INI, media, snapshot
or deployed executable has changed. Complete units, four-App integration,
desktop replay and eight-artifact refresh have not yet run for S4. No
implementation P is delivered before the entire admitted batch is complete.

## Verify Implementation Checkpoint

Original ATA-3 PDF page 84 (printed 70, section 7.19) confirms 40h/41h,
the requested sector count, no DRQ/payload, completion interrupt and first
unrecoverable sector address. Compaq PDF page 129 (printed 9-14) defines
Verify as Read Sectors without the system transfer or Read Long. The existing
IBM 40h/41h command grammar is retained. The sole HDC command dispatcher now
uses one bounded Verify loop, reusing record admission/read, count reduction,
CHS/LBA advancement and failure/completion. It does not add a phase, public
interface or mechanical-time claim; the current command service remains the
existing order-only model.

Five protocol/opcode combinations exercise one/two records, a third-record
failure, zero-count failure with 254 records remaining, and complete 256-record
success. The failed record address and remaining count are asserted, with no
host data phase, a completion/error interrupt and normal status acknowledgement.
The Compaq board fixture no longer accepts arbitrary offsets; its real port
receiver explicitly requests two Verify records on the slave drive and proves
exactly two additional provider reads, zero count and no DRQ/BSY/error.

After both changed receivers were rebuilt on the original cached trees, the
eight HDC units pass once per width (x64 8/8, x86 8/8). Both Core manifests,
Core dependency corpus and whitespace check pass. Aggregate counted HDC
production is +39/-35, net +4; chip/board tests +217/-13, net +204, excluding
documentation/manifests. This checkpoint has not run complete units,
four-App integration, desktop acceptance or deployed EXE refresh. S4's
remaining parameter, Xebec, FDC, timing and source-reconciliation obligations
remain open; no partial implementation P or S/T closure is claimed.

## Compaq Drive-Address And Step-Input Checkpoint

Compaq original PDF pages 124--125 (printed 9-9--9-10) were re-read
before editing. The command matrix's R bit also permits Verify 41h; the
earlier implementation restriction to 40h was not source-backed. Both forms
now use the same bounded Verify path and the same direct matrix.

Drive Address now reports the selected drive through bits 0/1 active low and
the effective gated head inverted through bits 2--5. Bit 7 is left clear for
the sole board wired-OR multipurpose-controller input. Physical WRITE GATE
is not represented by the flat-record provider and remains inactive in this
model; data-write phase is not falsely treated as an electrical write pulse.
Eight direct table rows prove both drives, enabled/disabled head bit 3 and
heads 0/10/15 without touching media. Both real board port receivers reject
the old raw Drive/Head loopback; the dual-drive receiver also proves the FDC
bit-7 contribution and slave selection independently.

The existing step selector now consumes Compaq's injected table as well as
IBM's; IBM-only Diagnose reset-rate behavior remains IBM-only. Board
conversion uses one wide multiply followed by upward tick quantization,
preserving fractional microseconds and avoiding divide-before-multiply loss.
Compaq codes 14/15 are 3.2/16 microseconds; all sixteen codes are checked
through the real Recalibrate command and board observation at 8 MHz. The
table is source-backed, while actual seek mechanics remain the existing
non-physical model and are not upgraded by reporting a duration.

Original x64/x86 Ninja trees rebuilt all changed HDC receivers; the complete
focused HDC selection passes 8/8 per width. Source/test Core manifests,
component boundary and whitespace gates pass. Aggregate counted production
is +57/-44, net +13; existing chip/board tests +263/-16, net +247, excluding
documentation/manifests. No Shared/MyNES, public API or asset changes occurred.
S4 remains incomplete: complete units/integration and eight EXEs are still
required after the remaining storage batch is implemented and reconciled.

## WD Per-Drive Parameters Checkpoint

Compaq PDF page 126 (printed 9-11) explicitly assigns independent maximum
head/sectors-per-track parameters to both drives, defaulting to 4 heads and
17 sectors when Initialize has not been issued. IBM original PDF page 363
(printed adapter page 10) requires Set Parameters before multi-sector work
and assigns track/cylinder crossing to its values. Both WD personalities now
store two private per-drive head/track counts in HDC, not in the medium or
board. One address-geometry helper applies them to the existing CHS resolver
and advancement; the independent physical medium/capacity checks remain.
IBM drive-1 selection now reaches its existing unit-aware provider rather
than being rejected as if the entire controller were single-device ATA.

Direct tests initialize unlike parameters on two drives, check exact record
addresses and head/cylinder crossings, return to the first drive to prove
isolation, and verify that an invalid zero-sector initialization leaves the
accepted parameters intact. Reset restores Compaq's documented defaults and
IBM's prior pre-initialization geometry model. The latter is explicitly not a
source-qualified alternative to the manual's required initialization. Zero
sectors per track are rejected as an invalid flat-record model input; no
undocumented zero encoding is claimed as physical behavior. The Compaq real
port receiver also proves different per-drive parameters through exact
provider offsets and two-record Verify.

The changed chip and board receivers pass on both original Ninja/ccache trees:
x64 HDC 8/8 plus the subsequently strengthened board receiver 1/1; final x86
HDC 8/8. Aggregate counted production is +100/-53, net +47; chip/board tests
+382/-17, net +365, excluding manifests/documentation. No public interface,
Shared/MyNES or artifact change occurs. ATA Initialize/Identify-current
geometry, remaining Xebec/FDC functions, deadline and source-reconciliation
items remain in the admitted S4 batch. Complete units, four-App integration
and eight-artifact refresh are still pending; this is not an implementation P.

## ATA Translation And Identify Checkpoint

ATA-3 original PDF page 75 (printed 61, section 7.11) requires saving the
translation, generating a completion interrupt, reporting ABRT for an
unsupported translation and IDNF on subsequent media accesses until valid
parameters are established. PDF page 68 (printed 54, sections 7.7.21--27)
separates native geometry, current CHS translation/capacity and total LBA
capacity. The command dispatcher now shares its sole 91h parameter owner
with WD. ATA derives current cylinders from total records divided by the
requested heads/sectors, capped at 65,535. Unsupported parameters use the
existing private zero-track parameter representation, not a second invalid
flag; CHS and LBA media admission both reject that translation.

Identify preserves words 1/3/6 and 60/61 and now reports current fields
53--58. Its direct receiver checks 128-record native geometry 4/8/4,
translation 32/2/2, exact last-record CHS access, no reads during invalid
CHS/LBA attempts, and recovery through valid native parameters. The selected
flat-record model supports nonzero translations fitting at least one logical
cylinder; this is not full ATA-3 command-set qualification.

Final focused HDC units pass 8/8 on each original Release architecture after
rebuilding their changed production dependencies. Aggregate counted
production is +141/-55, net +86; existing chip/board tests +455/-17, net +438,
excluding manifests/documentation. The new size reflects real command state,
translation reporting and direct failure/receiver matrices, not a new public
interface, queue or platform path. Remaining storage members and full S4
unit/integration/artifact qualification are still required before delivery.

## Xebec Logical Command Checkpoint

IBM Options/Adapters Volume 2 original PDF pages 304--310 define completion,
address-valid sense and error codes; pages 311--314 define the DCB command
forms. The sole DCB dispatcher now implements Test Drive Ready 00h,
Recalibrate 01h, Seek 0Bh and Read Verify 05h in the existing logical-record
model. Ready/Recalibrate ignore their unspecified CHS bytes; Seek checks the
required zero sector field and address bounds; Verify reads the requested
records without DMA/payload delivery. Completion uses the existing CSB/IRQ
path. No physical step, seek overlap, rotational/ECC or exact-time claim is
made. Zero Verify count remains rejected as an unsupported model input,
not an invented ATA 256-record encoding.

The medium query and transfer route now consume the DCB drive selection
instead of forcing drive zero. End-of-medium progression preserves the first
out-of-range cylinder in the existing DCB, so sense no longer names the last
successful record. The 15-row direct matrix covers present/absent selected
drives, ignored fields, illegal head/cylinder/Seek-sector fields, one-record
and cross-head/cylinder Verify, zero count, end-of-medium failure and unknown
command. It checks full four-byte sense, accepted read counts, no writes/DRQ,
completion IRQ acknowledgement and deadline termination.

Final related HDC units pass 8/8 per width on the original Ninja/ccache trees.
Aggregate counted production is +196/-61, net +135; chip/board tests +532/-17,
net +515, excluding documentation and manifests. No new public API, persistent
state, Shared/MyNES or artifact change occurs. The generic transfer-failure
sense mapping, physical seek timing, initialization, format/buffer/Long and
diagnostic command dispositions remain pending in the complete S4 batch.
Complete units, four-App integrations and eight-artifact qualification have
not run for this checkpoint; it is not a complete implementation P.

## Xebec Buffer, Response And Failure Checkpoint

The same IBM original command table (PDF page 313, printed Fixed Disk Adapter
12) defines 0Eh/0Fh as controller sector-buffer operations with DCB bytes
1--5 ignored. They now transfer the existing 512-byte buffer without a drive
query/read/write. Programmed DATA-port I/O and DMA both consume the existing
byte-transfer owner; the DMA mask alone controls DRQ. No second buffer or
phase is introduced. Direct DMA/PIO round trips test absent drive zero and
ignored CHS/count fields. The real board receiver uses 320h/322h/323h and
checks data, IRQ5 completion/acknowledgement, no spurious DMA3 and unchanged
medium bytes.

PDF page 304 (printed 3) requires an ending CSB for all commands. Request
Sense now supplies four sense bytes followed by that CSB, raising completion
IRQ when the CSB becomes readable and clearing it when consumed. The existing
five-byte response array owns the complete response. Direct tests and the
board's previous four-byte-only oracle now consume the full result.

Record preparation now has one owner for selected-drive query, immutable
geometry/512-byte admission, address/capacity and write protection. All
Read/Write/Verify paths propagate its failure and use one addressed sense
completion. Incompatible write geometry is rejected before DMA payload
acceptance. Provider RANGE maps to illegal address (21h), ABSENT to not ready
(04h), and write refusal to write fault (03h). Generic provider read failure
maps to the logical data-error class (11h); this is explicitly an L2
flat-record mapping, not detection or correction of physical ECC. Failure
injection covers the three commands and existing provider outcomes, plus
first-record success/second-record failure with exact sense and one accepted
prefix only. Initialize byte ordering/semantics, physical diagnostics,
format/Long/ECC, selected step timing and premature TC disposition remain
required unresolved H07/H08/T01 work. This checkpoint does not accept those
rows or qualify the whole S4 batch.

After rebuilding all changed HDC recipients, the final related set passes
9/9 on x64 (4.07 s) and 9/9 on x86 (0.89 s), including the Xebec real-port
receiver. Core source/test manifests, corpus, documentation governance and
diff whitespace gates pass. Aggregate tracked source/test numstat for the
current whole S4 worktree is production +284/-135 (net +149) and tests
+684/-24 (net +660), excluding manifests, documentation and artifacts.
The two private helpers own record admission and sense completion; they
replace duplicate checks/result construction and introduce no persistent
state or public API. Complete S4 units/integrations, deployed artifacts and
desktop acceptance remain unexecuted; no implementation P or S closure is
claimed.

## FDC Command Admission, Specify Reset And Read Track Checkpoint

Intel 8272A Table 4 owns the complete eight-bit command grammar. One private
opcode decoder now rejects reserved high bits before command-length admission
and execution. The direct independent table enumerates all 256 command bytes;
invalid forms return the one-byte 80h result without media access or IRQ/DRQ.
The reset path preserves the complete last Specify command, including ND,
as required by the original reset description, rather than only its timers.

Read Track no longer imposes unsupported MFM-only, DMA-only or unit-zero
restrictions. EOT selects the number of records up to the available track,
not necessarily the whole track. A 64-row direct matrix covers all four units,
FM/MFM, SK, DMA/NDMA and two partial/full-track counts. This remains the
existing flat 512-byte sequential-ID model beginning at R=1; it does not
establish physical index/ID ordering, NDMA interrupt service windows or
head-load/unload timing. Those remain required unresolved S4 receivers.

The initial new fixtures incorrectly supplied a zero step denominator and
were rejected during creation; fixing the test input exposed no command-path
failure. The Model40 rate-mismatch receiver used illegal E5h/E9h write command
forms. They now use legal C5h/C9h while preserving the same rate-mismatch
assertions; the complete byte matrix retains illegal-form coverage.

All 13 related FDC routes pass on x86 after rebuilding every changed receiver.
On x64 the same 13 pass, with the three Core Machine recipients then explicitly
relinked and re-executed (3/3) to exclude stale executable evidence. No Shared
or MyNES code, new public API, state or product artifact has changed. Full
S4 qualification and implementation-P delivery remain pending.

## FDC Non-DMA IRQ And ID/Format Checkpoint

Intel PDF pages 3--4 (printed 6-226/6-227) require each non-DMA data byte
to request IRQ and its data RD/WR to acknowledge it. The chip now publishes
the request at initial byte availability and each existing byte-gate expiry.
RD/WR clears the byte cause while retaining queued seek/READY causes. The
same execution phase, byte gate and IRQ owner remain; no new persistent
state or public API is used. MSR retains NDM and read direction while only
RQM is clear during the byte wait (Table 3). The four-form direct receiver
covers Read, Write, Scan and Format initial request, acknowledgement, early
access rejection, pre-deadline exclusion, next request and reset. Model40's
real DATA port receiver additionally asserts IRQ6 before the first byte and
its clearing after the read. Physical overrun deadlines are not yet modeled.

Intel PDF page 9 (printed 6-232) requires Format's returned R+1 and Read ID's
first available ID, not a previous command's leftover sector. Format now
increments the last successfully supplied R; Read ID reports R=1 in the
explicit existing flat sequential-ID model. It rejects empty or non-512-byte
track metadata instead of claiming a valid ID. A nonsequential Format header
order 3/1/2 verifies fill and final R=3, followed by Read ID returning R=1
on the selected head. This does not add a rotational position or arbitrary
on-disk ID model. Source/current reconciliation of that boundary remains F07.

Final related FDC receivers pass 13/13 per width after rebuild. The final
additional Model40 IRQ6 assertions are rebuilt and pass x64 (1/1) and are
included in the final x86 13/13 run. Whole S4 units, integrations and artifacts
remain pending. Counted FDC source is +74/-15, tests +175/-7, excluding
documentation/manifests. The private request/acknowledgement helpers own IRQ
publication, not another queue or controller.

### Required Interface Decision Before Overrun Repair

Existing `fm_byte_ticks`/`mfm_byte_ticks` express cadence only. Intel's Read
and Scan service limits are 27/13 microseconds versus Write's 31/15, and the
selected board also scales recording rates. Reusing cadence as the timeout
would conflate two facts. A proposed bounded extension is four guest-axis
duration fields in the existing `x86_fdc_timing`: FM/MFM read-service and
FM/MFM write-service ticks. Board-Base supplies selected-clock/rate conversion;
the chip selects by operation and encoding. The existing due scheduler would
consume one private outstanding-service deadline, returning ST1 OR on expiry
through the normal completion path. No new public function, second scheduler,
retry or Shared change is needed. No such interface extension or overrun
implementation has been made; the active packet requires reporting this ABI
need before execution. HLT/HUT execution and zero encoding need their separate
source/input reconciliation and are not hidden by this proposed extension.

## Media Receiver Verification Checkpoint

The existing Machine FDD/HDD providers and Board-Base registry still form
the sole byte/sector route to Lib Storage. Current insertion validates the
candidate before replacement; removal consumes the lease, clears presence
and returns its close status. Storage implementation remains read-only for
this task. The retained tests directly exercise readonly write refusal,
overlay discard without master-file changes, direct persistence, rejected
candidates preserving the installed medium, replacement close failure with
the new medium installed, and removal close failure with the medium absent.

The seven media/provider/lifecycle and Model40 media receivers were rebuilt
on the original cached Release trees. They pass once per width: x64 7/7
(3.44 s), x86 7/7 (0.86 s). This supplies fresh runtime evidence for M01's
retained access-mode and replacement/removal contract, not full controller
or four-App integration qualification. No production change was needed for
these observations. Complete S4 units, integrations and artifact refresh
remain pending, as do the unresolved FDC and HDC batch members.

## Scan Comparison Checkpoint

Intel PDF page 10 (printed 6-233), Scan Commands and Table 10, was freshly
rendered and visually reviewed. It defines FFh as the largest comparison
value. The existing host-FF comparison bypass incorrectly treated it as a
wildcard and preserved SH against unequal disk data. Removing that branch
uses the same equality/low/high comparison and existing sector-result owner
for every byte. No helper, persistent state or public interface is added.

The existing direct comparison table now checks seven disk/host pairs for
each of the three commands, including 00/FF, FE/FF, FF/FE and FF/FF. Before
the production repair these new cases failed in six rows; after repair they
pass. The real board DATA-port receiver also contained a stale no-care
comment and host-FF filler. Equal and high comparisons now supply actual
zero filler matching the disk; low comparison retains FF as a legitimate
upper bound. A further full-sector host-FF Equal request explicitly proves
SN rather than SH through the same port and media path. All prior deleted
mark, DMA, readiness and result assertions remain.

The first x64 regression run exposed that stale board oracle and also a
30-second static FDC negative-check timeout. The oracle was corrected from
the source; the timeout is retained as an observed failure, not diagnosed as
a production defect. Subsequent final runs use unchanged budgets and original
trees: x64 13/13 (6.87 s), x86 13/13 (6.77 s). No system configuration change
or replacement build tree was used. MT/SK progression, head timing, overrun
and remaining HDC/source members are still unresolved in S4. These focused
passes do not establish complete unit, integration, desktop or artifact
qualification and are not a partial implementation P.

## Multi-Track Payload Checkpoint

Intel PDF page 8 (printed 6-231), Tables 7/8, distinguishes transfer of both
sides of one cylinder from the terminal-count result address. The previous
owner stopped after the first side: the new R2/EOT3 receiver consumed only
1024 bytes instead of the required 2560. One private next-track preparation
helper now advances head zero to head one, R1, only while MT still has data
remaining. Read, Write and Scan consume that same preparation; no extra
state, queue, public interface or board-specific path is added. Starting on
head one does not wrap to another cylinder. Scan STP2 only adds the second
side when its first-side sequence actually reaches EOT.

The direct matrix checks all seven Read/Write/Deleted/Scan forms, both DMA
and non-DMA admission, starting heads zero/one, distinct side payloads,
deleted-mark SK transitions and Scan STP2 landing/missing EOT. The fixture
records the actual provider address, not a pre-byte cursor. Existing
single-sector board/App commands that accidentally requested MT were changed
to their single-track encoding. Model40's MT+DMA terminal-count command and
the existing complete board terminal-count matrix remain. A new real DMA2
receiver reads and writes 1024 distinct bytes across two sides through the
actual board ports, DMA and media registry, checking all bytes, provider
counts and terminal-count C/H/R.

The first new board DMA test failed because its predecessor deliberately
left the physical head at cylinder 3. Reset correctly leaves mechanical
position unchanged. The MT test now explicitly establishes cylinder zero;
no production reset workaround was made. The x64 13-route run initially
passed 12/13; after this fixture correction the sole failed route passes
(1/1, 4.04 s). The rebuilt x86 final group passes 13/13 (7.19 s). Earlier
MT/SK/STP and stale single-sector oracle failures are not passing evidence.
ST0 head/result reconciliation, head timing, overrun and remaining HDC
members still require their own complete dispositions. Complete S4 units,
four-App integration and artifact qualification have not run.

## ST0 Head Versus Terminal ID Checkpoint

Fresh visual review of Intel PDF page 12 (printed 6-235), Table 12,
confirms HD is the head at interrupt. The new direct MT and real DMA2
assertions fail on the preceding implementation: the entire side-one
payload is present but ST0 remains zero. Completion now captures HD in the
existing pending ST0 before TC constructs the next-record C/H/R. Result
publication adds only unit identity; Read ID supplies its selected HD at its
existing direct result path. No field, public interface or alternate result
owner is added. Error IC selection remains separate from HD/US identity.

The similar-path sweep found the natural-EOT/TC admission comparison also
treated the entire pending ST0 as an error predicate. A normal side-one HD
therefore prevented TC ID adjustment in eight of the existing board cases.
Admission now tests status bits excluding HD/US, retaining its other phase,
remaining-byte and ST1/ST2 checks. The unchanged expected TC C/H/R matrix
then passes, with the stronger independent ST0 assertion. Full two-side DMA
now returns ST0 HD=1 while its TC next ID is C1/H0/R1; these are deliberately
different facts, not duplicated state. The direct MT Scan STP2 receiver also
checks HD alongside both normal and abnormal termination status.

Final rebuilt related FDC groups pass 13/13 per width, x64 11.59 s and
x86 6.87 s. The initial two failing direct/board assertions and the
intermediate eight TC cases are retained here as diagnosis, not passing
evidence. This proves consistent H/HD flat-record transfers and TC output;
it does not qualify malformed H-versus-HD input or add physical head-load,
rotation or arbitrary ID behavior. Those input/model boundaries and the
remaining FDC/HDC rows still require reconciliation before S4 acceptance.
Shared-six, MyNES, owner INIs/media and product artifacts remain unchanged.

## Xebec Drive Diagnostic Checkpoint

The IBM Options/Adapters original, PDF page 314 (printed Fixed Disk Adapter
13), defines E3 Drive Diagnostic with selected drive and otherwise ignored
address bytes. The supplementary Xebec S1410 owner's manual, revision C1,
August 1983, PDF page 32 (printed 29), explains verification of sector zero
of every track without disk writes. The read-only external original is
`manuals-nxvm/hdc/xebec-s1410-owners-manual-revc1-aug1983.pdf`, SHA-256
`790793DB296587AEF55D0702522A258BC2717FC5B57C9ECCB01350C0B6F393A8`.
It was acquired during S4 on 2026-10-11 from the vendor-document archive
`https://bitsavers.trailing-edge.com/pdf/xebec/Xebec_S1410/104524C_S1410Man_Aug83.pdf`.
This corroborates the common diagnostic meaning, not IBM firmware identity,
reset geometry, exact diagnostic duration or all S1410 command behavior.
No manual, scan, OCR or vendor program enters this repository.

The new direct E3 cases fail on the preceding implementation, which returned
the generic drive-not-ready fallback. E3 now initializes its own address at
C0/H0/R0, ignores incoming address/count fields, walks every configured track
and uses the sole existing sector preparation/read/error/CSB path. Tests
verify both drives, four actual sector-zero reads (address sum 0+2+4+6),
zero writes, missing media, immediate provider errors and a failure after
the accepted first track with addressed sense H1/R0/C0. The real XT port
receiver checks the same partial failure through 320h/322h/323h, IRQ5
assertion/CSB acknowledgement, no DMA3 request and unchanged media bytes.

The similar-path sweep found configured CHS dimensions could exceed the
DCB's 10-bit cylinder, 5-bit head or 6-bit sector representation. The one
existing address validator now rejects those descriptions before any
transfer; read/write admission tests cover all three overlarge dimensions.
The diagnostic loop therefore cannot wrap its bounded local indices.
No new public interface, persistent state, helper or separate completion
path is introduced. Logical read verification is an explicit L2 behavior;
physical STEP/recalibration, ECC-circuit checks and diagnostic time are not
claimed. E0/E4 RAM/internal diagnostics and Initialize characteristics
remain unresolved members, not hidden by this E3 repair.

After rebuilding every HDC receiver on the original trees, final related
groups pass x64 9/9 (4.63 s) and x86 9/9 (0.91 s). The intermediate prefix
fixture initially had only one track, so could not inject a second-track
failure; its geometry is now explicitly two-by-two. Initial and intermediate
failures are diagnosis, not passing proof. Whole S4 units, integrations,
artifact rebuild, complete-P push and acceptance still remain pending.

Relative to this checkpoint's start, tracked production changes are +24/-1
(net +23), and its two test files are +56/-5 (net +51), excluding manifests
and evidence. These are incremental E3 counts, not the entire S4 diff.

## Xebec RAM Diagnostic Checkpoint

Fresh visual review of the supplementary S1410 original PDF page 31
(printed 28), section 4.5.17, confirms E0 performs a data-pattern test on
the controller RAM buffer, with DCB bytes 1--5 ignored. The IBM original's
Fixed Disk Adapter error table defines RAM diagnostic failure as sense 30h.
The newly added direct receiver failed on the previous implementation,
which incorrectly returned drive-not-ready. E0 now exercises the existing
512-byte controller buffer and uses the sole response/CSB/IRQ path without
querying, reading or writing a medium. The chosen 00/FF/55/AA pattern sweep,
its final AA contents and the existing configured completion delay are an
explicit logical model, not a claim about vendor microprogram details,
electrical RAM failure detection or exact diagnostic time. No fault injector,
new public API, helper, persistent state or second completion path is added.

The direct receiver checks all four DMA/IRQ masks with the selected drive
absent, no provider calls, no DRQ, correct completion IRQ and CSB read
acknowledgement. Read Buffer then observes all 512 final-model bytes through
the real transfer path. The XT board receiver independently checks ports,
IRQ5 acknowledgement, absence of DMA3 requests and unchanged medium bytes.
E4 remains a separate unresolved processor/buffer/ECC/program-memory
diagnostic; its missing physical components are not qualified by E0.

Every HDC receiver was rebuilt in the original Ninja/ccache trees. Related
tests pass x64 9/9 (5.02 s) and x86 9/9 (0.99 s). These are focused logical
and wiring regressions, not whole S4 unit/integration/artifact acceptance.
Incremental E0 production is +16/-0 (net +16); its two test files are
+57/-0 (net +57), including the pre-fix reproducer and provider query count.
Shared-six, MyNES, owner INIs/media and deployed EXEs remain unchanged.

## T01 Deadline Sweep, Reconciled Candidate

Current-source inspection after the E0 checkpoint confirms one deadline
arithmetic class still needs direct reproduction and owner-local repair:
HDC's sole `schedule_service` adds a 32-bit duration to elapsed ticks without
overflow checking. FDC similarly adds at its DMA-byte, non-DMA-byte, reset,
initial-seek and subsequent-seek deadlines. HDC already rejects backward
`advance_at`; FDC currently overwrites elapsed time even for a backward call.
Search: `rg -n 'elapsed_ticks \+|due \+|advance_at|next_due_tick'
src/core/chips/hdc/hdc.c src/core/chips/fdc8272/fdc.c`.

This is not a manual-derived physical timing value. It is the existing
absolute guest-time contract's representability boundary. An overflow can
publish an already-past deadline; merely saturating FDC to UINT64_MAX is
insufficient because `next_due_tick` currently also uses that value as its
no-event sentinel. Reconciliation must cover pending-event admission,
max-tick publication/consumption and backward advancement together, plus
the real board scheduler receiver. No production change or passing proof
is claimed for this class here. It remains T01 in the complete S4 batch,
not a new task or a qualification deferral.

## T01 Chip Deadline Boundary Checkpoint

New receivers first fail on the previous FDC/HDC implementations. Absolute
deadline additions now saturate at the representable guest-time horizon
instead of wrapping into the past. This is a representation model boundary,
not an invented physical duration or exact elapsed-time claim beyond that
horizon. FDC's one private deadline arithmetic helper serves reset, initial
and repeated seek, DMA byte (including zero-duration logical handoff) and
non-DMA byte scheduling. Existing pending conditions distinguish no event
from a legitimate UINT64_MAX deadline. No persistent flag or public API is
added. FDC now rejects backward advancement, as HDC already did; reset
continues preserving the chip's current board clock.

Direct FDC tests cover backward time and maximum-tick reset, two-step seek,
DMA and non-DMA byte eligibility, including publishing and consuming the
maximum-tick event. HDC tests cover all four personality command deadlines
and three task-file personalities' read/write next-sector deadlines. Early
advancement does not issue an IRQ; horizon consumption has its normal
completion/data phase. Existing normal-time tests remain unchanged except
for two fixture errors revealed by the stronger contract: Seek cadence now
constructs its genuinely fresh local chip rather than resetting a shared
fixture's clock backwards, and the real board DMA receiver uses an origin
relative to current elapsed time rather than jumping back to tick 100.
Original step durations, 120-tick byte gate and payload/result assertions
are preserved.

Both original Ninja trees rebuilt every related FDC/HDC receiver. Initial
22-route regressions pass 21/22 each (x64 13.27 s, x86 4.33 s); the sole
board DMA fixture failure is recorded, not erased. After the relative-origin
repair that route passes x64 1/1 (4.30 s) and x86 1/1 (3.33 s). The augmented
HDC next-sector test subsequently passes x64 1/1 (0.22 s), x86 1/1 (0.12 s).
No budget, system setting or replacement tree was used. This is not a claim
that one final 22/22 aggregate or the complete unit/integration suite ran.

The board reduction inspection shows FDC/HDC adapters forward the existing
deadline status and `board_consider_absolute` subtracts only after due>now;
it can represent a maximum-tick chip deadline. A neighbouring PIC reduction
still computes `now + device_ticks` before that subtraction, and requires
its own original clock/receiving reconciliation in the T01 board sweep.
Board scheduler runtime horizon proof remains pending. These residual
members prevent accepting the whole T01 row from chip tests alone.

## T01 Board Relative Deadline Checkpoint

PIC already supplies a relative delay. The board reducer previously converted
that delay to `now + delay` and back, overflowing near the clock horizon and
misreporting a future event as immediately due. The one private relative
minimum reducer now receives PIC and XT-keyboard delays directly; clock-domain
conversion and absolute-deadline subtraction feed that same reducer. There is
no public API, persistent state or change to physical chip timing.

The existing board timing receiver initializes a real XT PIC, configures a
five-tick IRQ1 unmask delay, and observes it at UINT64_MAX-2. A bounded old-branch
experiment with the valid fixture fails at stage 4 (the chip still reports five
ticks). Restoring the relative reducer passes x64 1/1 (1.00 s) and x86 1/1
(0.16 s), including the subsequent one-tick observation. Both targets were
rebuilt in their original Ninja/ccache trees. Earlier fixture-construction
failures are not evidence of the production defect. This proves the board
observation receiver, not complete scheduler advancement at the time horizon;
the remaining T01 scheduler/conversion sweep and full S4 qualification remain
open. No desktop, integration or artifact qualification is claimed here.

## T01 Scheduler Horizon Checkpoint

The existing scheduler receiver now injects the same-owner elapsed clock at
UINT64_MAX-5 and supplies a five-tick board deadline. It observes the exact
maximum deadline, invokes the public advancement path, and checks production
attachment delivery plus the execution-provider callback. Once at the maximum,
another relative deadline is not published as a wrapped absolute tick; an
explicit one-tick publication returns INVALID_ARGUMENT and leaves elapsed time
and provider delivery unchanged. Existing lifecycle, callback-context and
readiness assertions remain. No scheduler production change is needed for this
tested boundary. Both original targets rebuilt; x64 1/1 passes (1.37 s), x86
1/1 passes (0.15 s). This is scheduler-receiver proof using its existing copied
board probe, not a claim of an end-to-end storage command at the horizon.

Clock conversion still requires separate reconciliation: source_ticks_until
rejects overflowing delivered_ticks*denominator, while domain_advance's quotient
product and delivered-tick accumulation use unsigned arithmetic. The retained
ratio declarations and their actual consumers must be checked before treating
that representability surface as accepted. T01 and the complete S4 batch remain
open; no artifact, full-suite or desktop acceptance is claimed.

## T01 Selected Clock Ratios Checkpoint

Current immutable profile declarations select eight distinct ratios: 1:1,
1:2, 1:4, 596591:4000000, 715909:9600000, 256:125000, 64:15625 and
315:1408, all with reset phase zero. XT declares 1:4 PIT and 1:1 DMA;
Default and 5170's PC-AT descriptors and DeskPro's Model-40 configuration
supply the remaining declarations. Zero-initialized ratios resolve to 1:1.
These are existing clock facts/model inputs, not new physical-time claims.

For numerator<=denominator, quotient*numerator plus the converted remainder
cannot exceed elapsed ticks (phase carries across calls); cumulative delivered
ticks cannot exceed cumulative source ticks from reset. Scheduler publication
already refuses source-axis overflow. No production reader uses the diagnostic
delivered_ticks field. Thus the general unsigned multiplication concern is not
a confirmed wrap defect for these selected consumers. The existing rational
clock receiver now checks every selected ratio at UINT64_MAX, both in one block
and split at MAX-1 then one tick, with independently computed exact integer
quotient/remainder oracles. It preserves the existing 3:2 ordinary-time,
reset, inverse-deadline and overflow-rejection assertions. The test diff is
+36/-1, net +35; no production change. Original-tree rebuild and test pass
x64 1/1 (1.33 s), x86 1/1 (0.14 s).

This does not qualify arbitrary accelerating ratios at an unrepresentable
device-time horizon. Those ratios are not selected by these four PC Apps.
The inverse deadline function's checked rejection remains its existing
contract; storage durations and board receivers still require their full S4
disposition. This checkpoint alone does not accept the complete T01 batch.

## F02 Head Timer Owner Reconciliation

Fresh visual review of the original Intel 8272A printed 6-234 confirms HLT
delays the start of read/write after head-load assertion, HUT retains head
load after execution ends, and both scale with CLK like SRT. Nonzero HLT
units are twice the existing one-millisecond step basis; nonzero HUT units
are sixteen times that basis. The inspected page does not specify the zero
encoding. No physical zero-encoding or electrical head-load-pin claim may be
invented. The current recording-channel connection has no head-load output;
private timing state may model command eligibility and retention, not prove
an unexposed electrical pin.

Current single command execution has one owner in x86_fdc_execute. Head timing
must delay this existing command route, not create a second media execution
path. Private per-unit loaded/expiry state is necessary because commands may
select different drives during an unload interval; a head-load wait belongs to
the sole active command. Result/status, NDMA notification, DMA request, reset
and next_due_tick must all consume that same gate. Completion must start HUT
at execution end, rather than at result-byte consumption. Reset cancels the
private head timers while preserving Specify parameters. No public head API
or connection callback is needed.

Existing step validation only bounds numerator*16; HLT/HUT multipliers reach
254/240. The duration calculation therefore needs checked quotient/remainder
arithmetic before using the already repaired saturating absolute deadline.
Simply reusing `(units * step_numerator)` would introduce a new overflow.
Required tests cover nonzero HLT first-byte eligibility in DMA and NDMA,
same-unit reuse before HUT, reloading after expiry, another unit, reset,
clock scaling and maximum-tick admission/consumption, plus the actual Board
DMA receiver. Implementation and these tests remain outstanding; this is not
a completed F02 disposition.

## F02 Read/Write Head Timer Implementation Checkpoint

The sole FDC command dispatcher now waits in a private head-load phase before
Read/Write, deleted-data, Read Track and Scan execution. The phase was appended
without renumbering existing copied diagnostic values. Per-unit loaded and
unload-pending masks retain head ownership between commands; HUT begins at
execution completion, not result-byte consumption. A different unit receives
its own HLT. Reset clears private timers while preserving Specify fields.
All timers feed the existing next_due_tick and monotonic advance route. No
public API/connection field, second execution path or electrical output was
added. Zero encodings remain zero-duration logical models, not physical facts.

The private duration helper computes ceil(units*numerator/denominator) without
overflowing the intermediate product, then uses the existing saturating
absolute deadline. Tests prove fractional scaling, same-unit HUT reuse, expiry
and reload, another unit, reset, DMA/NDMA first-byte readiness and maximum-tick
load/unload. A large-ratio case whose raw multiplied numerator would overflow
still yields the exact 16-tick HLT and 15-tick HUT. Direct causes/contract/record
tests pass on both architectures; final augmented causes passes x64 1/1
(0.16 s), x86 1/1 (0.11 s).

The first 13-route FDC receiving run passed 10/13 on both architectures, with
Model40, Core topology and Core command tests retaining immediate-entry
assumptions. Their command fixtures now explicitly settle the existing future
busy deadline, stop at ready RQM/DRQ, and never service a payload byte. The
bounded wait covers four unload events plus one load event. App tests use the
Core-owned opaque-board fixture, not a private board layout. An initially
attempted private-board access failed compilation and was removed; tests run
against the old executable after that failed build are not qualification.
After successful rebuild, Model40/topology pass x64; the one remaining raw
Core read-entry assumption was repaired and that route passes x64 1/1
(3.89 s). All three repaired routes pass x86 3/3 (3.59 s). Original payload,
IRQ, reset and 120-tick byte-gate assertions remain; no budget was increased.
This is accounted focused evidence, not a final aggregate/full-suite pass.

Read ID/Format head-output applicability and original zero encodings still
need explicit source disposition. Full S4 source reconciliation, integration,
artifacts and coordinator acceptance remain outstanding. No whole-F02 or
whole-S4 closure is claimed by this checkpoint.

After the bounded fixture's final completion check was tightened, every FDC
receiver was rebuilt in both original trees and the complete 13-route focused
FDC selection passed once: x64 13/13 (10.84 s), x86 13/13 (6.38 s). Both Core
manifests, documentation governance and whitespace checks pass. This supersedes
the earlier fragmented focused accounting for this head-timer checkpoint only;
it does not substitute for the complete repository unit suite, four-profile
external integration or eight product artifact rebuilds required by S4.

## F02 Read ID Original-Source Reconciliation

The archived Intel command descriptions and NEC 765A/B data sheet do not
separately detail Read ID head settling. Further original-source research found
NEC Microcomputers Application Note 8, Richard Weiner/Gregory York, the mirror's
February 1979 preliminary copy. Its cover names Application Note 8; the dated
edition qualifier is the archive filename, not a date printed on that cover.
Acquired 2026-10-11 into the external manual archive from
`https://ftpmirror.your.org/pub/misc/bitsavers/components/nec/_appNotes/AN08_uPD765_preliminary_197902.pdf`;
SHA-256 `C1F9FAF41759BE1C6DC0DF2BD850C4F3916457EEB206322CD75AAD57027F0B47`.
No manual or derived rendering enters tracked source or product assets.

Visual review of Figure 7, printed page 12/PDF page 13, explicitly places
Read ID alongside Read Data, Read Deleted, Read Track and Scan in the same
HLT/HUT head timing diagram. Figure 9, printed page 14/PDF page 15, proves
Format asserts head load before index/write and unloads afterwards, but gives
no numeric settling interval. The latter is not proof that Format can bypass
HLT, nor proof of a precise Format HLT; that applicability remains unresolved.
This NEC source supplements, not silently replaces, the selected Intel source;
no electrical-output or physical rotational precision is claimed.

Read ID now uses the sole existing private load gate and starts HUT on its
successful result transition. Error completion already uses the common HUT
owner. The added regression submits raw neutral chip command bytes, observes
no early IRQ/DRQ, checks the exact scaled nine-tick admission, unchanged
seven-byte result, 48-tick unload retention, and reuse with renewed HUT.
Incremental production change is +2/-0; test change is +33/-0. No public API,
additional timer state, helper or media execution path is introduced.
Direct causes/contract/records pass x64 3/3 (0.93 s), x86 3/3 (0.26 s).
An initial hyphenated CTest selection matched no tests and is not evidence;
the reported runs use the actual underscore-containing registered names.
Broader receivers, complete S4 qualification and artifacts remain outstanding.

After relinking every affected FDC receiver in both original cached trees,
the complete 13-route FDC selection passes x64 13/13 (11.11 s) and x86 13/13
(6.53 s). Both Core manifests and whitespace checks pass. This is Read ID's
focused receiving proof, not the full S4 unit/integration/artifact gate.

## F02 Format Head Admission Reconciliation

Reconciliation of the previously unresolved Format question uses the combined
original contract, not a number measured from firmware. Intel printed 6-232
defines Format as writing IDs/data after index; printed 6-234 defines HLT
between head-load assertion and read/write operation, and HUT after execution.
NEC AN08 Figure 9 shows Format's head-load assertion before index/write and
deassertion after writing. Applying the existing HLT gate to that write
operation is an inference from these original definitions, not a separately
annotated Format-specific duration in Figure 9. It removes an unsupported
exception from the existing head admission owner. Actual index/rotation,
electrical head output and zero encoding retain the previously stated model
boundaries; no precise whole-track physical duration is claimed.

Format now enters the same load gate as every retained media read/write
command. Its existing normal, TC and provider-error completion already uses
the sole HUT owner, so no second completion path is necessary. The incremental
production change is +1/-0; the existing causes test adds +33/-0. DMA and
NDMA raw-command rows prove no early IRQ/DRQ or media change, nine-tick HLT,
correct first service polarity, actual four-byte ID transfer and sector fill,
R+1 result, and 48-tick HUT at execution completion. Direct chip tests pass
x64 3/3 (0.99 s), x86 3/3 (0.28 s). No API, helper or state was added by this
increment. Whole F02/S4 eligibility still requires the remaining batch proof.

All FDC receivers were subsequently relinked in the original trees; the
13-route selection passes x64 13/13 (11.38 s), x86 13/13 (6.09 s). Core source
and test manifests and whitespace checks pass. No artifact or complete S4
qualification claim follows from these focused runs.

## H07 Initialize Characteristics Original-Source Design Reconciliation

Fresh visual review of the already archived S1410 Rev C1 original, printed
27--28/PDF 30--31, resolves the previously missing byte-order and selection
facts. Two-byte fields are most-significant-byte first. The eight-byte packet
is cylinder count, head count, reduced-current cylinder, precompensation
cylinder and maximum ECC burst length. DCB bytes 1--5 are ignored; both drives
must share the same characteristics. IBM Options/Adapters printed adapter
12/PDF 313 independently marks all five DCB bytes ignored and lists the same
eight parameters. This is controller-global initialization, unlike the WD
per-drive parameter command already repaired. A per-drive flag derived from
DCB byte 1 would be a new source conflict, not a reusable simplification.

The current receive route already owns the eight-byte pending packet and
waits for it in INITIALIZE before scheduling command completion. The actual
completion still falls through to drive-not-ready and none of the address
receivers use the programmed cylinder/head counts. The next repair must commit
those controller-owned logical limits only after the complete packet reaches
the existing completion owner, without querying a selected drive or changing
the immutable medium/configuration owner. Partial receive and reset must not
publish partially programmed limits. Both units and multi-sector head/cylinder
advancement plus E3 enumeration must consume the same programmed limits.

Physical flat-record addresses must continue to use the selected physical
medium's head/track layout; reducing a controller head limit does not rewrite
the image geometry. This separates command-visible admission/advancement from
physical provider offsets, rather than adding another medium or pretending
that parameter programming resizes a disk. Reduced current, precompensation
and ECC parameters do not prove electrical/ECC behavior through the existing
512-byte provider; their retained raw packet and explicit physical boundary
must remain documented. S1410's 153/4 default is not blindly substituted for
the selected IBM adapter/profile's firmware/model defaults. The selected reset
default and unsupported/invalid parameter disposition still need final
reconciliation before this design becomes implementation acceptance.

## H07 Initialize Implementation And Receiving Checkpoint

The existing completion owner now atomically commits the complete MSB-first
cylinder/head packet, with DCB selection ignored and no drive query. Pending
bytes do not alter accepted limits. One private address-geometry selector
supplies both units' sector admission, multi-sector progression and E3 track
enumeration; physical offsets still use the configured medium head layout.
Reset clears the programmed counts through the existing reset owner and
returns to the selected configuration's pre-initialization model. No public
interface, second media owner or additional timer is introduced.

The direct chip regression programs 257 cylinders and one head on a physical
300-cylinder/two-head medium. Both units read physical offsets 1021, 1024 and
1025 across a cylinder boundary, reject the first address past the programmed
limit after the accepted prefix, and retain accepted limits after an invalid
replacement. Seven pending bytes and a fully received but uncompleted packet
leave the limits unchanged; reset cancels an incomplete replacement. The real
XT port receiver proves completion IRQ acknowledgement without DMA or media
access, E3 enumeration restricted by the programmed limits, and reset returning
to the original selected geometry. No physical master is written.

After rebuilding all nine HDC receivers in the original cached trees, the
complete focused HDC selection passes x64 9/9 (4.60 s) and x86 9/9 (0.98 s).
These are focused chip/board/App regressions only, not complete S4 eligibility
or artifact qualification. No S4 implementation P has been formed or pushed.

Similar-path review confirms Read/Write/Verify/Seek/E3 use the one address
selector, buffer commands do not reinterpret media geometry, and reset has
one programmed-state clearing path. Original-source review still exposes a
parameter-domain prerequisite: S1410 printed page 28 gives four head-count
bits, while the current logical model permits up to 32 and the IBM command
table merely specifies one byte. Reconcile the selected IBM adapter's actual
domain before accepting this row; passing tests do not resolve that conflict.
Zero/out-of-domain error 20h, pre-initialization selected geometry, electrical
write-current/precompensation, ECC and physical command duration remain explicit
model boundaries rather than newly proven hardware semantics.

The IBM original also explicitly assigns five bits to the command head number
(printed adapter page 7), and its BIOS listing forwards the parameter-table
head byte without a mask while describing supported BIOS heads 0--7. Thus the
S1410 four-bit parameter drawing cannot justify silently masking the IBM
adapter's byte, nor does the five-bit address field prove a 32-head Initialize
count. This remains an exact selected-controller-domain reconciliation, not a
reason to introduce a firmware-specific exception or replace the immutable
medium geometry. The current model domain is not marked source-qualified.

Core source/test manifest verification, documentation governance and
`git diff --check` pass after this checkpoint. Against the accepted S3 HEAD,
the four HDC paths currently count +350/-140 (`hdc.c`), +4/-2 (`hdc.h`),
+727/-13 (chip contract test) and +99/-5 (XT wiring test). These are whole-S4
accumulated changes, not the isolated Initialize increment; no claim of a
completed full-suite, external integration, desktop or artifact run is made.

### FDC TC Matrix Receiving Checkpoint (2026-10-11)

Intel Table 8 (printed 6-231) is now checked with literal next-ID results for
middle/last sector, MT clear/set and both actual heads. Read, Read Deleted,
Write and Write Deleted cross DMA/NDMA, full/partial sector and zero/positive
cadence: 256 cases. ST0 retains the actual terminal head independently of the
returned next CHRN; partial writes verify the accepted prefix and zero tail.

The matrix first rejected eager NDMA preparation of the next head before TC.
Preparation now belongs to next-byte admission, as in the DMA path. It then
rejected partial NDMA writes stuck in WRITE_TAIL: the existing internal byte
scheduler now admits that phase regardless of host transfer mode, without
asserting DRQ. No new state, API or scheduler was added for these two repairs.

The rebuilt complete FDC selection passes x64 13/13 (14.07 s) and x86 13/13
(7.72 s), in the original cached build trees. Whole-S4 changes against accepted
S3 in these paths are +244/-60 for chip fdc.c and +298/-7 for the records test;
these counts are not the isolated TC increment. Desktop and full-batch artifact
qualification are not claimed by this focused checkpoint.

The owner approved four appended FM/MFM read/scan and write service-duration
fields in x86_fdc_timing on 2026-10-11. This supersedes the pending approval
in the earlier interface-decision checkpoint, not its implementation status.
Overrun deadlines still require implementation and receiving tests; byte
cadence must remain distinct, with board-owned clock conversion and chip-owned
expiry through the existing scheduler. S4 and T555 remain open.

### FDC Service-Limit Implementation Checkpoint (2026-10-11)

The four approved fields are implemented in the existing timing value. Chip
execution owns one pending flag and one deadline; both DMA and NDMA start it
at byte admission, data service cancels it, and TC/reset/normal/error completion
remove it. MSR polling does not renew it. Expiry uses ST1 OR (10h), abnormal
ST0, the existing completion/IRQ path and the existing next-due reducer.
No observer, function, second scheduler or Shared change was added.

Fresh original Intel Read/Write/Scan review (printed 6-231--6-233) confirms
27/13-us Read/Scan and 31/15-us Write limits. Read Track/Read Deleted use the
existing read execution; Write Deleted uses write execution. Format's existing
host-header write execution consumes the write budget as a logical recording-
channel model; it does not claim measured index/gap timing. Deadline equality
is the first unavailable discrete guest tick; zero service inputs explicitly
retain the nonphysical logical model rather than proving absence of overrun.

Board-Base now owns one selected recording-rate lookup for both track sampling
and timing. It scales the datasheet limits at 500/300/250 kbit/s directly to
guest ticks with one ceiling, not a prior whole-microsecond ceiling. Byte
cadence is independently derived from eight recorded data bits (FM 32 us/MFM
16 us at the 500-kbit/s reference), replacing the mistaken 31/15 service-limit
cadence and the old zero-cadence 250-kbit/s path. Existing byte-gate timestamps
retain the admitted byte's axis, so late but valid acknowledgement no longer
delays the following byte. No additional persistent timestamp was needed.

The chip's 144-case matrix covers nine retained transfer command forms across
encoding/host mode, no service, one timely service followed by expiry, TC and
reset. It independently checks the deadlines, OR/CHRN result, accepted prefix,
unchanged unaccepted media, terminal count and request cancellation. The real
Board port/IRQ/DMA receiver checks 36 Read/Write/Scan cases against literal
500/300/250-kbit/s duration values; timely reads additionally verify independent
byte cadence through NDMA ports or the real DMA2 provider, never a chip bypass.

The first complete receiving run exposed three old fixtures: Model40 still
used 15 us; the topology fixture used a 128-tick gate at 250 kbit/s, whose
8-MHz MFM gate is 256 ticks; and the media fixture sent consecutive NDMA data
without waiting for RQM. They now use the independent literal gates or the
existing due interface for payload admission. A temporary first-failure probe
localized one remaining unbraced scan loop and was removed after repair.
There is no retained diagnostic instrumentation or relaxed production check.

Original cached-tree rebuilt FDC selections pass x64 13/13 (13.51 s) and x86
13/13 (7.19 s), including the direct and receiving additions. This does not
qualify the remaining HDC source prerequisites, all 22 S4 rows, complete units,
four-App integration, desktop interaction or refreshed eight artifacts. S4
and T555 remain open; no partial implementation P has been committed/pushed.

At this checkpoint the accumulated S4 FDC source slice is +348/-80 (net +268):
Board-Base fdc.c +32/-14, chip fdc.c +302/-65, private fdc.h +8/-1 and timing
interface +6/-0. Its eight affected test/fixture paths total +958/-63 (net
+895). These counts include earlier TC/head/command repairs and are not an
isolated overrun diff. Core source/test manifests, documentation governance
and whitespace checks pass; Shared-six, MyNES and assets remain unchanged.

Further F11/F12 current-path review must qualify disable/re-enable during a
pending byte, not only at reset: start-transfer tests service_enabled, while
the due-DMA publisher currently asserts its request without that test, and
set_service_enabled only stores the flag. Source/mask-owner reconciliation
and actual DOR/DMA/IRQ receiving tests are still required before accepting
these rows. The green FDC selection above does not prove this missing case.

### FDC DOR Output-Gate Receiving Checkpoint (2026-10-11)

IBM Options and Adapters Volume 2, April 1984, Diskette Adapter printed
page 17 (PDF page 286), identifies DOR bit 3 as Enable INT and DMA Requests,
separate from controller reset bit 2. The controller has no corresponding
DOR pin. Board-Base therefore gates its downstream PIC/DMA signals; the chip
always retains native IRQ/DRQ causes. The former chip service_enabled field
and setter are removed rather than mirroring a board mask inside the chip.
Existing refresh republishes native levels after DOR writes. Its PIC source
assert is idempotent, and the DMA request operation sets the existing request
bit, without adding a second cause, timer, pending state or public function.

The existing 36-case Board service-limit matrix now additionally checks both
host modes at all three recording rates and both encodings: disabling DOR
preserves the native request and original service deadline while removing
downstream signals; enabling restores the pending request; expiry still
occurs while masked and its result IRQ is delivered on re-enable. Timely
Read cases also disable through the following byte gate and restore its
pending request, then reset and confirm native/downstream outputs are clear.
Tests configure actual DMA2 before testing receiver eligibility; they do not
confuse an unconfigured/masked DMA channel with absence of a native DRQ.
Overrun IRQ assertions run after the existing pending-complete phase, not
before that phase has published the result. Temporary localization output
was removed; no production completion algorithm was changed to fit it.

Rebuilt original-tree FDC selections pass x64 13/13 (15.04 s) and x86 13/13
(7.36 s). Core source/test manifests and whitespace checks pass. Shared-six,
MyNES, artifacts and owner configuration remain unchanged. This supersedes
the preceding DOR candidate diagnosis, but does not qualify remaining HDC
prerequisites, complete S4, full units/integration or desktop interaction.
No partial implementation P has been committed or pushed.

### H08 Existing Completion IRQ Mask Checkpoint (2026-10-11)

Current-source inspection found that Xebec mask writes refreshed DRQ only;
an already asserted completion IRQ remained visible after its enable bit
was cleared. IBM Options and Adapters Volume 2, April 1984, printed Fixed
Disk Adapter 14--16 describes the mask port and enabled IRQ5, and printed
19/PDF 320 shows the separate mask-controlled IRQ5 output buffer. Fresh
visual review of that sheet confirms this output-gate boundary; it does not
prove every asynchronous flip-flop transition or a new masked-completion
notification policy.

The existing mask-write owner now calls the existing IRQ sync. The existing
visible-pending query applies the Xebec IRQ-enable bit, as it already applies
task-file nIEN. Existing pending storage, response acknowledgement and command
completion admission remain unchanged. No state, helper, public interface or
new response route is added. In particular, this fix does not change whether
a command completed while interrupts were disabled creates a pending cause;
that separate source/latch reconciliation remains H08, not implied by analogy
with the FDC's independent DOR wiring.

The direct command matrix first fails on the preceding implementation. It
now checks masking/restoring an existing completion, no additional media
operation, acknowledgement followed by no stale IRQ, and masked acknowledgement
of Request Sense's final CSB followed by no resurrection. Read/Write and
partial-TC cases additionally toggle IRQ enable during active DMA without
dropping DRQ or repeating accepted data. The XT receiver exercises 323h,
actual PIC IRQ5 and 320h acknowledgement with RAM Diagnostic and unchanged
media. This qualifies the observable existing-cause mask path, not an exact
electrical latch simulation.

All HDC receivers were rebuilt on the original Ninja/ccache trees; the
related group passes x64 9/9 (4.47 s), x86 9/9 (0.87 s). The final additional
DMA-isolation assertions pass the rebuilt contract route x64 1/1 (0.21 s),
x86 1/1 (0.13 s). This increment is production +3/-0 and tests +28/-0,
excluding manifests/evidence. It is not complete S4 unit/integration/artifact
qualification. Shared-six, MyNES and owner configuration/media remain untouched.

Fresh S1410 printed 29 visual review independently confirms E4 checks the
processor, buffers, ECC circuit and program-memory checksum without accessing
the drive. The current generic drive-not-ready fallback cannot satisfy that
command. E0's logical buffer pattern check does not prove these additional
components; E4 and the selected Initialize head-count domain remain unresolved
and must not be accepted merely because the HDC group is green.

## H06 Additional Original WD PIO Authority (2026-10-11)

The Western Digital 1986 Storage Management Products Handbook is now archived
externally as `manuals-nxvm/hdc/western-digital-storage-management-products-handbook-1986.pdf`.
It was acquired from the original vendor-document archive mirror
`https://bitsavers.trailing-edge.com/components/westernDigital/_dataBooks/1986_Storage_Management_Products_Handbook.pdf`
on 2026-10-11; SHA-256 is
`72382885ABFDE5CDEE875384AED7CA97C17112DB047D7792D3A4B4D3ED0C895F`.
The ordinary web reader returned 403; the research download succeeded and the
external copy's hash was independently verified. No PDF/OCR/vendor source is
committed or enters an executable.

Fresh visual review of printed 6-130/PDF 489, WD1002-WAH, establishes IRQ on
each available Read sector but none after the final host block. Write IRQs
announce subsequent buffers and final completion, excluding the first buffer.
Printed 6-175/PDF 534 identifies WD1003-WAH as a cost-reduced WD1002-WAH, but
also identifies its distinct WD2010A-05 implementation. The exact WD1002 text
therefore strengthens the existing AT PIO cross-check; it does not by itself
identify the custom Compaq 40-MB controller or prove its microprogram.

Compaq's own printed 9-3/PDF 118 says its fixed-disk systems share addresses,
registers and command structures. Printed 9-13/PDF 128 states Read Long's
data is returned after completion signalled by an interrupt. Its normal
Read/Write paragraphs still do not enumerate first-buffer/final-buffer IRQ
edges. The current Compaq-only branches in begin_write and next_read_sector
remain candidates, not accepted hardware distinctions. Do not change them
solely because another AT controller documents a different sequence. The
remaining decision needs selected-controller identity or an independently
validated interface rule; no notification branch was changed this turn.

The vendor's printed 6-131/PDF 490 additionally documents Format's 512-byte
interleave table, bad-block marking, zeroed data fields and diagnostic register
effects. Those are research leads for the unresolved H03/H05 function members,
not a licence to substitute short Read/Write or another firmware's diagnostics.
The current sector callback supplies 512-byte records but no raw ECC, ID or
bad-block metadata; Long/ECC/physical Format cannot be qualified by those records.

## Selected Xebec Domain Reinspection

Current App/Board construction was rechecked, not inferred from old paths:
My5160's `profiles/xt_5160_268.c` selects Xebec type 2; Board-Base topology
validation requires exactly 615 cylinders, four heads, 17 sectors/track,
512-byte sectors and 41,820 records. These selected values fit both the S1410
four-bit head-count drawing and the IBM byte/DCB representation. This proves
the configured values do not exercise the disputed upper domain; it does not
prove Initialize may accept a count of 32, or dispose a guest's programming
of that value. The programmed-count admission prerequisite therefore remains.

The E4 audit also confirms the existing HDC connection has only medium
query/read/write and IRQ/DRQ outputs, not controller program ROM, processor
or ECC diagnostics. E0 buffer testing must not be reused as proof of these
missing E4 facts. An always-success E4 branch or invented diagnostic code
would conceal the confirmed function gap; no such branch was added.

This is source/consumer reconciliation progress, not dynamic qualification.
There is no additional production/test/ABI/artifact change in this checkpoint.
All unresolved members remain in the S4 completion predicate.

## H07 Selected-Firmware Boundary And Missing Record Facts (2026-10-11)

Fresh visual review covers S1410 Rev C1 printed 19--22 and 27--30;
IBM April 1984 Fixed Disk Adapter printed 1, 4--13 was independently
re-extracted and compared with the current DCB decoder. This comparison
changes the next implementation decision: S1410's generic DCB uses a
three-byte logical address (printed 20--21), whereas the selected IBM
adapter explicitly supplies head/cylinder/sector fields (printed 7--8).
The current CHS decoder follows IBM, not the generic S1410 layout. S1410
must therefore corroborate function descriptions, not override the selected
IBM command encoding or determine an undocumented Initialize upper limit.
The four-bit H drawing in S1410 printed 28 does not independently authorize
changing IBM's current head-count admission. The configured four-head type
2 remains source-compatible, but the guest-programmable upper limit is
still unresolved; it is not qualified by the selected default.

The still-defined commands were separated against the selected IBM summary:

| Command | Required fact absent from the current implementation | Existing-path consequence |
| --- | --- | --- |
| 04 Format Drive | Track formatting, interleave 1--16 and ID/ECC creation over the addressed drive range | Repeating 512-byte zero writes would implement data clearing only, not the entire command. |
| 06 Format Track | Track ID order/interleave and regenerated ECC | The existing medium fill callback cannot express track IDs or interleave. |
| 07 Format Bad Track | A bad-track flag visible to later reads/verifies, with sense 19h | Neither HDC records nor HDD medium metadata contain that flag; a successful no-op or zero fill is incorrect. |
| 0D Read ECC Burst Length | One host byte, meaningful after a correctable data error (S1410 printed 28) | No current record result represents corrected data or a burst length. Zero must not be presented as proof of an implemented correction path. |
| E4 Internal Diagnostics | Controller buffer, program-memory checksum and ECC-generator tests; no drive access | Reusing drive readiness or E0 alone does not test the missing controller facts. IBM explicitly defines diagnostic senses 30h, 31h and 32h. |
| E5 Read Long | Exactly 512 data bytes plus four stored ECC bytes per sector, without ordinary correction | The existing read callback supplies only 512 data bytes; padding four zeros or aliasing ordinary Read loses the diagnostic data. |
| E6 Write Long | Accept and retain the host's four ECC bytes in addition to 512 data bytes | The existing write callback would discard them; computing fresh ECC would defeat the command's purpose. |

All seven currently reach the blanket sense-04 completion in
`core/chips/hdc/hdc.c`. This is a confirmed function gap, not a legitimate
absent-drive result for every command: E4 does not access a drive at all.
No branch or test was changed to make this blanket completion look qualified.

The smallest honest full repair needs a record model decision before ABI
work: track ID/interleave/bad-track and stored ECC must have one medium
owner and survive later Read/Verify/Long operations; controller self-test
facts belong to the chip, not a filesystem callback. Merely adding an HDC
format callback cannot solve E5/E6/0D, and a generic diagnostic callback
with a fixed-success implementation cannot solve E4. Conversely, these
facts do not require changing Shared Lib's three file access modes or
introducing a second storage backend. Exact ECC polynomial/bit order and
selected controller program-memory behavior still require original-source
reconciliation. No new public interface, sidecar, raw image format or
diagnostic success constant is authorized by this evidence checkpoint.

The further S1410 printed 31--32 review exposes two details that must not be
lost during that design. Sector format defines a separate identifier ECC field;
E3 verifies sector zero on each track, rather than making the data-read loop
proof of a physical ID diagnostic. The later source recheck below distinguishes
that additional ID requirement from an unsupported ID-only restriction.
Also, S1410 Format writes 6Ch in the data fields (printed 32), unlike the
zero-fill rule in the separately reviewed WD1002-WAH manual. A common
zero-fill implementation across these personalities would conflate different
controllers. This S1410 detail remains a corroborating lead until selected
IBM format-fill behavior is independently established; it is not silently
applied to IBM firmware. Format Bad Track writes flags into every identifier
and later accesses report 19h; host software, not the controller, owns spare
track assignment. No automatic bad-track remapping belongs in this repair.

This is a source-boundary and command-contract audit checkpoint only.
Focused/full runtime qualification and artifact publication are not claimed.

## M01 Installed Record-Metadata Lifetime Receiver (2026-10-11)

The existing FDD implementation was inspected before considering any HDD
metadata extension. Its Deleted-Data flags belong to the installed medium,
not the FDC: `vm_machine_fdd_reset()` rebuilds geometry but preserves flags;
rejected insertion leaves the installed medium and flags unchanged; successful
insertion publishes a fresh flag allocation. HDD currently has no analogous
ECC, ID or bad-sector facts. This is an existing flat-image model boundary,
not evidence that the file contains physical record metadata.

The existing `machine/media/provider_smoke.c` now exercises these FDD facts
through the actual media registry and Storage-backed provider. It marks sector
zero deleted, resets the FDD, checks unchanged generation/mark/data, attempts
an invalid-size replacement and checks the old mark, then reinstalls the valid
image and checks the new generation and normal Data mark. Only the owning
Machine-media test includes its private FDD declaration to invoke the already
existing reset; no cross-component public test API is added.

Actual test diff: +38/-1, net +37, including the private owning-header change;
one local test helper expresses this complete lifetime sequence. Production,
public ABI, persistent state, build registration and Shared/MyNES are unchanged.
The corresponding Core test manifest entry was updated and verified.

Both original Ninja/ccache trees rebuilt `vm-media-provider-smoke` successfully.
The four related media-provider/direct-readonly/close-failure unit receivers
pass 4/4 on x64 (0.23 s wall time) and 4/4 on x86 (0.22 s). These are focused
repository-only results, not the complete S4 suite, desktop qualification,
four-App integration or fresh executable publication. The HDC record model
decision and the remaining finite storage batch are still required.

## H03/H07 Record Boundary Design For Owner Review (2026-10-11)

The owner approved this record-interface and installed-lease lifetime boundary
on 2026-10-11 after the explicit blocker report. This approval does not resolve
the original-source prerequisites below. Fresh code inspection confirms that E4 still reaches
the generic sense-04 completion, and the HDC connection still transfers
exactly 512 bytes. E4 cannot honestly be repaired by changing that completion
to success: E0 covers buffer RAM only, not program-memory checksum or the ECC
generator. Likewise, Read/Write Long cannot be repaired by padding or ignoring
the additional four bytes. These are confirmed gaps, not passing tests.

### Minimum Owners And Data Flow

- HDC owns command decoding, transfer phases, ECC generation/checking once its
  selected original algorithm is established, correction result and last
  corrected burst length. It retains the existing scheduler and IRQ/DRQ paths.
- Installed HDD medium owns record metadata: supplied ECC, bad-track flags and
  formatted track ID order. The existing Storage medium remains the only owner
  of payload bytes. No second image, sidecar, file backend or disk manager is
  proposed. Metadata must not be cached in HDC against geometry alone.
- Board-Base adapts chip record requests to its existing media registry. It
  must not calculate ECC, invent successful diagnostics or clear metadata on
  controller reset. Machine-media owns allocation, replacement and destruction.
- Program-memory checksum facts belong to the selected controller, not to the
  host BIOS ROM or medium. The selected profile asset inventory inspected this
  turn contains no separately identified Xebec controller program-ROM input;
  this is not a claim about every owner-managed archive directory.

### Required Interface Decision

The four FDC timing fields and the HDC record extension have separate owner
approvals; the latter is now authorized by the subsequent explicit approval.
The proposed HDC seam is one record contract for ordinary and Long transfers:
512 payload bytes, four stored ECC bytes and an explicit indication of whether
stored ECC is available, plus record facts needed for bad-track/ID validation.
Ordinary writes generate ECC; Long writes retain the four bytes supplied by the
guest. Ordinary reads validate/correct; Long reads return stored bytes without
ordinary correction. The physical byte/word bus widths remain personality
specific, including Compaq's 16-bit payload and 8-bit ECC phase.

This requires changing the existing HDC read/write callback contract and the
Board-Base media provider record contract; it is not achievable solely inside
the current 512-byte functions. Track format also needs a provider operation
with explicit addressed range, ID order/interleave and bad-track disposition.
Exact declarations should be reviewed together after the source prerequisites
below, rather than appending separate Long/format/diagnostic shortcuts now.
No declarations, enum values or callback fields were changed by the design
checkpoint. The subsequent approval allows implementation within this boundary.

### Lifetime And Flat-Image Limits Requiring Approval

Record metadata follows the installed medium, like the existing FDD marks:
cold/controller reset preserves it; failed replacement preserves the old
medium and metadata; successful replacement installs new metadata; eject and
destruction release it. Readonly rejects destructive record operations before
either payload or metadata changes. Failed writes must not publish replacement
ECC/ID facts. Direct and Overlay use the same owner and record operation path.

A flat 512-byte image cannot persist supplied ECC, bad-track flags or ID order
across eject/reopen without a separate representation. Keeping them only for
the installed lease is therefore the explicitly owner-approved fidelity boundary,
not a claim to full physical disk persistence. A sidecar/raw-image
representation is not authorized by S4 and is not silently proposed as a fix.

### Original-Source Prerequisites And Qualification

The selected IBM/Xebec ECC polynomial, bit order and correction rules remain
unresolved. WD1002's separately documented polynomial is not evidence for the
IBM/Xebec 11-bit burst contract. A generic CRC32 or four zero bytes is not an
acceptable substitute. Selected Format fill and controller program-checksum
behavior also need authority before implementation; the generic S1410 facts
cannot override selected IBM firmware. Approval of a record interface alone
would not resolve these missing hardware facts.

After those decisions, direct and Board/actual-media receivers must cover
512+4 transfer length, guest ECC round-trip, invalid ECC ordinary-read failure
versus Long data delivery, bad-track Read/Verify behavior, Format ID ordering,
reset/replacement lifetime, readonly rejection and partial-write errors. E4
must run with no medium and prove each modeled diagnostic owner independently;
it must not invoke query/read/write. Current E3 logical accessibility remains
insufficient proof of identifier-ECC diagnosis.

No production/test edits or runtime qualification occurred in this checkpoint.
The next action is owner review of this necessary interface/model boundary and
continued primary-source reconciliation, not another constant-success command
or a claim that the full S4 batch is ready for publication.

## H08 Masked Ending-Status Receiving Checkpoint (2026-10-11)

The remaining mask-at-completion question was rechecked against the original
IBM April 1984 Fixed Disk Adapter printed 14--16 and complete schematic sheet
1, printed 19/PDF 320, including the enlarged IRQ gate region. The asynchronous
LS74 preset and independently gated IRQ5 output distinguish an outstanding
ending-status condition from the visible IRQ level. Re-enabling with ending
status still available can present that condition; reading it while masked
acknowledges it and must not resurrect a request on later enable. This is the
register-level inference from the circuit, not a claim to reproduce every
internal gate delay or to simulate the LS74's physical state bit.

Current software lost the condition when completion happened while masked:
both normal response publication and the Request Sense ending-status boundary
called raise_irq only if the mask was enabled at that instant. The existing
mask-write sync could then only restore requests created before masking.
Both call sites now retain the outstanding condition using the existing
irq_pending owner; the existing visible-pending query applies the mask, and
the existing data read/reset clears the condition. No extra latch, scheduler,
callback, helper or public interface was introduced. This supersedes the
unresolved completion-creation note in the earlier H08 checkpoint, not its
accepted output-mask/acknowledgement behavior.

The RAM diagnostic receiver now completes under each of the four mask patterns,
enables while status remains unread, masks again and reads status, then confirms
no stale request after enable. Request Sense independently reads four sense
bytes while masked, enables for its still-unread ending status, acknowledges
that status while masked and again checks no resurrection. All cases preserve
DMA and media-query/read/write expectations. The actual Board receiver now
starts its RAM diagnostic masked and proves the 323h enable-to-PIC IRQ5 route;
it no longer proves only the easier already-enabled-at-completion case.

This checkpoint changes production +2/-5 (net -3), the chip receiver +28/-1
(net +27) and the Board receiver +1/-0 (net +1), plus three corresponding
manifest hash replacements. The first build request used an incorrect target
name and Ninja rejected it before compilation; the actual registered targets
were then inspected and used in the unchanged original build directories.
Both final HDC groups were rebuilt and pass 9/9 once: x64 3.91 s, x86 0.54 s.
Core source/test manifests, corpus boundaries and whitespace checks pass.

ECC/Long/Format and controller diagnostic prerequisites remain unresolved;
none are qualified by this IRQ repair. Complete S4 units, four-App integration,
desktop acceptance, eight updated EXEs and complete-P publication remain due.

## Coordinator Remaining-Batch Reconciliation (2026-10-11)

This table reconciles the complete frozen batch after the preceding checkpoints.
It does not replace the original source/receiver detail or mark any row accepted.
"Implemented/direct proof" means the identified repair and its focused receiver
exist; final batch tests, four-App integration and publication are still due.

| Row | Current disposition | Remaining acceptance work |
| --- | --- | --- |
| F01 | Full-opcode admission and fifteen command grammars implemented/direct proof | Final storage and cross-profile qualification |
| F02 | Nonzero head timers and clock conversion implemented/direct proof; explicit zero/electrical model boundaries retained | Final qualification; no claim of missing physical head-load pins |
| F03 | Sampled ST3/head/unit retained with direct receivers | Final qualification |
| F04 | Per-unit seek/SIS/recalibrate retained, deadline/cause receivers extended | Final qualification |
| F05 | MT payload, TC/EOT and actual terminal head repaired with direct matrices | Final qualification |
| F06 | Drive/DMA/full-track restrictions repaired; flat 512-byte/no raw-rotation or CRC boundary retained | Final qualification within that declared boundary |
| F07 | First logical ID repaired; no physical rotational-ID claim | Final qualification |
| F08 | R+1, partial IDs, protection/provider failure and mark/fill route covered | Final qualification |
| F09 | FF comparison and MT/SK/Scan termination repaired/direct proof | Final qualification |
| F10 | NDMA IRQ/MSR/ack and distinct cadence/service limits repaired; approved four-field input in place | Final qualification |
| F11 | Specify-preserving reset, READY admission and native cause versus DOR output gating repaired/direct proof | Final qualification |
| F12 | One DOR/DIR/port/DMA2/IRQ6/provider path retained; Board receiving proof extended; current four-App integration passes 32/32 per width | Qualification must remain current after any remaining HDC repairs; no physical CRC/rotation claim |
| H01 | Selected ATA PIO variants/Verify/count/CHS/LBA/Identify repaired/direct proof | Final qualification; not full ATA-3 capability |
| H02 | ATA initial-write/final-read IRQ, status/alternate ack and reset/nIEN covered | Final qualification |
| H03 | IBM parameters/Verify/step consumption, logical Format and supplied-ECC Long transport repaired/direct proof | Selected ECC generation/check/correction and complete diagnostic-register disposition; final qualification |
| H04 | IBM reset/nIEN/head extension and Board IRQ14/control/step route covered | Final qualification |
| H05 | Compaq per-drive parameters/Verify, 40-MB exclusions, logical Format and supplied-ECC Long transport covered | Selected two-controller identity/notification reconciliation noted below; ECC and final qualification |
| H06 | Active-low address, shared FDC port and original step-code conversion repaired | First-write/final-read IRQ authority and admissible cross-selection behavior remain unresolved |
| H07 | Logical ready/recalibrate/verify/seek/Initialize/buffer, Format/Bad Track, supplied-ECC Long transport and healthy-controller L2 E4 repaired | ECC generation/check/correction/Burst Length; E3 ID-ECC versus logical access; selected Initialize upper domain |
| H08 | Select/reset/DMA3/TC and masked ending-status/PIC IRQ5 receiving paths covered | New record-dependent commands must use these same paths after their prerequisite is resolved |
| M01 | Sole lease/modes/replace/remove failure behavior retained; FDD marks and HDD supplied-ECC/track-ID/bad-track lifetime and failure boundaries tested | Final qualification; no second backend |
| T01 | Checked deadlines, monotonic epochs and selected clock conversions repaired/direct proof | Final qualification while retaining explicit non-physical command/mechanical timing origins |

### Compaq Controller Identity: Confirmed Topology, Unresolved Observable Gap

Fresh complete-page review of Compaq volume 2 printed 9-19--9-21
(PDF 134--136), particularly printed 9-20, establishes that each 40-MB
drive assembly contains its own controller; two controllers share I/O
addresses and are jumpered to respond to drive C or D. This strengthens
the earlier reason not to identify this system solely from WD1002/WD1003
command similarity. The current Board owner contains one HDC chip instance;
that chip has two parameter sets but one task file, data buffer, phase and
deadline. Existing dual-drive coverage proves sequential selected transfers
and geometry, not simultaneous independent controller execution.

The complete programming pages 9-7--9-8 also say other status/register facts
may be invalid while Busy and require checking DRQ before a data transfer.
Therefore the topology alone does not prove a valid host may switch units
in the middle of a DRQ block or overwrite a busy task file. Such an operation
must not become a purported hardware oracle merely because it exposes shared
software storage. The Compaq-only task-file-writable exception and retained
first/final IRQ branches remain explicit source prerequisites, not accepted
differences. No speculative second controller, state bank or queue was added.

The actionable missing decisions remain concentrated in H03/H05/H06/H07:
record interface/model authorization, exact selected ECC/diagnostic authority
and admissible Compaq notification/selection behavior. Final verification and
artifacts cannot dispose these source/function gaps. This reconciliation makes
the next work finite; it does not postpone it to S5/S6 or close S4.

### Record Outcome Must Preserve Data Validity Separately From ECC Status

Compaq printed 9-12/PDF 127 explicitly retains the flawed sector payload on
an uncorrectable data error, stops a multisector read at that sector and leaves
registers identifying the error/address. Printed 9-8 separately says a corrected
data error does not terminate a multisector read. The current record_result
has OK/absent/range/protected/generic failure only; load_sector routes non-OK
through fail, without presenting a payload phase. Generic storage failure is
not evidence that 512 meaningful corrupted bytes were returned.

The proposed record contract therefore also needs to distinguish payload
availability from corrected/uncorrectable ECC outcome. The sole HDC phase
owner must deliver the source-required flawed data and terminal error without
requesting the next sector; corrected data continues the existing count/CHS
path and records the burst length for the Xebec ECC query where applicable.
Long reads bypass ordinary correction but still require complete record bytes.
An I/O failure with no valid payload must not expose stale buffer contents.
Tests must inject all three outcomes independently, not conflate a read callback
failure with a physical ECC failure. This is part of the same pending interface
review, not an additional status side channel or a second reader path.

## Approved Record Seam Implementation Plan (2026-10-11)

Fresh inspection after owner approval confirms the actual receiving path:
`chips/hdc` connection -> `board-base/hdc` -> existing media registry ->
`machine/media/hdd` -> Storage lease. FDD also implements the registry provider;
it must keep its byte/address-mark behavior and must not acquire HDD ECC policy.
Neither Shared-six nor MyNES needs a change for this seam.

The approved replacement uses one copied HDD record value containing payload,
four ECC bytes and explicit ECC availability. Chip read/write callbacks consume
that value instead of an unqualified byte pointer. Board maps to the registry's
record operations, and HDD keeps sparse record metadata attached to the installed
lease. Existing ordinary reads/writes are migrated to the same record path;
there is no additional Long-only reader/writer beside it.

Media reports whether the payload was obtained and which record facts exist;
it does not calculate controller ECC or manufacture corrected/uncorrectable
results. Those results belong to the chip's selected ECC algorithm. This
distinction corrects a possible reading of the earlier outcome note: a provider
I/O error is not an ECC syndrome, and absence of stored ECC is not a four-zero
ECC value. Unknown ECC must stay explicit until its selected algorithm is known.

Allocate metadata before attempting an operation that requires new metadata;
publish its new facts only after the payload operation succeeds. A storage
failure must not replace the old ECC/ID facts. Reset preserves the installed
records. Replacement frees old records only after the new lease is installed;
rejected replacement leaves the old lease and records intact. Remove/finalize
release records even when Storage close reports an error, matching the existing
close-result contract. No controller-owned copy survives media replacement.

Planned production surface is `chips/hdc/{hdc_interface.h,hdc.h,hdc.c}`,
`board-base/{hdc.c,media_interface.h,media_interface.c}` and
`machine/media/{hdd.h,hdd.c}`. Receiving chip contract/taskfile fixtures,
Board HDC/media-provider fixtures and Machine media-provider tests migrate with
that path. Initial estimate for the complete seam and lifetime proof is
250--400 added and 50--100 removed production lines, and 180--300 added and
30--70 removed test lines; source-dependent ECC/format algorithms are excluded
from this estimate and must receive their own evidence-based estimate before
implementation. This is not a partial-P acceptance boundary.

Required direct proofs include stored ECC round-trip, ordinary payload writes
not reusing stale supplied ECC, readonly rejection before mutation, failed
replacement retention, reset retention, successful replacement clearing and
eject cleanup. Chip and Board tests must prove the same record path and reject
stale payload delivery after I/O failure. Source-dependent Long bus phases,
correction, format and diagnostic outcomes remain mandatory S4 work, not
successfully implemented by this plan or by a metadata-only test.

## Record Seam Receiving Checkpoint (2026-10-11)

Implemented the approved copied record contract on the existing HDC callbacks,
Board adapter and media registry. Ordinary ATA/WD and Xebec payload commands now
use that one record route. Byte-only providers explicitly lack stored ECC and
cannot accept a record carrying ECC. Registry checks the actual 512-byte source
geometry/range before dispatch and publishes a read candidate only on success.
Chip reads similarly publish payload only after the provider succeeds.

HDD's sparse supplied-ECC list belongs to the installed medium. Record writes
allocate before changing payload, publish metadata only after successful storage
write, and ordinary byte writes invalidate ECC for every overlapping sector.
Format invalidates affected ECC only after successful payload fill. Reset leaves
the list intact; install/remove/finalize use the existing lease lifetime and
release it. This is ECC-byte storage, not an ECC generator/checker or bad-track/
track-ID implementation. Those remaining record facts and Long/format/diagnostic
chip phases are still required by the frozen batch.

The new Machine media receiver proves supplied ECC round-trip, reset retention,
rejected replacement retention, byte-write and format invalidation, successful
replacement clearing, eject/reopen clearing, readonly rejection and Direct as
well as Overlay operation. Failed/range/absent reads leave the caller's previous
record unchanged. The Board provider receiver rejects non-512 geometry before
dispatch. Existing strict-warning positional provider initializers were migrated
to named fields, including the XT App's fixture; no warning was suppressed.

Both original Ninja/ccache trees built the 11 HDC/media receiving targets.
The current x86 focused selection passes 11/11 (0.74 seconds). x64 passes
11/11 (4.89 seconds), followed by rebuilding and passing the updated Board
record-geometry assertion (1/1, 0.14 seconds). Source/test Core manifests and
Core corpus boundary checks pass. These are focused repository-only proofs:
no complete unit suite, four-App integration, desktop run, new artifact or
complete S4 P is claimed by this checkpoint. Storage allocation/write-error
injection and record-dependent chip semantics remain to be completed.

## Record Failure-Atomicity Receiving Checkpoint (2026-10-11)

Expanded the existing media close-failure receiver, not production interfaces.
It now includes both owner implementations after their Types headers and applies
owner-local call substitution, the same pattern as chip allocation receivers.
An initial command-line substitution was rejected during the test: renaming
Types' inline allocator definition did not inject failure. That ineffective
setup was removed; final injection occurs after header declarations and proves
the actual media call site. The old separate-source/compile-definition setup
is removed rather than retained as another test execution path.

The final receiver proves allocation failure happens before any storage write,
failed ECC writes release their candidate and preserve old payload/ECC, failed
ordinary writes do not invalidate old ECC, and a read that scribbles its borrowed
temporary buffer then fails leaves the caller's record unchanged. Replacement
and eject close failures still report IO_ERROR after consuming the lease, with
record metadata released. The final allocation count returns to zero.

Chip contract/taskfile read providers also mutate their candidate before an
injected read error. The Xebec direct receiver verifies that the prior payload
buffer stays unchanged and existing IRQ/DRQ/sense termination still applies;
ATA taskfile error/status assertions retain their existing receiver. No
additional runtime recovery state, callback, Storage implementation or Lib
change is introduced by this checkpoint.

Both original x64/x86 trees build these receivers under their existing strict
warnings. The final changed three-route selection passes 3/3 on x64 (0.39 s)
and x86 (0.20 s). Both Core manifests pass; no full suite/integration/desktop
or artifact qualification is claimed. The two media-test/build files change
by +112/-9 (net +103), excluding manifests/docs, using git diff --numstat;
the chip fixtures additionally inject destructive read failures. These
lines own fault injection and observability, not a production mechanism.
This completes the earlier named allocation/write/read-failure injection
checkpoint, but ECC/Long/format/diagnostic batch members remain unresolved.

## Additional Original Xebec Supplement Review (2026-10-11)

Acquired the original August 1982 S1410 Owner's Manual Supplement from
`https://www.abc80.net/archive/luxor/diskdrives/drives/XEBEC-S1410-owners-manual-aug-1982.pdf`.
The archived copy is
`manuals-nxvm/hdc/xebec-s1410-supplement-aug1982.pdf`; SHA-256 is
`EB60BB918EDD71D6E3CE20BB38B33EF20B2F5D7801822F7A1BA34E36F021135B`.
Acquisition and hash verification completed on 2026-10-11. The PDF stays in
the external manual archive; ignored renderings/OCR are research inputs only.
The complete relevant PDF pages 11--12 were visually reviewed, not accepted
from the search extract alone.

This is generic S1410 Rev E/F firmware, not proof of the selected IBM firmware.
Its Long description corroborates payload followed by four supplied/stored ECC
bytes and an eleven-bit correction span. It gives no polynomial, initial
register value, bit order or correction implementation. Its controller
diagnostic describes modulo-256 program-memory summation plus RAM and ECC
tests; it does not identify the IBM controller program image or checksum area.
The adjacent Drive Diagnostic heading prints the same opcode as Internal
Diagnostic, so the selected IBM command table remains authoritative.

Other differences prevent treating this supplement as a drop-in specification:
generic 05 is Check Track Format, not IBM Read Verify; generic 0E assigns an
alternate track and 10 reads the buffer, unlike the selected IBM buffer
commands. Its normal corrected read terminates a multiple-sector operation,
whereas the selected Compaq source requires continuation. Its format fill is
6Ch and has a buffer-fill option not established by the IBM DCB table.
None of these differences changes the selected decoder in this checkpoint.

The approved record/lifetime interface removes an implementation prerequisite,
not these original-source prerequisites. H03/H05/H07 remain unqualified for
ECC/Long/format/controller diagnostics; H06's selected IRQ/identity questions
also remain explicit. No diagnostic success constant, synthetic ECC value,
new production branch or artifact is introduced by this research result.

## Registry Fallback And Broad Receiver Build Checkpoint (2026-10-11)

The record-boundary sweep found that the registry's existing direct receiver
only exercised rejection of a non-512-byte geometry. Expanded that same
fixture with configurable sector size and byte-call counters, retaining its
original four-byte/address-mark cases. The added 512-byte receiver proves an
ECC-bearing write to a byte-only provider is UNSUPPORTED before any write;
ordinary read reports ECC unavailable instead of retaining stale output ECC;
ordinary write uses the addressed sector; a destructive failed read preserves
the complete caller payload/ECC fields; and out-of-range, readonly and absent
operations retain their result and mutation boundaries. No production API,
branch, helper, scheduler or provider implementation was added.

Both original Ninja/ccache trees successfully completed `core-board-tests`
(326 scheduled steps per width), followed by rebuilding the changed media
registry receiver. This verifies compilation of the broader board/machine
consumers of the approved record interface; it is not a claim that all those
test executables ran. The final HDC/Xebec/media-provider/media-close selection
passes 12/12 on x64 (0.42 s) and x86 (0.40 s). Tests run once per width.

Relative to the accepted S3 baseline, the registry-test file now changes
+107/-32 (net +75), including its earlier S4 initializer and geometry proof.
The additional size owns fallback/failure assertions and fixture observability.
The test Core manifest is updated. Both Core manifests, Core corpus,
documentation governance and whitespace gates pass. Shared-six, MyNES,
owner INIs and published artifacts are unchanged. Full units, selected App
integration and desktop qualification remain required at complete S4 delivery;
no incomplete P is committed or pushed by this checkpoint.

## Owner-Authorized L2 Emulator Reference Reconciliation (2026-10-11)

The owner explicitly permits implementations under `repos.external` as L2
evidence. This changes the next action from waiting solely for missing original
manual details to testing bounded reference behavior. Hardware-manual facts
retain precedence; observed approximations remain L2, not exact hardware proof.
Read-only reference inspection is not authorization to copy or transliterate
third-party code into NXVM. Both inspected checkouts are clean.

Reference pins and reviewed owners:

- 86Box `4fef696a4eead1d55a28d6ac0e5bd2864e5454da`:
  `src/disk/hdc_st506_xt.c` and `src/disk/hdc_st506_at.c`;
  root COPYING identifies GPL version 2. Use behavior observations only.
- PCjs `c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70`:
  `machines/pcx86/modules/v2/hdc.js`; root LICENSE.txt is MIT,
  copyright Jeff Parsons. No source import is performed.

The reviewed references do not supply a selected-controller ECC algorithm:
86Box's XT Read/Write Long command cases are disabled, despite adjacent
516-byte buffer scaffolding; its ECC burst-length response is fixed at zero.
Its AT owner aborts on the ECC option. These cannot validate implemented
correction, supplied-ECC persistence or successful Long support in NXVM.
86Box's XT controller diagnostic and PCjs's E0/E4 path return successful
completion without modeling controller program checksum or ECC circuitry.
That is a healthy-controller approximation, not a completed self-test.

86Box groups Format Track and Format Bad Track into a zero-fill path and
labels its transfer phase incorrect but working. This cannot supersede the
original bad-track flag/sense-19 contract. Keep NXVM's approved installed-lease
metadata owner; do not copy this omission or its timer constants.

The AT reference supplies a useful independent PIO receiving candidate:
initial Write exposes DRQ without IRQ; completed writes raise IRQ; Read
raises IRQ when data becomes available. This corroborates the existing
IBM/ATA contract but does not by itself identify the custom Compaq controller.
Do not silently remove Compaq distinctions solely from generic AT behavior.

Next implementation design must distinguish three categories: manual-defined
behavior, bounded L2 healthy-controller/flat-image behavior with direct receiver
proof, and a missing reference feature that cannot be counted as implemented.
Existing record transport, scheduler, IRQ/DMA and media owners remain the sole
paths. No second backend, reference dependency, diagnostic framework or guessed
physical ECC polynomial is introduced by this research checkpoint.

This checkpoint changes the source-resolution route; it does not claim full S4
completion, new runtime results, artifact publication or a complete P delivery.

## E4 Healthy-Controller L2 Implementation (2026-10-11)

Following the owner's L2 authorization and instruction to complete S4, the
coordinator records one explicit E4 model decision before qualification:
controller program ROM and physical ECC circuitry are assumed healthy, as in
the pinned PCjs/86Box references. This is not their actual checksum/correction
algorithm and is not L3 self-test coverage. The existing logical buffer-RAM
pattern check is still performed, with the original sense-30 failure path.
The implementation shares E0's owner, completion and IRQ behavior; no medium
query, side diagnostic provider, new state, API or scheduler is added.

The same direct receiver now runs for E0 and E4 under all four IRQ/DMA masks,
with the selected unit absent. It checks zero media calls, normal completion,
masked-pending acknowledgement, Request Sense, buffer contents and subsequent
Read Buffer. Physical fault detection is excluded explicitly, not disguised
as a tested success constant. Remaining Long/ECC/format and H06 dispositions
still require reconciliation; this decision alone does not close H07 or S4.

The Board-Base receiver also runs E0/E4 through the actual 320h/323h route,
testing masked completion, enable-to-PIC IRQ5, acknowledgement, no DMA request
and unchanged media bytes/read count. It extends the existing diagnostic vector,
not a chip-only substitute for board receiving proof.

An additional local Bochs 2.6 reference was checked read-only:
`bochs-2.6/iodev/harddrv.cc`, SHA-256
`9F1630CC0A41073C09A700B3687D927EC4C8F7328512E3B49970C068CCA850F2`.
This local archive is not a Git checkout; its LICENSE describes LGPL with
per-file exceptions, so no code is imported. It reports normal diagnostic
success but explicitly aborts ATA Read/Write Long. It therefore does not resolve
the selected Xebec/Compaq ECC algorithm either. Unimplemented reference features
remain missing evidence, not authorization to report NXVM commands complete.

Both original Ninja/ccache trees rebuilt the changed HDC and Board-Base
receivers successfully. Final E0/E4/task-file/wiring selections pass 3/3 on
x64 (1.00 s) and x86 (0.21 s). Core source/test manifests, Core corpus,
documentation governance and whitespace checks pass. Shared-six, MyNES and
published assets remain unchanged. These results qualify the stated E4 L2
model and receiving path only; complete units, four-App integration and new
eight-EXE publication are still required for complete S4 delivery.

## Long Transport And Explicit ECC Availability Decision (2026-10-11)

Long transfer uses the approved copied record, replacing the chip's payload-only
buffer rather than adding a second image or buffer owner. Ordinary transfers
retain 512 bytes; MFM Long transfers 512 data bytes plus four byte-wide ECC
operations, while Xebec DMA/PIO transfers 516 bytes. A full Long write retains
the supplied tag; cancellation before its final byte publishes no record.

Raw payload files have no stored ECC until it is explicitly supplied. Long read
without ECC must fail through the existing model error path (MFM ABRT, Xebec
invalid/unsupported-command sense), not fabricate four zero bytes or report
drive-not-ready. This is an explicit L2 capability boundary, not a claim that
real hardware lacks its ECC field. Normal generation/correction and the frozen
invalid-ECC ordinary-read receiver remain unresolved; transport qualification
alone cannot accept those items or close S4. Planned production logic +120--180,
tests +200--300 plus mechanical private-field migration; no new public API.

Long transport is now implemented for IBM/Compaq 22h/23h and 32h/33h plus
Xebec E5/E6. The private payload buffer is replaced by the existing copied
record type; 41 owning source/test lines were mechanically migrated to its
payload field. Normal-sector geometry, Identify, Format and buffer commands
retain their payload length. MFM consumes 256 words then four ECC bytes;
Xebec uses the same 516-byte limit through existing DMA and PIO paths. The tag
becomes available only at full input completion; normal writes invalidate it.

Initial qualification exposed two old assertions that accepted WD Long as a
short 512-byte Read. They now prove absent-ECC rejection and retain ordinary
Read separately; new record-capable receivers prove complete 516-byte success.
This changes the opcode contract rather than deleting the ordinary payload
assertion to hide a regression.

Direct tests prove multi-record tagged round trips, no completion at byte 512,
PIO/DMA parity, ordinary-write invalidation, explicit absent-ECC failure,
reset/early-TC cancellation and failure-preserved media/staging records. The
IBM port receiver additionally proves Board/Registry retains supplied ECC
and returns it after the data words. Existing actual Storage-provider tests
retain supplied tags through reset and discard them on replacement/eject;
the chip does not become their lifetime owner.

Final affected receivers pass 12/12 on x64 (6.50 s) and x86 (4.17 s) after
successful original-tree builds. The four changed transport/receiver files
have cumulative git-diff counts +1895/-213 (net +1682) against the accepted
source baseline, including earlier S4 work and mechanical renames. Normal
ECC creation, correction and Burst Length are not proven by these transport
tests; H06 and complete S4 qualification remain. Shared-six, MyNES, owner
INIs and assets are unchanged; no incomplete P is pushed.

## Track-Metadata Implementation Decision (2026-10-11)

The approved record/track extension is implemented as one copied format-track
request, not as an additional disk backend. The request carries the first
logical sector, count, zero-based sector IDs in physical order, per-ID bad flags,
fill byte and whether payload is to be filled. Its bounded 256-ID capacity
matches the existing 512-byte AT format table. Registry validates range,
unique IDs and write protection; the HDD installed lease owns the resulting
ID order/bad flags. Ordinary record access reports a bad-track result; byte
access remains a payload operation and cannot erase physical ID flags.

Reset retains the track metadata, replacement/eject destroy it, and failed
allocation/format does not publish the replacement ID table. Formatting data
uses the existing Storage fill operation; the flat image has no second payload
copy or persistence sidecar. A reported backing-write failure is not a promise
of rollback of any bytes already accepted by Storage. Normal zero-fill is an
explicit L2 reference-model choice, not the generic S1410's 6Ch hardware fact.
This stage adds the owning operation/receivers before connecting chip commands;
the format commands cannot be counted complete until the receiving path passes.

The owning track operation is now connected for selected Xebec 06/07 DCBs.
The original IBM command table supplies their track address, interleave 1--16
and bad-track disposition; the logical physical-ID ordering and normal zero
fill are explicitly L2. Neither implementation claims physical ID/ECC encoding.
Track metadata is allocated before backing fill and published only after
successful completion. Read/Write record share one bad-sector lookup; byte
payload writes do not clear header flags. Reformatting installs new ID flags.
The existing Registry binding helper now sets its successful lookup result
itself, removing the record helper's duplicated fix for a stale INVALID_RANGE.

Direct vectors exercise 06/07 at every interleave, unique IDs, a fixed
interleave-three order, invalid interleaves and protected/range callback failures.
The Board-Base port receiver proves 07 -> controller reset -> Read -> PIC IRQ5,
no DMA and addressed sense 19h through 320h/323h. The actual Storage-backed
HDD receiver proves selective bad IDs, reset retention, payload writes retaining
flags, invalid-ID rejection, failed replacement preservation, normal reformat,
readonly rejection and eject/reopen cleanup. Existing owning failure injection
additionally proves track allocation failure precedes writes and fill failure
retains the old ID/ECC ownership without leaking the candidate.

Initial media-provider qualification caught the new track entry checking the
binding helper's stale result. Fixing that result at its sole owner, rather
than adding another entry-local reset, resolved the failed route. Both original
Ninja/ccache trees subsequently rebuilt the affected chip/board/media and App
receivers and pass the final 12-route selection: x64 12/12 (4.40 s), x86 12/12
(0.83 s). Shared-six, MyNES, owner INIs and assets remain unchanged.

At this checkpoint the 13 changed HDC/media/receiver files show cumulative
git-diff size +2305/-210 (net +2095) against the accepted source baseline;
this includes earlier S4 record, command and failure work, not just track work.
Additional code owns the copied track operation, installed metadata and direct
failure/receiving proof. Format Drive, WD/Compaq format and Long/ECC remain in
the S4 predicate; no incomplete P or complete-suite/artifact claim is made.

## Full-Drive And MFM Format Phase Decision (2026-10-11)

Before implementation, original Compaq printed 9-9, 9-14 and 9-17 were visually
reconciled. Note 4 excludes E4/E8 buffer commands from the selected 40-MB
controller, despite their descriptions elsewhere in the general chapter.
They remain rejected; the format repair must not add the 130-MB command set.
The selected format table carries 00h/80h flags followed by one-based sector
IDs. The common AT-style 512-byte post-command input phase is explicitly L2
for the 40-MB personality; it is supported by the pinned AT reference, not a
claim that the generic Compaq preload description proves that phase for 40 MB.

MFM format receives one complete table, validates it, then issues the existing
track request. It is not an ordinary sector write and does not accept a short
table. The captured command fields identify the target; no second buffer,
queue, scheduler or persistent format flag is added. Xebec drive formatting
iterates the programmed tracks using the same track operation and reports the
first failure, preserving the successfully accepted prefix. Neither route
claims physical ECC creation or rotational timing; those remain explicit
record-model boundaries. Planned production +120--150 lines, tests +150--200,
within the already approved Core HDC/media extension.

The three format command paths are now connected to the one track request:
Xebec 04 iterates from the requested starting track through the programmed
drive geometry, including its last cylinder, unlike the pinned reference's
explicitly marked incomplete capacity formula. It stops at the first rejected
track and never retries an accepted prefix. Address-width bounds make the
iteration finite; no wall-clock/retry coverage budget is introduced.

IBM WD1003/Compaq 50 receives the full 512-byte table in the existing DATA_WRITE
phase and formats only after its last word and scheduled completion. It validates
00h/80h flags, one-based unique sector IDs, captured geometry and medium range.
Format does not become an ordinary sector write; no read/write-record callback
is invoked for the table. The existing captured command fields determine the
drive/track even if the Compaq live selection changes while supplying data.
Readonly/malformed/failed format uses existing error/IRQ completion; cold reset
cancels a partial table without publishing metadata. No public ABI or persistent
format state was added by this phase implementation.

Direct tests cover Xebec full-drive/start-head/failed-second-track cases;
both MFM personalities cover 510-byte non-completion, full-table completion,
invalid flags/zero IDs/duplicates, provider protection failure and reset
cancellation. The original IBM port receiver proves its 1F0h table path and
IRQ14; the Compaq dual-drive receiver proves a captured slave target cannot
be redirected to master by changing the live drive selection. The existing
40-MB Identify/Buffer rejection receivers remain unchanged and pass.

Final affected receivers pass 12/12 on x64 (3.38 s) and x86 (1.13 s) after
rebuilding both original Ninja/ccache trees. At this checkpoint the four
changed phase/receiver files have cumulative git-diff counts +1638/-186
(net +1452) against the accepted source baseline, including previous S4 work.
Long/ECC, remaining disposition review, full units/four-App integration and
eight-EXE publication are still required; no incomplete P is delivered.

## Broader Core Receiver And Closure-Prerequisite Review (2026-10-11)

Both original Ninja/ccache directories rebuilt `core-board-tests` successfully.
The `ctest -L core -L unit` selection then passed 245/245 on x64 (157.68 s) and
245/245 on x86 (32.82 s). These are the Core-labelled registered receivers,
not all Core-owned tests: some chip/CPU routes retain another label. This
checkpoint therefore does not substitute for the complete repository unit
selection, four-App integration, eight artifact builds or desktop acceptance.

The matrix above now includes the later logical-format and Long-transport
checkpoints instead of leaving those implemented routes described as wholly
pending. Transport preserves supplied ECC; it does not create or validate it.
Normal reads still cannot establish the frozen invalid-ECC/corrected-read
contract. The pinned reference implementations do not implement the selected
ECC polynomial/initialization/bit order and cannot establish that contract.
Inventing a polynomial or converting every ECC result into success would
violate the current packet, not complete H03/H05/H07.

H06 also remains unaccepted. The Compaq source establishes independent
controllers, but not the current exceptional busy task-file writes and
first-write/final-read IRQ branches. The current Board has one HDC instance.
Adding independent instances or state banks changes the packet's explicit
no-second-controller constraint; topology alone does not authorize a guessed
mid-transfer host sequence. Neither shared-state compatibility nor generic
ATA behavior qualifies that personality without the missing contract decision.

No production code, Shared-six, MyNES, INI, media, snapshot or artifact was
changed in this review. S4 stays active and unaccepted; no incomplete P is
committed or pushed. Its next admissible implementation needs the selected
ECC authority or an owner-approved explicit model boundary, and a reconciled
Compaq topology/selection contract rather than another unbounded search or
a passing boot substituted for controller proof.

## Complementary Unit Qualification (2026-10-11)

The original x64/x86 trees then successfully rebuilt `build-unit-tests`.
To avoid repeating the already completed 245-entry Core-labelled group,
`ctest -L unit -LE core --output-on-failure --parallel 8` ran its 301-entry
complement once per width. Each completed 299/301 successfully; the two
failures were timeouts of `lib.test-types-boundary-selftest` (30-second budget)
and `emulator.verifier-negative` (60-second budget), not reported assertion
failures. The x64 complement elapsed 141.41 s and x86 142.12 s. All registered
four-PC-App labelled unit receivers in this group passed. MyNES is disabled
in these product build configurations and is not claimed tested here.

After both complete complement processes terminated, only those two entries
were replayed, first on x64 and then x86, with `--parallel 1` and unchanged
budgets. x64 passed 2/2 (13.61/27.17 s; 40.90 s total), x86 passed 2/2
(13.54/19.36 s; 33.03 s total). The result distinguishes successful assertions
in isolated runs from failure under the preceding concurrent load; it does
not prove a stable, clean concurrent full-suite qualification. No timeout,
Shared test, operating-system setting or product code was changed to obtain
these replays. Every registered entry now has a passing execution on each
width, while the two original timeout results remain explicit.

Source/test Core manifests, Core dependency gate and whitespace checks pass.
No Shared-six, MyNES or deployed asset diff was introduced by these builds.
The S4 ECC/Compaq contract prerequisites, four-profile integration and artifact
publication remain outstanding. Unit success does not dispose those items;
no incomplete implementation P or S closure is claimed.

## Four-Profile Current Receiving And Candidate Artifacts (2026-10-11)

The original six selected-profile caches were found parked beneath
`build/to-be-deleted`, not lost. Their cache metadata still named the original
`build/t550-s8-{my5160,my5170,mydeskpro386}-{x64,x86}` paths. Each source and
destination was validated below the workspace build root, each destination
was absent, and the existing directory was moved back without deleting or
overwriting files. No new qualification build directory was created.

The original default trees completed `run-integration-tests` once per width.
The other six restored trees each built only `run-integration-tests` and the
current `vm-0-5-0555` artifact, with persistent ccache and Ninja jobs 8. Groups
ran sequentially, using their existing one-job integration setting and
300-second aggregate containment. No per-test timeout, instruction budget or
guest checkpoint was changed. All processes terminated successfully:

| Selected App | x64 | x86 |
| --- | --- | --- |
| NXVM | 22/22, 82.09 s | 22/22, 82.44 s |
| My5160 | 2/2, 28.17 s | 2/2, 33.58 s |
| My5170 | 4/4, boot 40.11 s | 4/4, boot 59.06 s |
| MyDeskPro386 | 4/4, 116.36 s; boot 115.24 s | 4/4, 162.71 s; boot 161.85 s |

This is 32/32 registered four-profile integration receivers per width,
including the AT/DeskPro native Console process fixtures. These are automated
native fixtures, not owner desktop acceptance. Each current INI declares
overlay for all installed media; the four INIs remain unchanged. This review
does not claim the setup silently overrides every possible future INI mode.
No protected master was intentionally written or copied into the repository.

All eight optimized Release 0555 candidates were built and copied by the
existing deployment recipe. Their PE widths pass the existing verifier, and
each asset SHA-256 matches its corresponding original build output:

| Candidate asset path | SHA-256 |
| --- | --- |
| `assets/nxvm/nxvm_default_0_5_0555_x64.exe` | `9E754982AFEBFF63A50A20F178ED95A1A68F890D9BB33B9C8872980F4C9B0607` |
| `assets/nxvm/nxvm_default_0_5_0555_x86.exe` | `1286B6A265DA9B646982364D545DF4A8E751A77CA8A21C209F54272FFC7A6174` |
| `assets/my5160/nxvm_xt_0_5_0555_x64.exe` | `ED5290C1D56C21069BD4EF48CAB5B336964C50113F308227F9EF95476217940F` |
| `assets/my5160/nxvm_xt_0_5_0555_x86.exe` | `8DA073B4477506310BCECA20C651CE331E4AE48D0372972DD5B5497535CBEC18` |
| `assets/my5170/nxvm_at_0_5_0555_x64.exe` | `0B9F0AC9C916CDF3C5C2E0E017D779592BDBE8D088916EDAD7B470C802F775CC` |
| `assets/my5170/nxvm_at_0_5_0555_x86.exe` | `F85BD2CE244DD029BFECD4EC6C0AD3135CD8A3F59DC1BFBAECF01C42B410ED50` |
| `assets/mydeskpro386/nxvm_model40_0_5_0555_x64.exe` | `911922D357AB52633DD97F518C0772138EAA7EA9A06BD26A4672D418A25B743E` |
| `assets/mydeskpro386/nxvm_model40_0_5_0555_x86.exe` | `4965ABB597B8190BC705860A86CA0EA3E4F604869273FA3AEC0201B6CB3F6004` |

The source baseline remains accepted S3 plus the uncommitted S4 worktree;
these are candidate bytes, not an accepted/pushed source revision. The build
recipe's normal post-build deployment explains the eight new asset diffs.
Shared-six, MyNES and all owner INIs remain untouched. ECC generation/check/
correction and the selected Compaq controller contract still prevent whole-S4
acceptance. Passing firmware checkpoints does not repair or qualify them.
No partial implementation P, artifact release or S closure is declared.

## Approved Compaq Topology And Notification Checkpoint (2026-10-11)

The owner approved the proposed unblock route: use two independent instances
of the existing Compaq HDC implementation and the existing Board scheduler;
prepare a concrete ECC L2 design for review before its implementation. This
supersedes the earlier prohibition of a second physical controller instance,
not the prohibition of a second implementation, scheduler or input queue.

Board-Base now selects the instance from Drive/Head unit bit 4 before delivering
that write. Other task-file/data/command accesses go only to the selected
instance. Shared reset/control reaches both. Both receive the same guest time,
and the existing board deadline receiver gets the minimum of their deadlines.
The one PIC IRQ source reflects the OR of existing native outputs; no mirrored
IRQ state or new manager was added. Destruction removes each pointer before
its native destroy callback can observe it. Controller selection is Board
wiring; sector protocol and all command state remain chip-owned.

Both configured Compaq endpoints are reset-ready even when a logical medium is
absent, preserving the established controller-probing model. An absent medium
is not controller hotplug. This is a fitted-endpoint model boundary, not proof
that every physical machine had two disks installed. Successful configuration
guarantees both instances; the unnecessary null-selected floating-bus/noop
fallback was removed rather than retained as unreachable compensation.

The Compaq source requires checking busy/DRQ status and does not establish the
previous writable-in-flight exception. The chip now protects in-flight task
files using the same guard. For the unestablished first-write/final-read IRQ
exceptions, the model adopts the already pinned WD/AT reference behavior:
first write becomes available through DRQ without IRQ; each ready read block
raises IRQ; subsequent write buffers and final write completion raise IRQ;
consuming a final read block does not raise another completion IRQ. This is
explicit L2, not a claim of captured Compaq hardware waveforms. No status IRQ
acknowledgement, interrupt-mask rule or independent command context is replaced.

The existing direct taskfile receiver now runs all read/write notification cases
for ATA, IBM WD1003 and Compaq, and attempts every task-file/command mutation
during both BSY and DRQ. The existing Compaq Board receiver proves two separate
command deadlines, shared IRQ mask/OR and selected acknowledgement, independent
partial reads/writes/format data, and shared reset cancellation. Existing media
absence, step/head geometry, machine-port and Xebec receiving checks remain.

After the final cleanup and notification change, original Ninja/ccache trees
rebuild the six focused chip/Board receivers. x64 passes 6/6 in 2.50 seconds;
x86 passes 6/6 in 0.70 seconds. An initial build request named nonexistent short
aliases; Ninja rejected it before compiling. It was corrected to the existing
targets without creating another target or build tree. These are focused unit
results, not a new complete-unit/integration/Desktop/artifact qualification.

Measured cumulative diffs against accepted HEAD, not isolated topology-only
increments: `board-base/hdc.c` +103/-24; `hdc.h` +2/-1;
`hdc_interface.h` +3/-1; `chips/hdc/hdc.c` +598/-179;
Board dual-drive receiver +183/-15; Board fixture +2/-1;
chip taskfile receiver +123/-8. The large chip totals include all earlier S4
storage repairs and must not be attributed to this checkpoint alone.

The [concrete ECC review](t555-s4-storage-ecc-model-review.md) records original
source gaps, chosen model alternatives, encoding checkpoints and review-only
decoder checks. It is not yet production ECC proof or authorization for an
unspecified algorithm. H03/H05/H07 ECC generation/check/correction still prevent
whole-S4 acceptance; the old EXEs and integration checkpoint precede this new
Compaq code. Required final rebuild/regression, complete P push and coordinator
review remain. Shared-six, MyNES, owner INIs and media are unchanged. No partial
implementation P or S closure is declared.

## Construction Failure And Current Four-Profile Replay (2026-10-11)

Extended the existing Board dual-drive receiver with owner-local native-create
and native-destroy call substitution. First-controller and second-controller
creation fail independently. Both report NO_MEMORY, leave no native controller
allocation or configured endpoint, and permit the subsequent real configuration.
Final destruction releases every subsequently created instance. The added test
scope is +42/-0 lines relative to the preceding checkpoint; no production/API
change, allocator framework or new test target is needed. The two helpers own
failure injection and allocation counting, not a forwarding production layer.
The receiver passes x64 1/1 in 0.72 s and x86 1/1 in 0.40 s after rebuilding.
Its cumulative diff against HEAD is now +225/-15; its manifest is refreshed.

The proposed ECC arithmetic review now also has a complete bounded-error-domain
uniqueness check: 4,217,855 Fire bursts and 65,999 WD bursts, no zero or duplicate
syndromes. This is review-only mathematical evidence, not production ECC or
command qualification. All original-source/model boundaries and the required
owner model review remain intact.

Using the original eight Ninja/ccache trees, rebuild only the actual receiving
integration targets and current product target. Integration groups execute
sequentially once per width, with the existing budgets and INIs:

| Selected App | x64 | x86 |
| --- | --- | --- |
| NXVM | 22/22, 71.81 s | 22/22, 90.64 s |
| My5160 | 2/2, 32.32 s; boot 31.78 s | 2/2, 39.17 s; boot 38.74 s |
| My5170 | 4/4, 50.48 s; boot 49.27 s | 4/4, 61.35 s; boot 60.50 s |
| MyDeskPro386 | 4/4, 153.76 s; boot 152.91 s | 3/4, 181.34 s; boot fails at 180.17 s |

Current receiving result is 32/32 x64, 31/32 x86, not an all-green matrix.
The DeskPro x86 test emits TERMINAL-TIMEOUT at its unchanged internal 180-second
limit, rather than CTest's 190-second containment. Final diagnostic PC is
F000:90DF, guest tick 373067509, next deadline 373067570, lifecycle running and
progress disposition active. It is not a process crash, but those facts alone
cannot distinguish insufficient throughput from an incorrect guest sequence.
The preceding baseline had narrow x86 margin; x64 now passes more slowly too.
No exact causal attribution to host load or the new HDC code is claimed.
Code inspection confirms both HDC instances use existing board time and
deadlines; it does not prove that their added execution cost caused this timeout.
No guessed optimization, enlarged budget, restart or success reclassification
was used. The failing result remains required S4 qualification work.

All eight product targets compile/link and pass their normal architecture
verifier; DeskPro x86's umbrella command fails because its test fails, not
because linking failed. Existing deployment copied their bytes to assets.
All eight asset hashes equal their respective original build outputs:

| Candidate asset | SHA-256 |
| --- | --- |
| `assets/nxvm/nxvm_default_0_5_0555_x64.exe` | `EF034B30017CC429C3FDD1E9EDDE9517D1D896915BE4DF487614FB3E984D3900` |
| `assets/nxvm/nxvm_default_0_5_0555_x86.exe` | `2CC847F0CAEFD3A81ABE83E6DC2257367C3D694B82BEE564D069BADCA5CF340B` |
| `assets/my5160/nxvm_xt_0_5_0555_x64.exe` | `3E2F70A637BF225CD881579025EEC772D7DC71016C987DAAEA41018AB360032C` |
| `assets/my5160/nxvm_xt_0_5_0555_x86.exe` | `14BEFE769D9100794214726C2BD9E3688D3332BD6EB3C4615308D3C6A499F0A9` |
| `assets/my5170/nxvm_at_0_5_0555_x64.exe` | `0DA3811AC7100019CDB9C41778F0FCEA8B041D1BEE102C25E4AD48CF93E23EBF` |
| `assets/my5170/nxvm_at_0_5_0555_x86.exe` | `D083B620B5FD14A9A256D14ACE8F17868E1708D2B925C24AB8CEACEDECE377AA` |
| `assets/mydeskpro386/nxvm_model40_0_5_0555_x64.exe` | `68EFB5F7B45672EBCD5F02B2611569023DF290BA802656CAC973DACB0C896FE6` |
| `assets/mydeskpro386/nxvm_model40_0_5_0555_x86.exe` | `DF7F81721F1994F9F94BDCF9C8408B98F75F92C9FE92FFFF5481A85EEADCD9DB` |

These are uncommitted candidate artifacts for the S4 worktree, not accepted
source-revision deliveries. The four owner INI hashes still equal the preceding
checkpoint. Shared-six and MyNES have no worktree change. Complete-unit
qualification predates the latest HDC work and must be renewed at the complete
repair boundary. Owner desktop acceptance is not claimed. ECC model approval,
implementation and final full receiving remain prerequisites; no partial P
or S closure is declared.

### Approved ECC Implementation Checkpoint — 2026-10-11

The owner subsequently approved the concrete L2 model in the linked ECC review.
The chip now generates four-byte tags, checks supplied tags, locates a unique
bounded single-burst candidate and verifies the corrected working copy before
publishing it. Correction never writes back to the medium. Raw untagged payloads
are healthy logical records, not evidence of physical disk ECC. Long reads and
writes retain supplied tags; ordinary MFM/Xebec writes generate replacement tags.
ATA payload access remains outside this ECC model.

Four private arithmetic helpers own model selection, remainder, encoding and
checking. The existing Xebec state gains only its programmed correction limit
and last corrected burst length; these are required Initialize/0D register
facts, not a recovery manager. No public ECC type, function or framework is
added. IBM uses the reviewed five-bit WD-family model; Xebec/Compaq use the
eleven-bit Fire model. Source limitations remain as stated in the model review.

Chip receivers cover independent vectors, every single-bit position, maximum
bursts at representative positions, uncorrectable data and detection-only mode.
Command receivers cover IBM/Compaq corrected and uncorrectable ordinary reads,
Compaq flawed-data delivery/address retention, and Xebec PIO/DMA data, sense,
Burst Length, Initialize limits and reset. Existing Board fixtures now persist
generated tags through their actual record callbacks instead of rejecting all
tagged ordinary writes. The six selected chip/Board receivers pass per width.
One Model 40 App assertion still expected the superseded extra final-read IRQ;
it is corrected to the chosen notification contract and passes individually.

A header-only Format must invalidate old record tags as well as a filling
Format. The media owner now invalidates the affected range without changing
payload bytes. Its provider receiver proves preserved payload and absent tags.
The allocation/failure fixture reinstalls a record after this successful Format
before testing failed filling Format, preserving its old-facts-on-failure
assertions. Provider and close-failure receivers pass both widths. Close error
reporting and consumed-lease behavior remain covered, not weakened.

Whole-unit intermediate checkpoints are 545/546 x64 (only the stale App IRQ
assertion) and 546/546 x86. The corrected x64 App receiver passes separately.
These checkpoints precede the latest header-only Format change and therefore
are not final qualification of the frozen repair boundary.

The bounded Model 40 x86 diagnostic at the unchanged 180-second guest boot
limit again timed out, at F000:90CE, tick 363357122, next deadline 363357123.
Its zero IRQ/port counters were unusable evidence: Release disables runtime
trace recording. They do not establish missing interrupts. The existing probe
now explicitly reports disabled tracing and captures the paused native FDC
and Board IRQ state through existing Core test support; its target links that
existing fixture. This changes no product execution, budget or public API.

A temporary native-only advance/deadline micro-measurement found 189.1597 ms
for one controller and 203.3385 ms for two over ten million iterations. It does
not include the full Board/guest path and neither proves nor excludes the
cause of the boot timeout. Its temporary source and binary were removed.
No source-tier promotion, enlarged deadline, partial P or S closure follows
from these intermediate results. The deployed eight candidates still predate
the new ECC implementation and must be rebuilt before delivery.

The subsequent copied-FDC diagnostic also times out at the unchanged budget:
PC F000:90E3, elapsed 348156333, next deadline 348156433. FDC is execution-read
(phase 2), command E6, MSR 10, remaining 7682, no result bytes, native IRQ/Board
wire/DRQ all low. Boot signature is not present. These sampled facts establish
an unfinished transfer at containment, not whether throughput or an incorrect
sequence caused it. Its temporary worker exits and is contained by the same
190-second Windows job wrapper. One narrow media rebuild ran during the early
part of this diagnostic, so it is not an unloaded performance measurement.

The final-candidate eight product builds now complete on their original
Ninja/ccache trees; all PE checks pass and every deployed asset equals its
build output. No MyNES artifact is rebuilt. Updated hashes are:

| Asset | SHA-256 |
| --- | --- |
| NXVM x64 | `ACA472837D93662011C4FB70BD6D7C114B08B4A69F771E784016C230FF627472` |
| NXVM x86 | `07737C1A1CED60DCEB3CD826DC7AA0B44C59D89DED023527130C662DA3911BB7` |
| My5160 x64 | `BEE8B5B2C8DF6FAC0A1802E5AB2417B6BA8B5E1097DE7F82D548329414AA9340` |
| My5160 x86 | `75E7D6F0EEB98622E385C785A7F7474DEF3DF0DE4A5C78DB1F1C9A90D0D575EA` |
| My5170 x64 | `84C69B0F1BC7BCF9CFC552501348F2FA3156D262293E85A281CED847B2133631` |
| My5170 x86 | `6EE4ED6A310E44FF7A400C8756BB5416A6AE20E46C04C3B7858A0512C5EF8F18` |
| MyDeskPro386 x64 | `55BBC5CE82A53145E8FDEF29B76CC8352D528D01123D5622F71895ABF91BE13B` |
| MyDeskPro386 x86 | `32C786211C814C91512AFAF45AE3C6489EB057699D415E24828A59902D9614A0` |

Renewed frozen-candidate x64 complete units pass 546/546 in 263.83 seconds,
serially inside the existing 300-second aggregate containment. Core/source
and Core/test manifests pass. This is not renewed four-App integration or
interactive desktop acceptance; the x86 complete run and integration remain
separate required results.

Renewed frozen-candidate x86 complete units also pass 546/546 in 214.57 seconds,
serially inside the unchanged 300-second containment. The final integration
matrix now runs once per App/width, sequentially after units and without
concurrent product compilation outside its own prerequisite target.

### Final-Candidate Row Reconciliation

The earlier open-row table is a historical implementation checkpoint, not the
latest disposition. Its 22 original rows remain the acceptance universe:

| Rows | Current implementation/proof disposition | Qualification still due |
| --- | --- | --- |
| F01--F11 | Existing detailed command/reset/ready/TC/IRQ/DRQ/timing receivers remain passing in both complete-unit runs; raw rotation/physical CRC/head-load pin boundaries remain explicit | Current receiving matrix |
| F12 | Sole Board ports/DMA2/PIC/provider route retained; no firmware compatibility path | Current four-App receiving |
| H01--H02 | ATA variants, count/Verify, address and notification receivers pass; no ECC protocol grafted onto ATA | Current receiving matrix |
| H03--H04 | IBM WD record/format/Long and five-bit approved L2 ECC, reset/control/step/IRQ14 receivers pass | Current receiving matrix; no original physical ECC-wire claim |
| H05--H06 | Two Compaq instances, shared control/ORed IRQ, selected-only acknowledgement and independent streams/deadlines pass; WD/AT notification and eleven-bit ECC are explicitly L2 | Current receiving matrix; original Compaq waveform not claimed |
| H07 | Xebec commands, 0D actual burst, Initialize 0/5/11 and rejected 12, Long and approved Fire correction pass; logical E3 and healthy-controller E4 remain the documented model, not physical ID/controller-ROM/ECC-circuit diagnostics | Current receiving matrix within those bounds |
| H08 | The same Board DMA3/PIC5/TC/masked-ending-status route receives the record implementation; no parallel input/result queue | Current XT receiving |
| M01 | Record/track facts remain sparse installed-lease metadata; ordinary write/format invalidation, replacement/eject and failure receivers pass | Current receiving matrix; no sidecar/commit backend |
| T01 | Existing checked deadlines and clock conversions pass, including two-controller minimum-due reduction; nonphysical mechanical timing stays classified | Current receiving matrix |

No row is removed by the ECC approval. Neither passing unit counts nor an
unimplemented physical phenomenon constitutes an exact-hardware claim.
Final qualification, complete-P delivery and actual-pushed-diff review remain
required before S4 acceptance.

### Final Batch Results And Console Receiver Correction

The complete once-per-App/width batch finishes with 31/32 x64 and 30/32 x86:

| App | x64 | x86 |
| --- | --- | --- |
| NXVM | 22/22, 67.23 s | 22/22, 82.91 s |
| My5160 | 2/2, 29.71 s | 2/2, 37.82 s |
| My5170 | 3/4, 53.90 s; Console command result fails | 3/4, 68.84 s; same receiver fails |
| MyDeskPro386 | 4/4, 157.94 s; boot 156.62 s | 3/4, 180.97 s; boot containment fails |

The DeskPro x86 terminal snapshot is F000:90F3, elapsed 411816212, next
deadline 411816216, FDC execution-read E6 with remaining 11983, ST0 04,
DRQ high, no result/native IRQ/Board IRQ wire. INT09 has changed to 0FD6:069C;
the sampled guest is further along than the earlier diagnostics, but it has
not reached the declared terminal checkpoint. That is not qualification of
throughput or proof that hardware behavior is correct. No budget changes.

My5170's failed command is confirmed as help, not speed or INFO. Readback from
the failing private Console gives buffer 53x9001, viewport (0,58)..(52,71),
cursor (2,71). Both startup help and the newly requested help are present in
the backing buffer; their titles have scrolled out of the viewport. Thus the
old visible-only search could miss real emitted output. An unchanged-behavior
diagnostic replay passes on x64 but still fails on x86; those facts are retained,
not reported as a fixed product defect or unconditional passing batch.

Only `test/core/setup/pc_console_process.c` changes for this correction:
+30/-9 lines. It searches emitted buffer rows through the current cursor in
bounded overlapping chunks, so a marker crossing a chunk is not lost. Before
each command it clears prior output in this test-created private Console so
startup help cannot falsely satisfy the later help request. Cursor, geometry,
attributes and input remain unchanged; no input flushing or product/Lib change.
The failure command index is retained, but the temporary screen dump is removed.
No helper, interface or persistent state is added; the five-second budget stays.
This file and the changed NXVM CMake file are normalized to LF, and the Core
test manifest hashes the normalized bytes rather than a CRLF working copy.

Corrected My5170 receivers pass x86 0.31 s, x64 0.46 s; the other consumer,
DeskPro Console, passes x64 0.39 s and x86 0.49 s after rebuild. Both Core
manifests and documentation governance pass. The earlier complete units are
unchanged-executable checkpoints before this integration-only setup repair;
these focused receiving results do not turn the failed complete batch into
an unconditional all-green run. DeskPro x86 boot remains unresolved.

All eight rebuilt PC artifacts still match their build outputs. Shared-six,
MyNES, owner INIs/media and source authorities remain unchanged. No S4
implementation P, push, actual-pushed-diff acceptance or closure is declared.

### Published-Frame Reader Ownership Repair

Current-source inspection finds a confirmed test boundary defect, independent
of whether it caused every prior timeout: the main-thread presentation helper
calls `vm_machine_copy_emulator_frame()`. That calls display publication and
mutates the Core snapshot/generation/cadence/cache while the bound executor
also owns those operations. It does not hold the existing published-frame
lock and cannot use a direct driver call as a concurrent read contract.

The helper now uses the existing
`emulator_machine_copy_published_frame(executor, ..., run_generation)` whenever
an executor is bound. It returns unavailable if that run has no publication;
it does not force rendering, clear device change facts or borrow a mutable
staging frame. Synchronously driven, unbound units retain their original
direct read. No public API, Shared change, observer manager, queue or second
renderer is introduced. The boot poll captures once and classifies both POST
error and terminal from that same copied frame, retaining every terminal/error
string and the ten-millisecond poll/180-second containment.

The original-tree DeskPro x86 receiving route now passes in 176.12 seconds
(boot 176.06), within its unchanged limit. This is receiving evidence, not
proof that every earlier timeout had the same cause or a throughput guarantee.

The existing default-App display composition unit adds 20 lines: first it
proves a ready unbound local frame, then binds a real stopped Emulator Machine
that has no publication. The helper must return INVALID_STATE without changing
the output generation, local display generation or publish time. Temporarily
restoring the original direct-call helper makes this new x64 unit fail;
restoring the repair makes it pass. Both widths pass the repaired unit. The
temporary mutant is removed and the helper hash is restored to
`FA8654404781325985C81665966ACC0C3A14F6829694B246411290027216AD4F`.

The similar-issue sweep finds one additional direct driver-copy unit call in
runner cadence initialization. Its force/copy check is moved before executor
binding (+3/-3), so it remains a synchronous initialization check rather than
an executor-owned driver call. Its lifecycle/cadence assertions pass both
widths. The other 22 presentation-helper references receive the single helper
repair; no per-App polling or publication path is added.

Production inputs and eight artifact hashes remain unchanged by these test
repairs. The final code is now frozen for renewed complete units and the
once-per-App/width receiving batch; all prior failed checkpoints stay recorded.

Frozen reader-repair complete units finish cleanly: x64 546/546 in 225.52
seconds, x86 546/546 in 238.32 seconds. Both use serial scheduling and unchanged
300-second aggregate containment. The final once-per-App/width integration
batch follows these complete runs, with no concurrent unrelated compilation.

### E3 Similar-Path Source Recheck — Retain Data ECC

The candidate concern that E3 must bypass payload ECC is rejected before any
production edit. The August 1982 supplement (same pinned SHA-256) explicitly
describes checking both ID and data fields for sector zero of every track;
its Drive Diagnostic opcode heading is already documented as inconsistent
with the selected IBM grammar and must not change IBM opcode admission.
The August 1983 Rev C1 manual, SHA-256
`790793DB296587AEF55D0702522A258BC2717FC5B57C9ECCB01350C0B6F393A8`,
printed 29 section 4.5.20, requires verification of sector zero across tracks.
The sector format on printed 30--31 separately identifies identifier and data
check fields. The former ledger wording could be mistaken for an ID-only
restriction; that interpretation is unsupported.

Retain the existing E3 logical-access/read loop and its approved data-ECC
arithmetic. It still does not implement a raw physical ID/check field, STEP
circuit diagnosis or original mechanical diagnostic duration; those remain
the explicitly approved model bounds. Data error detection cannot be labelled
proof of physical identifier ECC. No new callback, record field, state or
production branch is needed. Existing E3 query/read-failure, track enumeration
and no-write receivers plus the one transfer/check implementation remain the
relevant evidence; no weakened diagnostic success constant is introduced.

### S4 Final Qualification And Executor Review

The final frozen reader-repair batch passes all eight groups without a retry
within the batch or budget change: 32/32 x64 and 32/32 x86.

| Selected App | x64 | x86 |
| --- | --- | --- |
| NXVM | 22/22, 64.51 s | 22/22, 77.02 s |
| My5160 | 2/2, 28.77 s; boot 28.35 s | 2/2, 36.26 s; boot 35.87 s |
| My5170 | 4/4, 47.22 s; boot 46.13 s | 4/4, 59.72 s; boot 58.96 s |
| MyDeskPro386 | 4/4, 143.93 s; boot 142.97 s | 4/4, 178.20 s; boot 177.38 s |

Complete units are 546/546 x64 and 546/546 x86 on this frozen code. Core
source/test manifests, whitespace and documentation governance pass; all
eight optimized stripped PC EXEs have matching deployed/build-output hashes
and successful architecture checks. The four INIs retain their baseline
hashes. Shared-six, MyNES and external masters have no worktree change.

Using `git diff --numstat` against admission HEAD, counted C/H code changes
are production 16 files +1589/-341 (net +1248), test 35 files +3795/-250
(net +3545), combined +5384/-591 (net +4793). Two build CMake paths add four
and remove seven lines (net -3); two manifests replace 46 hash lines. These
exclude supporting documents, generated files and eight artifact paths.
The increase implements the finite 22-row set of missing command/timing/media
facts and its direct/receiving/failure matrices, not a second backend,
controller implementation, queue, clock, renderer or product workaround.

Executor self-review inspects actual Core/FDC/HDC/media/Board changes,
callback/enum extensions and all consumers, native-create/record/format/read
failure boundaries, IRQ/DMA/deadline wiring and the repaired receiver ownership.
No task-scope defect remains undisposed in the 22 rows. Physical raw-track/ID
checks, ECC wire encoding and diagnostic circuit/mechanical durations remain
the explicit approved logical-model bounds, not claims of exact hardware.
The byte cadence and service deadlines remain distinct; copied media facts
have one installed-lease owner; Compaq's two instances share the existing
scheduler and combined Board IRQ source.

The similar-path scan explicitly transfers one presentation contract to S5:
Core's driver currently copies a ready cached graphics frame even when no
new generation exists. Its cache-valid fact must not substitute for pending
delivery; S5 must qualify first/reset/paused publication and actual producer
wiring without adding a Shared interface or parallel renderer. The proposal
records that bounded receiver. Compaq speed/AUX-PIT/HOLD remains S6 as before.

DeskPro x86 still has a narrow host-time margin; one qualifying pass is not
a performance guarantee. No interactive desktop acceptance, universal physical
ECC correction or full-tree/no-bug claim is made. This is executor-complete
qualification evidence, not yet a pushed implementation or coordinator
acceptance. Those remain separate gates before S4 closure.
