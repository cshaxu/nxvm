# M3 T11: Interactive Dual KVM Vertical

## Admission

The owner approved automatic M3 execution admission after T10 closure on
2026-09-21. T11 consumes the first ordered M3 candidate. Its eventual outcome is
an original NROM fixture controllable through both Window pixels and Core-created
Console text, with safe pause, monitor return and representation switching.
This history records the active task's evidence; the active S contract remains
the sole packet in [Current](../states/CURRENT.md).

## Frozen Task Convergence Inventory

T11 consumes G01; initial G02-G04/G07-G13; and remediation R01-R05/R08-R10,
with R06/R07 established rather than closed. The proposal's later receivers
remain explicit transfers: sprite/precise PPU/DMA timing to PPU completion;
full controller/command-state coverage to Input; native/conversion qualification
to Presentation. No task may claim their completion here.

S1 consumes this finite mechanism batch before graphical work:

| Row | Required disposition in S1 |
| --- | --- |
| R01/R02 | Remove App command/debug's Core-driver runtime coupling; composition is the sole integration owner; verify include/link and real command behavior. |
| R03 | Establish one immutable Core-owned opcode value definition for CPU decode and copied disassembly; reject duplicated production metadata. |
| R04 | Make composition callback storage and construction/teardown ownership safe across every construction stage and failed machine/UI join. |
| R05 | Deliver command-line/path and pending transaction essentials; full command-state matrix transfers to Input. |
| R08 | Construct before publish and preserve valid output semantics; PPU device reset timing transfers to PPU completion. |
| R09 | Prevalidate or stage PEEK so failed ranges leave all output untouched. |
| R10 | Initialize CHR RAM and make cartridge candidate allocation/read/close failure transactional. |

Every row has positive, rejected-input and failure-cleanup proof through the
production path. A new variant of these mechanisms enters this table before
acceptance; unrelated discoveries go to the appropriate queued receiver or
TODO with an admission path. The active S must not create an audit-only result.

## S1 Delivery Boundary

S1 delivers a usable safety and transaction foundation, not merely planning:
existing ROM insertion/ejection, cooked management and copied paused debug
remain functional while their ownership/lifetime paths are consolidated. It may
change product code, tests and build visibility required for that outcome. It
does not add PPU/controller/KVM code, a second machine lifecycle, a shared fork,
or a provisional renderer. Subsequent S work is dynamically admitted only after
S1 evidence establishes the next bounded implementation obligation.

## S1 Implementation Record

S1 changes the live `mynes-0-1-0011` path. `app_command_context` retains only
the accepted cartridge fact and Common machine handle; it cannot receive a Core
driver. `app_composition` is the only App structure that owns that driver and
now lives on the heap for the entire callback interval. It shuts down the
machine before releasing UI, session, machine or driver. A failed shutdown or
UI/session destruction returns terminal failure while retaining the complete
borrowed graph, so a live callback cannot observe freed storage.

The command protocol now distinguishes an accepted request from its completion
fact. A reset, resume, pause, stop, or accepted media replacement becomes
pending until Common reports completion; only `help`, `status`, and `quit` are
accepted while pending. Candidate media failures preserve the previous accepted
fact. ROM paths accept at most 1023 printable ASCII bytes (with existing quoted
path handling); control bytes, non-ASCII input and longer paths are rejected.

Core now owns a single immutable 151-form opcode table through
`core_opcode_describe`. CPU decode consults it before execution and App
disassembly receives only copied mnemonic/mode/length values. The public debug
wire definitions similarly live in `core/debug_interface.h`. `core_machine`
does not publish a candidate until cartridge construction and power reset have
succeeded; warm reset preserves elapsed instruction/cycle accounting, and PEEK
prevalidates its complete range before copying. CHR RAM is allocated zeroed.

| Row | S1 delivered proof | Remaining receiver |
| --- | --- | --- |
| R01/R02 | Command no longer includes or calls driver APIs; only composition has the driver include/link. Command, media and debugger integrations pass. | None for this mechanism. |
| R03 | One Core metadata table serves CPU acceptance and copied disassembly; the 256-value decode-ledger test checks supported and rejected bytes. | PPU work may add hardware metadata only when required. |
| R04 | Heap composition owns all callback storage; construction and early media failure unwind through the production entry point; teardown order quiesces the producer first. | Native presenter teardown qualification remains Presentation. |
| R05 | Path boundary/control-byte checks, accepted media fact and pending completion behavior are direct App tests. | Full input command/state matrix remains Input. |
| R06/R07 | Composition resets input before paused/stopped/error completion facts; no PPU clock is claimed. | Full input drain/generation proof is Input; slot clock/PPU timing is PPU. |
| R08 | Candidate creation publishes only after successful reset; deterministic power/warm-reset accounting is covered. | Device reset timing and PPU MMIO cases remain PPU. |
| R09 | RAM-to-MMIO PEEK rejection leaves every sentinel byte unchanged. | None for this mechanism. |
| R10 | CHR RAM zero initialization is covered; existing Core storage-fault contract preserves the accepted machine on allocation/read/limit failures. | Native host I/O qualification remains Presentation. |

Verification at this checkpoint: GCC x64 configure/build, all 94 registered
CTest cases, documentation governance for `mynes-0-1-0011`, and whitespace
checks pass. x86 and stripped binary evidence is deliberately not claimed here;
R12 remains owned by the later presentation closure.

## S1 Acceptance And S2 Admission

S1 is accepted at local commit `21da610` after actual-diff review: the 25-file
change has no whitespace faults, no driver include/call outside composition, and
no second production opcode table. It adds 94-test x64 regression evidence while
leaving Lib/Common byte-identical. The only review correction was removal of a
trailing blank line before acceptance.

S2 is now admitted under the owner's automatic M3 execution authority. Its
bounded outcome is the single slot clock plus a Core PPU background image and
driver publication path. It consumes the initial clock/register/background
parts of G02-G04 and R07, and the device reset portion of R08. Sprite/OAM/NMI
edge timing remains PPU-completion work; controller, text conversion and native
KVM behavior remain later T11 continuations. The active S2 packet in
[Current](../states/CURRENT.md) is the controlling contract.

## S2 Frozen Hardware Matrix

The S2 implementation reads the registered NESdev PPU programmer and rendering
references. A fresh indexed consultation confirms the eight CPU-visible PPU
registers mirrored through `$3fff`, PPUDATA's buffered non-palette read, and
the NTSC 262-by-341 dot geometry with vblank beginning at scanline 241 dot 1.
S2 therefore freezes only these directly testable members: each CPU-visible bus
transfer advances three PPU dots independently of trace retention; `$2000-$2007`
mirrors route to one PPU object; `$0000-$1fff` CHR, `$2000-$2fff` mirrored CIRAM
and `$3f00-$3fff` palette accesses use the declared mapper-0 mapping; and an
enabled, unscrolled background produces complete 256-by-240 indexed samples.

Power/reset suppression, scroll-copy intervals, odd-frame shortening,
status/NMI races, sprites and OAM DMA are expressly not approximated in this S.
Their named PPU-completion receiver remains responsible for the corresponding
source-specific checkpoints.

## S2 Implementation Record

`core_ppu` is now private machine-owned hardware. Every CPU-visible bus read or
write takes one monotonically counted machine slot and advances that PPU three
dots before the transfer. This removes the former trace-capacity-derived clock
from the production PPU path. CPU `$2000-$3fff` mirrors route by their low three
bits to PPU registers; `$0000-$1fff` accesses cartridge CHR, `$2000-$2fff` uses
mapper-0 CIRAM mirroring and `$3f00-$3fff` uses palette aliases.

The admitted background renderer writes one 6-bit palette sample for every
visible dot using nametable tile, both CHR bit planes and its attribute-table
quadrant. At 262 by 341 dots it commits a complete 256-by-240 image revision.
The Driver publishes a copied graphics frame only for a fresh completed revision.
Media eject/replacement clears that publication watermark, so the first frame of
a replacement cartridge cannot be suppressed by a coincident revision value.

Direct evidence: `mynes.core.ppu-smoke` writes CHR, nametable, attribute and
palette data through the PPU register protocol and proves its expected first
pixel plus frame completion. `mynes.core.media-failure-contract-smoke` drives
the production media/CPU/Driver path to a copied 256-by-240 frame, verifies no
duplicate publication, then verifies a replacement cartridge publishes again.
The full GCC x64 build and 95 registered tests pass. This does not claim x86
or final presentation evidence; those remain R12/Presentation work.

## S2 Acceptance And S3 Admission

S2 is accepted at local commit `158a577` after actual-diff review, whitespace
and governance checks. S3 is automatically admitted as the next T11 continuation
to implement the controller serial path and Driver input ownership. The S3
packet in [Current](../states/CURRENT.md) controls its finite button/strobe
matrix; full source-generation and binding behavior remains the Input receiver.

## S3 Implementation Record

Core now owns an eight-bit controller with live state, falling-edge capture and
NES serial order A, B, Select, Start, Up, Down, Left, Right. `$4016` routes
strobe/read through that device; post-eight reads return one and `$4017` remains
the declared disconnected port. Driver maps copied default key facts to the
eight bits and consumes the existing input-reset latch before guest execution.
Direct controller and Driver-to-`$4016` tests pass; the full x64 suite contains
96 registered tests. Full source-ledger and configurable bindings remain Input.

## S4 Implementation Record

Driver now owns selected output representation. The existing versioned
`OUTPUT_SET` protocol accepts Window (0) or Console text (1), rejects malformed
reserved bytes without mutation, and invalidates the publication watermark.
Window continues to receive the indexed 256-by-240 image; Console receives only
a Core-created 80-by-25 ASCII text frame with a copied ASCII character map.
The same completed PPU samples drive both routes. Production regression verifies
Window-to-Console, rejected selection preservation and Console-to-Window; the
full x64 suite and governance pass.

## S4 Configuration And Identity Correction

The owner removed the App-facing `set video` requirement and renamed the product
MyNes. App now loads the fixed `mynes.ini` placed next to the executable before
constructing Common. It accepts an optional `rom = PATH` and
`video = window|console`; missing video defaults to Window. The parsed output
mode is copied into Core Driver creation, while live cartridge changes remain
`rom insert` and `rom eject`. The cooked monitor has no live presentation
selection command. The product banner and Window titles say MyNes; the approved
repository directory and internal build identity remain `mynes` for this task.
An unverified PPU-to-CPU NMI experiment was removed from this correction rather
than treating it as presentation evidence; PPU completion owns its measured
timing and direct interrupt proof.

## S4 Acceptance And S5 Admission

S4 is accepted at `ba15ae5`: Core converts a complete indexed PPU image to a
copied 80-by-25 text frame, while App reads the fixed startup representation
from `mynes.ini`; no monitor `set video` path remains. GCC x64 build, all 96
registered CTest cases, documentation governance and whitespace checks passed.
S5 is admitted under the standing M3 execution approval for PPU vblank/NMI and
OAM DMA only. Sprite rendering and scrolling remain later PPU-completion work.

## S5 Implementation Record And Scope Correction

S5 uses the existing machine slot clock to sample the PPU NMI line after every
three-dot CPU transfer.  The machine combines that level with an independently
asserted external NMI line, so clearing vblank cannot erase an external request.
At vblank, an enabled PPU NMI reaches the normal CPU interrupt entry and its
vector.  `$4014` latches a DMA page; the next machine service performs the
documented production reads and `$2004` writes, preserving OAMADDR wrapping and
the 513/514-slot parity distinction.

`mynes.core.interrupt-smoke` and `mynes.core.dma-smoke` provide the direct NMI,
vector, OAM copy, OAMADDR-wrap and both-parity checks.  The S5 implementation
boundary is `e5af1aa`.

Commits beginning at `cc4b8c7` added raster, Console adaptation and input work
while the S5 packet was still active.  Those changes are not reclassified as
S5 scope.  They are received by S6 for direct proof, correction or transfer;
this preserves the original S5 stop boundary instead of treating scope drift as
an acceptance claim.

## S5 Acceptance And S6 Admission

S5 is accepted for its original NMI/DMA outcome.  The current registered CTest
suite (97 tests) passed on both shared x64 and x86 builds, and the documentation
governance gate passed before S6 admission.  S6 is automatically admitted under
the owner's M3 execution authority to own the unaccepted raster/input increments
and their bounded evidence.  It does not claim native-presenter or final-M3
qualification.

## S6 Checkpoint: Raster, Input And Conversion Foundation

S6 made the inherited Core increments explicit and added direct receivers. The
PPU records color plus emphasis for each completed pixel, sets vblank at scanline
241 dot 1, clears status at pre-render dot 1 and shortens rendered odd frames.
It evaluates sprite overflow per scanline and covers sprite priority, scroll,
mirroring and PPUDATA palette-buffer behavior. Driver input retains two source
button sets, cleans retiring sources independently and neutralizes opposites.

Window publication derives RGB from completed samples, retaining an exact
palette through 256 colors with the documented 216-color fallback. Console uses
the same RGB source, centers a 66-by-25 content area, maps luminance to ASCII
and selects nearest RGBI foreground. Production-frame tests cover base color,
Console bars/glyph/foreground and character maps; App tests cover F5/F12
allowed and pending/error-rejected paths.

The project-owned `mynes-visual-rom-fixture.nes` is generated during
`mynes.integration.visual-rom-smoke`; it is not an imported ROM. Its 6502 program
writes a solid CHR-RAM tile, palette entries and the background mask through the
production CPU/PPU MMIO route. Its zeroed nametable makes the complete background
use palette `$3F01`; its controller loop reads pad A through `$4016` and switches
that palette entry between `$21` and `$2A`. The test asserts the corresponding
Window RGB pixels and a changed Core-produced Console text cell after a real K
press on x64 and x86. This closes the in-memory, scripted-fixture portion of G13;
it does not substitute for the native observations owned by Presentation.

This checkpoint is not S6 or M3 acceptance. Native Window/Console observation,
complete input generation/drain coverage, final artifact evidence and remaining
PPU pipeline races stay with Input and
Presentation receivers.

## S6 Acceptance And T11 Closure

S6 is accepted at `96a9f71`. Its final project-owned fixture extends the
controller serial proof into the requested graphics path without adding a
commercial asset: 6502 code drives CHR-RAM, PPU registers and `$4016`, and the
published Window and Console observations change on the same A input. The
registered 99-test CTest suites passed on shared x64 and x86; the documentation
governance and whitespace checks passed; the local delivery is clean. No remote
is configured, so this is a local commit under the approved delivery rule.

T11 closes its bounded vertical/raster foundation. Exact PPU register, fetch,
scroll, sprite and DMA behavior transfers to T12; complete source-generation,
drain and App state behavior transfers to its named Input receiver; native
Window/Console activation, observation, artifact proof and final M3 closure
transfer to Presentation. These transfers preserve the remaining G02-G14 and
R04/R06/R07/R11/R12 requirements; T11 makes no final-M3 claim.
