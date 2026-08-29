# T501 S2: Model-40 Firmware-Visible Diagnostic List 1

`T501-S2-MODEL40-DIAGNOSTIC-LIST-1`

## Scope And Evidence Boundary

This is the finite diagnostic path exposed by the owner-authorized DeskPro
386/16 ROM during the T501 S1 1.2-MB replay: POST configuration, FDC drive
selection/status, `601` publication, and the visible `RESUME`/keyboard branch.
It is not a replacement List 1 for the 8272A, keyboard controller, RTC, D4
memory controller, or display controller; their chip-local ledgers remain the
authorities for their own behavior.

The Compaq *Deskpro 386 Maintenance and Service Guide*, chapter 5, is the
normative source for the `601` meaning.  Its POST table defines `601` as a
diskette-controller error; its advanced diagnostics table separately assigns
the `600-xx` through `610-xx` family to concrete drive/controller tests
(including format, read, write/compare, seek, ID/media, speed, reset and
change-line).  It describes `RESUME` as the F1 continuation branch.  It does
not define an absent-drive ST3 value, drive-B readiness, the exact ROM test
sequence, or a board timing formula.  The owner-managed Technical Specification
was visually checked at its floppy/serial/printer material: it identifies the
option-board function but supplies none of those missing relations.

External source was inspected read-only and is corroboration only:

- 86Box `src/machine/m_at_386dx.c` selects the 1986 DeskPro ROM, installs the
  ordinary AT FDC, then installs the Compaq memory controller; `src/chipset/
  compaq.c` owns the high-memory remap.  It does not specify a DeskPro POST
  drive-ready override.
- PCjs `machines/pcx86/compaq/deskpro386/ega/2048kb/machine.xml` selects the
  1986 ROM, declares two 1.2-MB drive *types*, mounts media only in A:, and
  declares B: `None`.  Its ROM notes also identify this as a recovered early
  ROM configuration, not an original controller specification.
- MAME `src/mame/pc/at.cpp` supplies a generic AT/386 ISA FDC topology, not a
  DeskPro-386 board model.  QEMU `hw/i386/pc.c` derives generic-PC CMOS floppy
  fields from FDC drive types, while `hw/block/fdc.c` is an 82078 controller.
  Neither is a DeskPro-386 diagnostic authority.
- The local Bochs 2.6 compatibility tree contains no DeskPro-386 machine
  model.  It cannot corroborate this board relation.

No external implementation is imported, transcribed into product behavior, or
used to manufacture a requirement.

## Complete Selected Relation Ledger

| ID | Firmware-visible relation | Normative basis | Cross-check | Level | Required observation / boundary |
| --- | --- | --- | --- | --- | --- |
| D1 | The selected configuration has one 5.25-inch 1.2-MB drive. | Technical Specification, floppy option description and T499 visual reread. | PCjs represents two type slots but only mounts A:. | L3 configuration fact | A: is the one configured/media-backed drive; a second ready drive is forbidden. |
| D2 | `601` is a diskette-controller error, not a generic request to add a drive. | Maintenance and Service Guide POST error table. | No external model is authoritative for the diagnostic. | L3 | The rendered `601` is a real diagnostic surface requiring an upstream owner classification. |
| D3 | `600-xx`--`610-xx` diagnose controller/drive operations including reset and change line. | Maintenance and Service Guide advanced-diagnostics table. | 86Box/PCjs use an AT-style FDC; neither defines Compaq's POST result. | L3 | FDC command/result, ready and media-change relations must remain observable; no success shortcut. |
| D4 | `RESUME` is the firmware's F1 continuation branch. | Maintenance and Service Guide POST guidance. | No machine-specific external implementation found. | L3 | Scan-set-2 F1 reaches firmware only after ROM publishes this branch. |
| D5 | FDC drive selection and ST3 READY/TRACK-0 are controller-owned observations. | Retained uPD765/8272A List 1; this board source does not override it. | PCjs mounts B: `None`; generic FDCs remain distinct implementations. | L3 chip relation | An absent/empty B: may not be declared ready without an original-board basis. |
| D6 | A media mount/change is an external input, not a controller clock or a ROM patch. | Existing Core media/FDC contract. | QEMU's generic FDC and PCjs configuration retain separate media configuration. | L3 owner relation | Only the Core media registry can alter mounted-media state. |
| D7 | The 1986 ROM is mapped and immutable to Core after validated VM selection. | Existing Model-40 BYOB profile contract. | 86Box and PCjs both select a distinct early ROM image. | L3 owner relation | VM supplies frozen bytes at construction; no firmware rewrite or mailbox. |
| D8 | POST-visible FDC/DMA/PIC/IRQ state progresses through Core's sole timeline. | T499 scheduler closure and T501 S1 replay. | 86Box/MAME/QEMU have their own schedulers; no behavior is derived. | L3 current architecture | A ROM diagnostic cannot create a profile time path. |
| D9 | Keyboard F1 is delivered through the existing Core KBC scan-set-2 path. | Existing KBC contract and Maintenance Guide's F1 action. | External machine models do not define the ROM's precondition. | L3 current architecture | Input cannot bypass Core or clear a diagnostic before `RESUME`. |
| D10 | Displayed POST text is a copied VADP snapshot, not VM-maintained firmware state. | VADP owner boundary. | External renderers are not an owner. | L3 current architecture | VM presentation only consumes the copied frame. |
| D11 | Exact POST test order, expected ST3 for unit 1, and the electrical relation from FDC/drive-change to `601` are not specified by available original material. | Explicit primary-source gap. | External implementations disagree in machine scope and cannot close it. | Source-gated | No L3/L2 value, second-drive behavior, or F1 shortcut may be invented. |
| D12 | Physical motor/ready/change-line delays and their board clock conversion are not present in the selected source. | Explicit primary-source gap. | Generic emulators implement their own timing assumptions. | Source-gated | No physical timing claim or guessed delay belongs in Model-40. |

## S3 Receiver

Rows D1--D10 already have the stated owners.  D11--D12 are one indivisible
source-gated **Model-40 FDD/ROM diagnostic relation**: acquire original board
schematic/service timing or a source-backed ROM diagnostic description before
admitting any owner-local repair.  It is not a Core FDC, VM, keyboard, display,
or scheduler repair batch under the present evidence.  T501 S3 transfers that
exact class to [the bounded debt entry](../../states/TODO.md#hardware-and-compatibility-debt).
