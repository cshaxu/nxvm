# T539 S10: FDC Extraction Decision Record

Baseline 02931b886. This is a prerequisite characterization, not a migration or
full 8272A qualification. Current owns admission. Production remains unchanged.

## Verified Sources

The owner archive `manuals-nxvm/fdc/intel-8272a-floppy-disk-controller-1982.pdf`
has SHA-256 `C03A1FABE42FE43BB47FBD84042840F7B78B1DDD03E8A117FC7059C3453CC398`.
It is the Intel 1982 preliminary 8272A scan with an OCR text layer. S10 reads
PDF pages 9-11 and visually checks page 11 (printed 6-234). OCR is navigation,
not an authority for numerical bit positions. Existing acquisition provenance
remains in the FDC audit records; S10 acquires no new original.

Page 11 explicitly permits four parallel seeks, requires NR on loss of READY
at seek entry or during seek, requires SIS after completion, and describes
RECALIBRATE's Track-0 test and 77-step limit. It also defines SRT/HLT/HUT at
8 MHz, with doubled durations at 4 MHz. These facts contradict the current
comment that SEEK reserves Not Ready for read/write. Pages 9-10 distinguish
data availability, record search, service bounds, scan and format semantics.
An upper service bound is not by itself a byte arrival-period formula.

Read-only cross-checks, no copied source:

- PCjs `machines/pcx86/modules/v2/fdc.js`, checkout
  `c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70`, command completion and not-ready
  handling: the DeskPro IRQ exception is explicitly described as a workaround
  without hardware confirmation. It cannot establish real board READY wiring.
- 86Box `src/floppy/fdc.c`, checkout
  `4fef696a4eead1d55a28d6ac0e5bd2864e5454da`, command dispatch: sector operations
  go to the drive layer; recalibration samples drive presence, motor and Track0.
  This corroborates the responsibility split, not every timing or variant.

## Complete Command And Input Inventory

The existing command switch has 15 defined operations plus invalid encodings.
The new repository-only matrix tests ten data/result operations, each with
normal input, absent medium, motor off, mismatched DOR select, READY mask zero
and installed mask zero, under both old policies: 120 cases. It checks phase,
IRQ, DMA request and error-result withdrawal through ports. Fixtures reset and
re-arm DMA between cases; a previous TC mask is not a controller failure.

| Operations | Current admission/result; extraction disposition |
| --- | --- |
| READ DATA, READ DELETED | Selected DOR + NRS + motor + medium. Missing input gives 40/04 or DeskPro 48/00 ST0/ST1. Remove policy after neutral READY/media separation. |
| WRITE DATA, WRITE DELETED | Same admission, always 40/04; write protection is checked on transfer. Use the same READY/record contract as read. |
| SCAN EQUAL, LOW/EQUAL, HIGH/EQUAL | Same admission, always 40/04; mark/compare logic remains one chip owner. Recheck SH/SN bit definitions against rendered manual before moving. |
| READ TRACK | DMA, drive zero, opcode 42, N=2 and exact geometry restrictions; entry error 40/04. Mid-transfer shares the ordinary read path and its DeskPro special case. Keep limitations explicit until separately implemented. |
| FORMAT | Entry uses drive-ready predicate; header/data failures use media results, without the same later READY check. One command owner; drive supplies record/format operations. |
| READ ID | Successful seven-byte IRQ result, missing input 40/04 regardless of policy. READY and record-not-found must be separate inputs. |
| SPECIFY | Three timer fields, NDMA and polling enable; no result. Existing smoke covers configuration and service cadence. Chip owns registers, board converts qualified durations. |
| SENSE DRIVE STATUS | ST3 READY from frozen mask, Track0 from installed/cylinder/inversion, WP from medium, two-sided from binding. Existing topology tests cover these; replace with one copied pin sample. |
| SEEK, RECALIBRATE | Existing smoke covers cadence/parallel completion, but completion uses the latest command byte and current DOR select. Repair per-operation ownership before extraction. |
| SENSE INTERRUPT | Reset mask, seek results, then scalar interrupt; two bytes even if none. Existing reset/seek tests cover draining. Audit no-pending length and arbitration against manual; do not bless the baseline. |
| Invalid encoding | One 80h result, no new IRQ. Existing command-family smoke retains coverage. |

Additional inputs/paths are not interchangeable:

- `drive_media_ready` and ready-change polling query media presence, not READY
  wiring. Both insertion and removal currently store C8 plus unit.
- `drive_ready_for` checks DOR selection/reset/motor and media, but ignores
  installed and READY masks. The new matrix confirms those omissions.
- `drive_mechanical_ready_for` checks installed/reset/selection, not motor,
  although its comment says otherwise. Track0 inversion affects ST3 but not
  RECALIBRATE completion.
- DIR uses motor, installed and media generation; seek clears observed change.
  It is board/drive state, not an 8272A register.
- DOR reset cancels command, IRQ, DRQ and byte gates. Changing selection/motor
  during transfer can cancel without a result. Existing cadence/reset tests
  cover cancellation; the cutover must specify pin loss versus explicit reset.
- DMA TC, NDMA byte gates and terminal observation share completion state.
  Existing smoke retains deleted-mark, scan, format, readonly/media errors,
  multi-sector TC and result IRQ acknowledgement. No duplicate sector cache.

## Concrete Ownership And API Contract

The intended `x86/devices/fdc8272` instance owns command/result bytes and phase,
Specify registers, status construction, PCN, per-drive pending seek operation,
completion causes, transfer CHRN/offset/remaining, scan/deleted/format state,
IRQ/DRQ levels and service deadlines. Its only dependency is Types.

NXVM owns DOR/DIR/CCR and diagnostic ports, unit/motor/reset decoding, clock
conversion, drive installation/mechanics, media registry/generation/change
latches, PIC/DMA routing and terminal-observation publication. Physical head
position and controller PCN are distinct facts, not two mirrors of one value.
Only the drive changes physical position; only the chip changes PCN.

The public surface must provide opaque create/destroy/reset; status/data byte
read/write; DMA byte service and TC; advance and next-event query; copied IRQ/
DRQ outputs. A borrowed drive provider samples READY/Track0/WP/two-sided/fault,
accepts a step direction, and performs bounded record query/read/write/mark/
format operations. Results distinguish absent record, protected media and
provider failure. No board name, port number, media-registry pointer, machine,
file handle, scheduler, generic device framework or mutable state getter.

One machine execution owner invokes every operation. Provider calls are
synchronous and cannot re-enter advance/reset. Borrowed contexts outlive the
chip; board routes and DMA bindings are withdrawn before destruction. Failed
construction unwinds registrations before freeing their contexts. Reset
preserves frozen bindings and explicitly releases IRQ/DRQ. Copied terminal
results are published once by the adapter, with no second result authority.

The board supplies durations in one declared scheduler unit, derived from
the qualified chip clock/rate or explicitly labelled existing logical model.
No host-clock advancement or new precision claim. The earliest command,
completion, seek and byte-service event must be queryable, replacing
`machine_scheduler.c`'s private `fdc.data.phase` checks. A due event and no event
cannot share a sentinel meaning; earlier deadlines cannot be skipped.

## Mandatory Repair Gate Before Cutover

1. Replace the shared command-byte dependency with per-drive seek kind/PCN/
   completion state. Apply SIS admission rules and bounded pending causes;
   do not make the four-element arrays into an unbounded FIFO. The current
   append has no bound check, so malformed guest sequences are a safety risk.
2. Resolve READY/Track0 against Intel for every command, reset, mid-operation
   input change and parallel drive, not just the first boot failure. Test
   interrupted commands and fifth undrained completions without permitting
   out-of-bounds accesses. The current characterization is not the new oracle.
3. Delete `DESKPRO_REFERENCE` from the future chip contract. PCjs's workaround
   proves no electrical wiring. A source-qualified board READY model may be
   selected only with evidence and whole-profile verification; do not rename
   the switch, patch status after completion, or infer physical READY solely
   from whether the BIOS boots. Until that decision is proven, FDC relocation
   remains gated, not accepted or silently degraded.
4. Reconcile status bits, unavailable-command outcomes, READ TRACK limits and
   service-time provenance before preserving them. Existing successful boots
   do not override manual contradictions. Qualification changes must be
   explicitly reported, not hidden inside file moves.

This prerequisite can finish with explicit repair obligations; it cannot
close the FDC ledger row. The next automatic S must repair the bounded pending
seek/completion class first, then revalidate the pin/drive contract before
production extraction. HDC/video/CPU work does not excuse an unresolved FDC.
