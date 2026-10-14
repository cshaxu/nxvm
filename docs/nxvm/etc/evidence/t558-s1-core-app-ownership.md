# T558 S1 Core And PC App Ownership Audit

## Baseline And Limits

Read-only production inspection at `e2cb3a692`, requested by the owner while
planning PC110. Examined four App source inventories, build declarations,
bindings, construction and firmware paths; Core construction, AT preparation,
board attachment, media setup, profile contracts and dependency gates.
This is a boundary audit, not a complete instruction/device correctness audit.
No production, tests, assets or Shared-six files change in this delivery.

## Existing Four Apps

- My5160 owns its fixed XT configuration, firmware slots/mapping and prepared
  construction. It consumes Core XT/PPI and common devices, not another App.
  Its distinct firmware preparation is justified by its ROM layout and optional
  Xebec/video roles; do not force it through the AT ROM helper.
- My5170 owns its IBM board constants, constraints and ROM selection. Its
  `machine_plan.c` delegates common preparation to
  `core/machine/pc_at_preparation.c` and supplies the fixed board values.
- NXVM follows the same preparation path but owns its configurable default
  board and project-authored `firmware/`. Its BIOS is guest software, not a
  Core service and not the firmware for the other machines.
- MyDeskPro386 owns D4 memory decode, refresh/Port B/NMI behavior and selected
  Compaq board values. `machine_construction.c` reuses AT route materialization,
  then installs its profile factory. D4 storage is App-private; the published
  board attachment owns destruction before borrowed Core/PIT providers die.
  More App code here reflects real hardware differences, not automatically
  duplication. Retain the approved flat App layout.
- All four bindings inject `prepare`; all four link Core rather than each
  other. App-to-Core include inspection found public `_interface.h` use, not
  imports of Core private layouts. Core/chips and Core/x86 include searches
  found no outward board/machine/product imports.

## Concrete Risks And Dispositions

1. **AT preparation is a constrained helper, not every PC's constructor.**
   `pc_at_preparation.c` accepts one fixed-size system image or two interleaved
   halves and an optional video ROM. `input_interface.h` exposes two BIOS
   slots, video, font and CMOS roles. PC110 chipset shadow, font banks and
   controller firmware cannot be renamed into these roles. The board receiver
   will use the existing construction/profile lifetime and an App-owned typed
   firmware context, extending only a proven missing embedding boundary.
   No generic ROM registry or runtime file fallback is planned.
2. **Machine currently presumes a first floppy.**
   `machine_devices.c` allocates and binds slot zero unconditionally; only the
   second slot is conditional. `construction_begin()` rejects a second fixed
   disk image. These are existing supported-topology constraints, not evidence
   of a current four-App failure. The storage receiver must represent the
   selected PC110 fitted/absent media accurately and distinguish socket/card
   hardware from inserted media. Generalize the one existing path only where
   that topology requires it; do not create phantom media or a second manager.
3. **Shared AT metadata retains firmware-policy coupling.**
   `pc_at_profile_interface.h` still names an IBM-5170 firmware-slot enum and
   firmware-service descriptors; `pc_at_profile.c` owns the service table and
   validates `fdc_bounce_segment`. Source search found service data copied and
   checked by profile validation, not a live BIOS-interception dispatcher.
   Do not misreport it as host BIOS emulation. The board receiver must audit
   source and test consumers before deleting obsolete metadata or moving
   genuinely App-specific expectations to their App tests. Do not extend this
   contract with PC110 BIOS services. Existing `default_pc_at` type names do not
   by themselves prove duplicate implementation or require a mass rename.
4. **Character-generator assets are not arbitrary font ROMs.**
   `preparation.c` converts a fixed two-plane eight-row asset into character
   glyphs. That remains a distinct existing contract. PC110 font-ROM bytes and
   banking belong to its actual mapped hardware, not this conversion or a host
   text replacement. The display receiver validates the single resulting frame
   path through Core Machine and Emulator UI.
5. **A small stale comment remains.** My5170's CMake header says shared
   mechanisms remain in `ibmpc`; actual links use Core. Correct it in the board
   receiver's existing-App boundary cleanup, without changing working behavior.

## Proposed PC110 Ownership

- `core/chips/cpu`: complete admitted 486SX instructions, exceptions, reset,
  feature selection and timing. CPU behavior never tests the PC110 App name.
- `core/chips`: independent VL82C420, F65535, RB5C396, Super-I/O or audio chip
  mechanisms belong here when their actual identity and bounded contracts are
  established. A second consumer is not required for a genuine chip boundary.
  Candidate chip names from the research ledger are not yet qualified models.
  Integrated peripherals reuse appropriate existing device instances through
  one composition owner, not duplicated PIC/PIT/DMA/RTC state.
- `core/x86`: retain the sole CPU/bus/memory/timeline executor. New mappings,
  deadlines and DMA use its existing ownership; no PC110 execution loop.
- `core/board-base`: reusable routing, ROM mapping, clocks, media/display
  providers and construction lifetime. `core/board-at` contributes only genuinely
  compatible AT wiring. PC110 is not a selection flag inside the default AT
  descriptor, and does not require a new Core board-pc110 directory by default.
- `core/machine`: retain the single Emulator Machine driver, lifecycle, pacing,
  copied input/frame conversion, paused-debug access and storage ownership.
  Add topology capabilities only when needed by the selected machine.
- `core/product`: retain common PC version/copyright, startup, INI grammar,
  factory and PC extensions. Accept App identity/configuration/firmware binding;
  do not branch on `mypc110`. Shared Emulator and x86 Product stay unchanged
  unless a separately reviewed real contract gap is established.
- `app-mypc110`: fixed board revision and CPU selection, memory limits, fitted
  devices, address/IRQ/DMA/clock wiring, reset order, LCD characteristics,
  sockets, firmware roles and hashes, and board-specific glue. Bowman/Pluto,
  MCU board protocols and power wiring remain here unless research establishes
  an independent chip mechanism; do not prematurely invent generic frameworks.
  Raw firmware stays external; App declares embedding, and guest hardware reads
  the embedded immutable bytes. No BIOS address hooks or native DOS services.

Start with a flat App like DeskPro, with cohesive files such as `machine_plan`,
`machine_config`, `firmware_map`, `product_binding`, and hardware-named board
files only when implemented. Do not pre-create empty chips/machine/product
subdirectories or allocate one file per register. The exact filenames are a
construction-task decision, not additional APIs approved by this audit.

## Tests And Queue Integration

Chip semantics stay in the existing `test/core/chips` unit owner; executor behavior in the Core
x86 owner; reusable board and driver contracts in their Core unit owners.
PC110 wiring/reset/firmware-map tests belong to `test/app-mypc110/unit`, and
external-ROM POST/guest tests to its integration owner. Shared Core fixtures
stay under Core; no sibling App fixture imports. A chip unit pass is not proof
that PC110 connected it correctly.

The existing nine queued receivers remain sufficient. CPU receivers own the
486 boundary. The board receiver owns risks 1, 3 and 5 and the initial App;
storage owns risk 2; display owns risk 4. Input/power and audio own their chip
versus board boundaries; firmware/qualification validate the full connection.
T558's next contract-freeze step must resolve each hardware identity and owner
before its implementation packet. This audit neither admits code cleanup now
nor blocks CPU work on cosmetic AT metadata cleanup.

## Verification

Core source corpus and source/test manifests pass. The repository production
dependency-DAG check passes. Searches inspected App include boundaries and Core
outward references; these checks do not prove all failure/lifetime behavior.
Documentation structure and actual diff are reviewed for this delivery.
No dynamic tests, desktop/RDP checks or new builds were run: executable inputs
are unchanged, and the current ten artifacts remain unchanged. T558 stays open.
