# Fixed-Machine Selection And Simplification Evidence

Research date: 2026-09-21. Supporting evidence, not a second architecture or
machine-selection authority. Baseline: `b03f827b` after T532. No production,
test, build, session YAML or executable was changed by this research.

## Selection Result

No researched candidate is yet proven to meet all four requirements at once.
Intel Model 302 is the strongest primary-document/Intel-component lead;
AMI 386XT Series-4 is a useful alternative with identified BIOS dump leads.
Neither is frozen as Standard. First obtain one matching complete hardware/
firmware set; do not delete the runnable baseline before this gate.

### Intel386 MicroComputer Model 302

The original Intel technical reference, order 505067-002 Revision F (January
1990), documents a 25 MHz 386 board, 82C54 timer, dual PIC/DMA, 1287 RTC,
firmware/CMOS and board interfaces. PDF page 42 (printed 1-7) was visually
checked against OCR: it names 82C54, two PICs, two 8237s and 50 CMOS RAM bytes.

This is not a pure discrete-chip AT: CAT, DBC and RIO ASICs, cache and AT32
require board-specific work. The manual describes Phoenix firmware and two
27256 EPROMs. It does not supply the firmware bytes or a matched VGA card.
The inspected Intel 30x archive page had no downloadable BIOS. A family name
or similar board image is insufficient proof of compatibility.

- [Intel original](https://bitsavers.trailing-edge.com/components/intel/80386/Intel_386_Model_302_Board_Technical_Reference_Jan90.pdf)
- [Archive lead, not an approved firmware source](https://theretroweb.com/motherboards/s/intel-30x-system-board)
- External archive: `nxvm-assets/manuals/standard-candidates/intel-386-model-302-board-technical-reference-jan1990.pdf`
- SHA-256: `376375438F954A51040486E8DE7BEF66025F1C5B382E05F8CB0F23F489EB70E3`

### AMI 386XT Series-4 / Atari PC5

The original AMI-386 BIOS PLUS/manual archive includes motherboard schematics.
The owner's Atari archive identifies four PC5 AMI BIOS chip images. MAME's
`ataripc5` ROM definition provides a cross-check of four interleaved chips,
but its source marks the machine not working and notes refresh/blank-screen
failure. This is an asset/document lead, not proof that another emulator
already runs the machine correctly. A specific display card/ROM and storage
adapter still need a matched, documented configuration.

- [Manual original](https://www.ataripc.net/wp-content/uploads/Atari-PC5-Manual-English.pdf)
- [Dump provider's BIOS identification](https://www.ataripc.net/biosrom-images/)
- [MAME implementation, inspected 2026-09-21](https://raw.githubusercontent.com/mamedev/mame/master/src/mame/pc/at.cpp)
- External archive: `nxvm-assets/manuals/standard-candidates/atari-pc5-ami386-bios-plus-manual.pdf`
- SHA-256: `20A9513809B5C62EC66388FF83C34E348028191BA43179CEBFB7D7CE8586078C`

Both original PDFs were acquired on the research date outside the repository.
OCR text is a search aid, not proof of schematic details. No vendor ROM payload
was acquired or imported by this Td.

### PC110

The read-only Open-Source-PC110 checkout inspected at
`deb665f1b31c946b918f03850813745c406f403c` contains component references and
reconstruction work. The PC110-EMU checkout at
`81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c` labels its display and chipset
configuration as placeholders. They are research leads, not IBM authority or
proof of PC110 boot compatibility. Original manufacturer sources must be
separated from reconstructed material.

- [Reverse-engineering source](https://github.com/ahmadexp/Open-Source-PC110)
- The selected CPU must be qualified as the actual PC110 486-class variant;
  a 386 CPU label is not sufficient.
- Expected investigation includes VLSI chipset, display/LCD, flash/ROM,
  PCMCIA/storage, keyboard/pointer and power. Freeze exact identities and
  revisions before any completeness claim.

## Asset Rules For Selection

Select one motherboard revision, CPU, video adapter, storage adapter and drive
topology. Record required BIOS/video/keyboard/option-ROM slots separately;
some functions are integrated and may not have an independent ROM. CMOS is
writable state initialized by a documented seed, not executable firmware.
Do not apply a universal 64/128-byte size without the selected device contract.

Use the original manual, exact chip images/layout, dump provenance and a
reference implementation as separate evidence. A manual without a BIOS, a ROM
without board identity, or a runnable reference without register/timing evidence
does not close selection. Non-Intel support parts in a Standard candidate
must be explicitly reconciled with the owner's component restriction.

Machine-specific C and ROM mapping declarations belong to the two profile
owners. Vendor payloads and their local manifests remain in external
`nxvm-assets`; no protected BIOS is committed or silently synthesized.

## Code-Level Opportunities

Inspected live source, not inferred solely from directory names:

| Current surface | Observed cost | Target disposition |
| --- | --- | --- |
| `src/app/config.c` | Machine-name parser plus per-board ROM/CMOS/media/CPU restrictions. | App parses values; fixed profile owns hardware constraints. |
| `src/core/profile/profile_resolver.c` and header | Recursive parent traversal, field masks, runtime owner strings and copied resolved values. | Direct frozen plan; retain generic conflict validation. |
| `src/core/profile/default_profile/pc_at_profile.c` | A 5170 parent is assembled to derive default AT; descriptors and resolved snapshots coexist. | Two independent complete board declarations, no retired-parent inheritance. |
| `src/core/machine/machine.c` | Separate XT/Model-40/PC-AT create and asset branches. | One resource and publication transaction, board facts supplied once. |
| `src/core/machine/machine_private.h` | Multiple resolved boards, ROM buffers, firmware-kind and private flags in one object. | Selected-board lifetime only; no replacement mega-union. |
| `src/core/machine/model40_composition.c`, `lifecycle.c` | Compaq wiring in generic adapter, reset/provider selection by machine kind. | Retire unselected wiring and switches together. |
| `src/core/core/vadp.c`, `hdc.c`, `fdc.c`, `d4_memory.c` | Compaq/DeskPro personalities coexist with reusable mechanisms. | Dependency-led removal; preserve shared video, disk, timing and memory owners. |
| `src/core/machine/media/` | Device-semantic adapter over Lib Storage. | Keep needed geometry/change semantics; do not duplicate file storage. |
| `src/core/core/cpu_interface.h` | Actual enum: DEFAULT, 8086, 8088, 80186, 80286, 80386. | Retain all actual models; 80188/486 are not currently implemented enum entries. |
| `src/app/composition.c`, `core/machine/lifecycle.c` | Existing Common driver/session/UI binding. | Keep one shared route; do not add product-specific queues or executors. |

The audit identifies replacement opportunities, not numerical code savings.
Implementation must count actual deleted/added lines and prove that removed
paths have no retained consumer. CPU retention overrides machine retirement;
old-machine tests containing generic regressions must be rehomed first.

## Governance Verification Boundary

The principal design documents are target requirements. The current runnable
0532 baseline is unchanged. Documentation validation and actual-diff review
verify this delivery; no new boot, unit, integration or artifact result is
claimed. Implementation and final board selection remain queued.

Closure review: the full documentation governance gate and its self-tests pass.
The checker now recognizes PC110 instead of retired NXVDM UX; self-test fixtures
also match the existing dual-host-architecture artifact and packetless-closure
rules. No runtime predicate was relaxed. All 43 archived proposals were compared
with their Git originals after normalizing links, the retirement notice and
punctuation: no unexpected prose change. Historical facts remain intact.
`git diff --check` passes; `src/`, `test/`, CMake and session assets have no diff.

The documentation/architecture governance skills guided the single-authority
split and explicit retirement map; coding governance applied only to the
checker schema and stale self-test fixtures. This Td creates no new executable
and does not reopen the closed layout task or admit a numeric implementation T.
