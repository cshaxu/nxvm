# T555 S7 Residual Video Reconciliation

## Scope And Current Proof

Baseline is accepted S6 `73d796724`, with S7 admission `f8ff92073`.
This record consumes the seven complete residual classes from the S5 ledger:
V08, V09, V17, V18, V19, V20 and V28. It is not a replacement universe or
a claim that all video behavior is already qualified. The initial reconciliation
changed only documentation; the subsequent NXVM candidate now repairs the RAM
gate and live planar text/font producer as recorded below. Shared trees,
published EXEs, owner INIs and external masters remain unchanged. S7 remains
active; no implementation P exists.

Architecture and coding governance were read and applied in that order.
Chip register/VRAM/raster ownership, board clock/decode/IRQ ownership and
Machine copied publication remain distinct. No presenter-native patch can
replace a missing chip producer. Changes stay in NXVM unless an independently
reviewed Shared contract proves necessary.

## Originals Rechecked

- IBM Options and Adapters Technical Reference Volume 2, archived as
  `platform/ibm-pc-options-adapters-technical-reference-vol2-apr1984.pdf`:
  SHA-256 `B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F`.
  The relevant EGA chapter pages carry August 2, 1984 dates. Printed 13,
  15, 17, 19, 22 and 61 were visually rechecked. RAM-enable/clock selection,
  status sources, diagnostic MUX, reset polarity, font maps and panning are
  separate facts. OCR locates material; it is not the decisive oracle.
- Compaq Enhanced Color Graphics Board/Color Monitor Technical Reference
  Guide, December 1986, Book 106451-001: original scan remains in ignored
  active research storage, SHA-256
  `278C839C18962ABD57B661315A80C519E343005C9FA1708FD1BC7461A92ADB34`.
  Printed 2-7, 2-19, 4-3, 4-10, 4-12 and 4-13 were visually rechecked, as was
  IBM printed 23's Alpha/extended-memory requirement. Do not replace the
  original clock-selector diagram with a guessed common EGA rule.

Page rendering initially landed on adjacent pages. The footer inspection
corrected the mapping before using a fact: IBM RAM/clock is printed 13,
not 14; Compaq oscillator selection is printed 2-7, not 2-8. No incorrect
page label is used as proof. Images and manual bytes stay outside Git.

## Complete Residual Inventory

| Class | Current source evidence | Required repair/proof and retained distinction |
| --- | --- | --- |
| V08 RAM/status inputs | Generic Miscellaneous bit 1 now gates CPU RAM with its sourced enable polarity; CECG keeps its opposite CPU-disable polarity. Generic Input Status 0 bit 7 now reports the live display/retrace state, separate from the vertical-IRQ latch. The Default-PC profile selects generic EGA but does not yet declare the physical switch/feature-connector wires for bits 4--6. | CPU decode remains separate from scanout/reset. Direct chip and Board-port tests prove RAM and live status independently of the IRQ latch. Generic switch/feature wires remain an explicit profile-input gap: do not invent their electrical defaults in Video; a later profile declaration must own them. |
| V09 clock/reset consumers | Personality-specific register masks and generic asynchronous Character Map clear already exist and are tested. `x86_video_advance()` freezes enhanced raster during reset; the Board uses a fixed `vadp_clock`. Misc clock-select is stored or used for CECG switch sense, not consumed by that clock domain; horizontal scaling only consumes sequencer bit 3. | Keep the accepted mask/reset repairs. Reconcile 8/9 dots, divide-by-two, shift-load, bandwidth and actual declared oscillator inputs; route selected clock facts through the one Board clock owner. IBM and CECG reset-bit labels differ. Do not invent asynchronous RAM-loss contents or claim an exact physical axis for an unqualified profile. |
| V17 live fonts | `x86_video_capture_text_snapshot_from()` copies the construction 8x16 font into 32-byte output slots. It neither reads plane 2 nor sets `text_font_selection` or the secondary bank; the latter fields are only consumed in `core/machine/frame.c`. | Use the existing VRAM owner for actual text characters/attributes/fonts, not CPU read-map inspection as scanout. IBM maps are spaced 8 KiB, CECG 16 KiB with font-address bit 13 zero; both glyph strides are 32. Respect extended/alpha memory conditions and active CRTC height. Same bank selections disable dual-map interpretation. Preserve color intensity rules. Default project BIOS must establish actual guest-visible font state rather than retain a host-only fallback. |
| V18 raster/MUX/IRQ | Programmed enhanced raster, interrupt latch/deadline and Board IRQ refresh already exist. `x86_video_ega_graphics_diagnostic()` samples only graphics in eight-dot mode; text cannot drive diagnostic bits. CECG diagnostic status is later filtered by output-active checks. | Share a chip-owned scanout/address/color mechanism between visible capture and status sampling; prove status does not change with read count. Reconcile blank/retrace endpoints, wrapping and status polarity against originals. Preserve the existing IRQ/PIC/deadline owner and source grades. No presenter clock or second scheduler. |
| V19 advanced geometry | Graphics already consumes start, offset, preset row, panning and line compare through `x86_video_ega_graphics_offset()`. Enhanced text still returns construction columns/rows and reads contiguous bytes using a fixed text window; it does not consume those text geometry controls. | Retain proved graphics behavior and test its remaining boundary cases. Produce text geometry from CRTC and real display addresses, including row stride and start/cursor coordinates. Reconcile fine-scroll/split and nine-dot representation against the copied output contract before selecting a rendering design. Do not silently ignore unsupported programmed geometry or create a parallel renderer. |
| V20 CECG Restore/blanking | Output-active handles selected internal/external blank bits; Control bit 7 affects status. Control bit 6 is stored but has no Restore consumer. Existing monitor tests cover eight blank-selection cases, not complete priority/Restore. | Reconcile Restore, diagnostic override and monitor compatibility with original timing/logic diagrams; one output-admission rule must drive capture and status without gating CPU storage merely because output is blank. Preserve cold reset and real color/mono routes. |
| V28 retained VGA | No shipped App selects VGA. Current common enhanced CRTC masking discards cursor-disable bit 5 and other VGA-only bits before later VGA capture can consume them. Read-count diagnostic toggling is a distinct retained VGA path. | Isolate genuine VGA register masks/semantics and test them without applying VGA rules to selected EGA/CECG. Keep the four selected cards unchanged. Retained VGA regression proof is not a fifth shipped product or physical VGA timing qualification. |

These are current-code facts, not seven newly reproduced guest failures.
In particular the existing graphics controls must not be reported as missing
simply because the earlier ledger recorded their pre-S5 state.

## Design And File Surface Before Implementation

The simplest design preserves one chip output/capture owner and one VRAM
store. CPU decode, display enable and sequencer run state are different
inputs, not synonyms. Existing private address/palette/raster helpers should
be extended where they own actual computation; a manager, mirrored flags,
second text cache, compatibility font bank or second event loop is forbidden.

Expected NXVM production surface is `core/chips/video/video.c`, its existing
private/public value headers only when needed, Board `vadp.c`,
`board_advance.c`/`board_deadline.c` and current copied display inputs; Machine
frame/capture only for a demonstrated receiving need. Selected oscillator
declarations belong to App construction, never a machine-name switch in the
chip. Project font initialization belongs to `app-nxvm/firmware`, not a BIOS
address special case in Video. Existing corresponding chip, board, Machine,
App and guest-firmware receivers own the tests.

Initial code-size estimates, excluding documentation/manifests/artifacts:

- CPU/RAM and output/reset gate separation: production +25..60/-10..30,
  existing tests +80..150/-15..40.
- Live planar text/font production and geometry: production +100..220/-45..90,
  existing tests +180..350/-30..80, plus the separately estimated necessary
  project-BIOS initialization after its exact existing font-input review.
- Clock/raster/status and CECG priority: production +100..220/-35..90,
  existing tests +180..350/-30..80. Exact Core-only copied clock input changes
  must be listed before editing; this estimate is not permission to invent
  oscillator or feature-connector inputs.
- VGA mask separation: production +10..25/-5..15, existing tests +40..80/-5..15.

These are design estimates, not a claim that unresolved source or presentation
constraints already have an approved implementation. The generic KVM text
renderer has an eight-pixel cell and no fine-pan/preset/split descriptor.
The full text representation therefore needs a concrete contract check before
claiming nine-dot or fractional text scrolling. No Shared extension or fake
graphics-mode transition is authorized by this record. Ordinary plane-2
fonts and RAM gates can use existing contracts independently.

## Initial Regression And Next Proof

### Live Plane-2 Production Admission

IBM printed 22/23 and CECG 2-19/4-12/4-13 are visually reconciled: both
Alpha/character-select and extended-memory bits enable map selection; IBM
font banks are 8 KiB, CECG 16 KiB, while each glyph is 32 bytes. Extend the
existing Video contract with real processor writes into plane 2 and planes
0/1, no borrowed memory reader, every map pair and heights 8/14/16/24/32.
Processor read-map/compare settings must not choose display data or mutate
latches during capture. The test fails on the old borrowed construction-font
path with `VIDEO:LIVE-FONT:FAIL`; its log is retained under ignored build.
Admit one private map/copy helper plus the existing capture function's actual
planar character/attribute reads, with distinct existing non-planar backing
retained for CGA. Estimate this production segment +65..95/-15..25 lines;
the existing contract receiver adds 79 lines. No new public boundary, state,
renderer or font fallback is authorized. Whole S7 caller/firmware/geometry
and final product proof remain required before delivery. A necessary private
text-storage ownership predicate is shared by capture and generation
observation, rather than leaving the former borrowed-storage classification
after the producer has changed.

### RAM Gate Admission

Implement V08's source-proven generic RAM-enable interpretation separately
from the existing CECG inverse-polarity control. Reuse one private predicate
for processor access and the existing copied bus observation; display-enable
does not become a CPU RAM gate. Retained nonselected VGA behavior is not
silently changed by this generic-EGA repair. Source surface is Video C and
the existing bus-observation comment, with the current planar smoke receiver.
Estimate production +10/-5 lines and tests approximately +70 lines. No public
function/type or persistent state is added. First run the expanded receiver
against unchanged production and require failure, then repair the chip and
propagate explicit RAM setup through actual callers/tests and project BIOS
without changing reset semantics to hide a missing guest write. Whole S7
and real Board receiving proof remain required before an implementation P.

Existing frozen-code baseline passes x64 63/63 in 30.04 seconds and x86
46/46 in 8.21 seconds in the original Ninja/ccache trees. The first regex
also matches `negative` and `legacy` names; its extra rows are incidental,
not another hardware claim or comparable performance measurement. The x86
selection corrects that matcher. Both include chip, board, Machine/App and
six default external video receivers. Logs remain under ignored `build/`
as `t555-s7-video-baseline-{x64,x86}.log`. No complete S7, complete repository,
manual desktop, new artifact or electrical qualification follows.

Next, finish paired original-page and output-contract reconciliation; add
failing same-owner assertions for the confirmed gate/font class before
production repair, preserving all existing successful routes. Final complete
dual-width units, four-PC integration (DeskPro startup 300 seconds), current
affected eight PC artifacts and actual-pushed-diff review remain mandatory.
S8's speed/AUX-PIT/HOLD remains intact. The whole T555 goal is not complete.

## Candidate Implementation And Bounded Receiving Proof

The RAM regression fails on original production, then passes after the one
private family-aware gate feeds CPU access and the existing copied bus
observation. Generic EGA bit 1 enables RAM, CECG's bit 1 disables it; CGA and
retained VGA behavior are not silently changed. The real Board receiver
proves a disabled device declines decode without overwriting its VRAM; the
neutral Core's ordinary RAM remains its distinct existing owner. Re-enabling
the device exposes the preserved device bytes. Attribute blanking still
permits CPU access. Reset remains disabled for generic RAM until an explicit
guest port write; the two default project-firmware writes now use 03h.
The corresponding RAM-stage dual-width 21/21 selection includes all 19 Video
chip cases, Board planar routing and guest BIOS cursor execution. It predates
the live-font producer and is not final current-firmware qualification.

The live-font regression then fails with `VIDEO:LIVE-FONT:FAIL` against the
construction-font/borrowed-text producer. The repaired existing capture path
reads characters/attributes from its own planes 0/1 using the existing display
address computation, and glyphs from plane 2 with the source-specific bank
spacing and fixed 32-byte slots. All 16 map pairs, three Alpha/extended modes,
five active heights and both personalities are checked, including glyph 255
row 31, untouched processor latches and trustworthy generation acknowledgement.
CGA/non-planar backing and the explicitly retained logical CGA compatibility
view remain distinct; no font fallback or extra VRAM/cache owner is added.

The complete x64 Video sweep initially passes 17/19. The two failures are
`compaq_cecg_contract` and `ega_planar`: their old text bytes entered a
borrowed fixture instead of device VRAM. Their inputs are migrated through
the real processor write contract, with monitor blanking, palette, cursor,
repeated-capture and memory-operation assertions preserved. CECG's independent
CPU probe moves outside the first visible text row rather than overwriting
the very character being asserted. The Board receiver receives the same
proper odd/even plane setup. Final current chip/Board selection passes 20/20
per width (0.47 seconds x64, 1.24 seconds x86); logs are
`build/t555-s7-live-font-video-owner-{x64,x86}.log`. These are no complete
repository, four-product, native desktop or published-artifact claims.

Measured current C/H against admission: production three files +80/-23,
net +57; tests five files +204/-10, net +194. Project firmware is +2/-2,
net zero. Documentation/manifests/artifacts are excluded. There are three
new private production helpers: family-specific CPU RAM interpretation,
shared planar-text ownership and glyph map/copy. Each owns real computation
used by the existing paths, not a forwarding API. No public function/type,
persistent state, renderer, queue, thread or frame cache is added.

Remaining before delivery: all actual Machine/Profile consumers and source
oracles, real default-BIOS font loading and valid text setup, live geometry
and presentation limits, clock/reset/raster/MUX/Restore/VGA residual classes,
full dual-width units/integrations and affected eight PC artifacts. The
published ten EXEs still represent accepted S6; they are not this candidate.

## Default Guest Font Receiving Gap

The existing default-firmware cursor receiver previously stopped at valid
cursor registers, without checking whether any glyph existed. Its existing
text-ready checkpoint now also requires the captured `A` glyph to contain a
nonzero active raster. This is seven added lines and one removed line in the
existing App test, with no new target or production API. The rebuilt x64/x86
receivers fail in 1.20/0.08 seconds; this is a failed qualification, not a passing
cursor result. The terminal builds and CTest results establish the missing
consumer independently of the already passing chip/Board font tests.

Current code establishes the cause: the offline BIOS builder has no font
input; entry calls INT 10h mode 03h, but that service never loads plane 2.
The firmware test embeds no construction font. The product's existing BYOB
MDA character generator reaches the non-planar construction-font path, not
guest planar VRAM. Loading construction glyphs inside capture would restore
the wrong ownership and is rejected. Text mode also selects Graphics Misc
08h (B0000), despite its own services writing B8000, and sets CRTC Offset
to zero. These are actual project-firmware inputs to repair, not exceptions
to add to the generic EGA chip.

The next bounded implementation uses the existing offline builder, guest
mode-setting service and embedding recipe. Normalize the existing approved
BYOB character-generator input into a reserved BIOS font extent, validate
that code cannot overlap it, and let guest instructions load fixed 32-byte
slots through the ordinary plane-2 processor aperture. Then establish the
text odd/even planes, B8000 aperture and word stride. Production builds use
the already validated external font; unit builds use an explicitly code-owned
pattern and must verify that exact pattern in the captured glyph, not merely
nonzero output. No runtime file access, host font fallback, shared changes,
extra renderer or persistent font-loading flag is admitted.

Estimate for this whole receiving mechanism: builder C +30..50/-3..8,
guest assembly +90..140/-10..20, existing CMake and test recipe +20..40/-5..10,
and existing receiver assertions +20..40/-1..5. Actual extents, register
ordering, failure-preserving output and cold/mode-return receivers must be
reviewed before implementation. These estimates do not allocate a partial P,
close S7 or replace the remaining seven-class and four-profile qualification.

### Guest Font Candidate And Exact Receiver

The existing builder now requires the validated character-generator input,
normalizes its two eight-row halves into 32-byte slots at A000h, and reserves
that extent by reducing the preceding video assembly limit. It uses existing
Lib owned-file read and writer services; missing, short and oversized input
fails before output is opened. The existing unit recipe confirms deterministic
construction, both font halves and zero padding, all original assembly-failure
cases, and preservation of old output after all three invalid font cases.
Production and unit outputs deliberately differ only in their font input:
production consumes the existing approved BYOB file; units generate code-owned
55h/33h rows. One builder, one guest assembly and the same embedding script
serve both. Configure writes use `file(CONFIGURE)` to preserve timestamps when
the fixture content is unchanged. No external byte is a unit dependency.

Mode 03h resets inherited processor transforms, selects read/write mode 0,
loads plane 2 through CPU `REP MOVSW`, then establishes planes 0/1 odd/even
access, B8000 decode, text stride 28h and word addressing. The live receiver
checks every row of the captured `A` slot, including inactive 33h rows and
zero padding, then runs all three original graphics modes and returns to text.
Before return it deliberately damages the live glyph. Characters A/B on row
zero and C on row one plus distinct attributes prove the aperture and stride.
The damage test initially exposes retained Graphics Mode during font load;
an explicit mode-zero write repairs it. No stale-font success is accepted.

The prior 1000-instruction mode-call containment expires during the actual
4096-word font copy. It is now 10000, with the same required BIOS return PC,
register preservation and final frame assertions; no guest timing or product
timeout is relaxed. Both failed attempts remain visible in execution evidence.
Final bounded firmware selection passes 3/3 per width: x64 10.85 seconds,
x86 9.16 seconds. These terminal runs are not complete repository or desktop
qualification. Core source/test manifests and whitespace still pass.

This receiving segment adds no Core or Shared interface/state. Against the
S7 admission, its files currently count builder +30/-3, video assembly
+98/-2 (including the earlier RAM-enable spelling), CMake +19/-6, existing
App receiver +43/-4 and builder unit +23/-2. Documentation and generated
font/ROM/object paths are excluded. The one new private builder helper owns
input validation and actual format conversion; it is not a forwarding API.
Remaining geometry/clock/raster/MUX/Restore/VGA and actual published-product
verification are unchanged requirements before S7 delivery.

The final combined changed-owner selection passes 23/23 per width, x64
6.16 seconds and x86 8.92 seconds: all 19 Video cases, actual Board planar
receiver and the three firmware tests. Both receiving processes are terminal.
Selected BYOB production firmware assembles and embeds on both widths; the
candidate ROMs have the identical SHA-256
`F17D1201B75377DBAFA1FAD7DB0949C5298B2D1C2DCDC31F4CF9783314644F17`.
These ignored candidates are not deployed EXEs or proof of all four real
firmware boots. Documentation and whitespace gates pass; shared six and assets
have no diff. Manual desktop and complete integration remain unexecuted here.

## Whole-Cell Enhanced Text Geometry

Original pages visually rechecked for this segment: IBM EGA printed 31/32
(Overflow bit 1 and row-scan counter), and CECG printed 4-24, 4-28 and 4-34
(displayed character count minus one, character height minus one, displayed
scanline count minus one). Existing scanout computes the same nine-bit
vertical endpoint for graphics; text now reuses that helper instead of a
construction-time 25-row value. This does not change the distinct MC6845
logical-text geometry or retained nonselected VGA route.

Before editing production, the new existing-contract receiver fails with
`VIDEO:PLANAR-TEXT-GEOMETRY:FAIL`. It then passes after existing column/row
helpers consume the actual enhanced registers, capture rejects dimensions
beyond the copied capacity, and planar cursor matching uses the programmed
row stride rather than dividing a contiguous address by visible width.
The eight normal vectors cover both personalities at 40x25/8, 80x50/8,
80x25/14 and 80x16/32, enlarged logical strides, high start addresses and
16-bit wrapping. Bytes enter actual CPU aperture planes 0/1; copied cells,
attributes and the final-row cursor are independent expected values.
Four invalid geometries per personality cover 81 columns, 51 rows, a partial
final character and no complete row. Failure leaves both the prior snapshot
and capture-cache data unchanged after the register write.

The existing font/cursor/palette receivers now explicitly establish valid
CRTC registers rather than relying on construction geometry. One same-owner
test helper owns that register setup; it is neither a validation oracle nor
a production interface. The font matrix uses one complete row at each of
its five heights, preserving all 480 bank/memory-mode captures. Existing
Board port smoke programs 40x50/8 through the real registered ports, writes
the first and last rows through Core memory routing, and checks copied cells,
attributes and row-49 cursor. No chip-private helper is its input path.

The first 21-case sweep exposes one firmware receiver issue: capture during
mode programming can return the existing `UNSUPPORTED` before the geometry
is ready. The bounded guest test now continues only for that no-ready-frame
result; run faults and other capture errors still fail, and the final exact
glyph/cursor/return-PC assertions remain mandatory. Production Machine
already treats that result as no publication, preserving its frame and
generation; it is not patched to accept truncated geometry.

Final combined regression passes 23/23 per width, x64 7.41 seconds and x86
8.49 seconds, in the original Ninja/ccache trees. Both processes are terminal.
Updated Core source/test manifests verify and whitespace is clean. Public
six, MyNES, INIs/media/snapshots and published artifacts have no change.

Production segment is +27/-3 in the existing Video file, with one forward
declaration of an existing vertical-endpoint helper and no new function,
public ABI, persistent state or renderer. Additional tests versus the prior
font checkpoint: existing contract +108/-1, same-owner fixture +20, Board
receiver +23/-1, the two existing planar/CECG fixtures +1 each, and App
guest receiver +6/-3. Documentation/manifests/artifacts are excluded.

This is whole-cell qualification, not complete V19: nine-dot cells,
fractional/preset text scrolling, partial final rows and non-cell-aligned
split windows remain unrepresentable in the current eight-bit whole-cell
Shared output. They are not silently rounded into a successful frame here.
Their source/contract review still needs a separately approved Shared
decision, alongside remaining V08/V09/V18/V20/V28 and real Machine/four-App
receivers, before S7 can satisfy its original exit.

## Retained VGA Disable Bit And CECG Circuit Reconciliation

The V7VGA Technical Reference Manual, June 1988, archived original SHA-256
`1E702B0B450A90CEB43220C5C5AD7DA48D124DA76D70C4EAAB35AC1D9E38FFC8`,
was read locally and printed 3-41/3-42 visually inspected. This is original
VGA-compatible register documentation, not authority to override IBM EGA or
CECG cursor encoding. Its Cursor Start bit 5 disables the cursor, whereas
selected EGA's current five-bit mask must remain unchanged. The existing
capture already consumes this VGA bit, but the common write mask discarded
it before that consumer could see it.

The extended existing cursor matrix fails before repair. Moving Cursor Start
out of the common five-bit case and choosing 3Fh only for configured VGA
repairs that exact missing input. Production +2/-1; matrix +10/-4. All 1024
start/end pairs now run with both values of bit 5 for each EGA/VGA personality,
4096 captures. EGA must ignore that bit and preserve its own start encoding;
VGA must preserve it in the stored register, exclude it from scanline values
and hide the cursor when set. No interface, state, clock, source file, renderer
or shipped card selection changes. Other unimplemented VGA upper-field,
readback, diagnostic and physical-render behavior is not inferred complete
from this narrow proof.

Final complete changed-owner selection passes 23/23 per width (7.57 seconds
x64, 8.65 seconds x86). Both processes are terminal; refreshed Core source/test
manifests and whitespace pass. These are not complete repository/four-App
qualification or released EXEs.

For V20, original CECG Figure 7-1 sheets 5, 4 and 13 (printed 7-5, 7-4,
7-13) were rendered and visually traced. U27's control latch bit 6 drives
RESTORE, inverted to RESTORE*, which goes to U47's PLS173 input on the indexed
CRTC compatibility path. That path includes the indexed address latch and
U52 internal SRAM. It is not an immediate VRAM clear/reset or host-frame
restore signal. The earlier store-only observation remains true, but the
candidate must not repair it by inventing one of those unrelated effects.
The PLS173's actual decode/translation function still needs original-source
reconciliation; seeing the net alone does not prove its truth table.

The same read distinguishes monitor-driver enable on sheet 13 from ordinary
raster blanking and the documented diagnostic override. Bit 7's status input
is currently filtered after the initial diagnostic result. The full priority
and compatibility logic still need proof before changing capture and status;
no guessed override, recovery reset, mirror SRAM or new rendering path is
introduced. The external 86Box/PCjs searches are investigative leads only:
no source is copied and their general EGA behavior is not a substitute for
this original CECG circuit contract. V08/V09/V18/V19/V20 and the whole S7
receiving/publication exit therefore remain open.

## Real Driver And Bound Worker Receivers

Shared raster approval is still pending. This segment modifies no production,
Shared, MyNES or artifact input; it reconciles actual NXVM receiving tests
with the admitted RAM/font/geometry producer rather than bypassing that
producer or treating chip-only green results as publication proof.

`nxvm_display_composition_smoke` fails before receiving reconciliation.
It assumed a construction-time 80x25 frame without programming CRTC and
wrote Miscellaneous Output 01h before planar writes. Its explicit same-owner
port setup now establishes 80x25 whole-cell text, and its graphics setup
enables RAM with 03h. Reliable generation for device-owned planar text is
asserted rather than classified as borrowed backing. The separate logical
CGA borrowed route, palette changes, cadence, immutable published content,
failed-copy preservation, pitch changes and reset/first-publication assertions
remain. Reset checks the current reset register projection (1x1), then a
fresh port program must produce 80x25 again. That is not a claim that physical
cold CRTC registers have a documented 1x1 display or that reset runs BIOS.

`nxvm_runner_display_cadence_smoke` now proves the complete bound route:
paused Debug requests travel through the existing Emulator lease/executor to
Core memory/ports, populate both font banks and planes 0/1, and program 80x50
with a row-49 cursor. Before actual execution, the old published graphics
frame remains unchanged. Four real instructions followed by pause publish
one text frame with both glyph bytes, cell bank selection and the final-row
cursor. A secondary-font-only edit publishes one more frame; another unchanged
trace does not publish. Reset changes run generation, clears both live font
banks and publishes its new frame; the old generation cannot read it.
No observer calls capture on the bound driver and no test callback replaces
the frame producer. The existing paused host-cadence expiry is retained.
One private test helper owns the reused lease-aware port-table execution.
There is no new worker, sink, queue, runtime flag or public interface.

Model40's machine-contract receiver initially fails at its video stage. It
programmed character height 14 without a compatible display endpoint and
expected the old host-16-row font's scaled cursor. It now supplies actual
80x25/350-scanline CRTC registers and proves the original CECG last-two-row
meaning directly as scanlines 12..13 of the live 14-row output. All preceding
RTC/FDC/HDC and reset assertions remain unchanged.

The first wider current receiving sweep passes 21/23 per width. Its two
failures are isolated to old inputs: the Core CPU-gate receiver expects generic
RAM enabled before any write; the Model40 Feature receiver writes B8000 while
the configured aperture is A0000 and has an unprogrammed one-cell display.
The former now proves disabled generic decode before an explicit enable,
retaining CECG inverse polarity and reset. The latter explicitly establishes
the two-character row and B8000 aperture, retaining both original character
assertions and every Feature/Environment/reset result. It first still fails
when only geometry is supplied, confirming that aperture selection is a
separate necessary input, not a dimensions-only workaround.

Final 23 receiving units pass once per width: 11 Core, five default App and
seven Model40 App cases, x64 0.58 seconds and x86 0.66 seconds. The additional
compiled CGA-640 executable is an external integration case and is not
claimed as executed by this unit selector. This is separate from the prior
23 chip/Board/firmware selection, with one overlapping Board case; counts
are not added as though all rows were unique. Original static registrations
were inspected. A non-returning read-only JSON catalogue request was cancelled
through its own terminal handle; no test was restarted or host setting changed.
All launched builds/tests and that cancelled diagnostic handle are terminal.

The segment's actual five test-file diff is +174/-26, net +148, from Git
numstat excluding documentation/manifests/artifacts. The two default receivers
are +142/-22; Model40's two receivers +28/-3; Core CPU-gate +4/-1.
The added evidence covers real inputs and transport, not production wrappers.
Updated Core manifests and whitespace pass. Manual desktop, full repository
units, four-profile external integration and final artifact delivery remain
unexecuted for this candidate, alongside the unchanged Shared/source decisions.

## V09 Character-Clock Failure And Unit Reconciliation

The preceding terminal x64 contract test failed with
`VIDEO:CHARACTER-CLOCK-DEADLINE:FAIL`. Its first oracle assumed one reference
tick represented eight master dots, which the public logical-clock contract
does not establish. That absolute convention is not accepted as source proof.
The existing test now compares only the sourced 8:9 character-clock ratio
and each mode's divide-by-two relation. It selects the actual color CRTC
decode and synchronously halts the personality-specific Sequencer before
changing Clocking Mode, then restarts it. Same-owner phase setup establishes
the same initial phase, without adding runtime state or a public test API.

Original CECG printed 4-10 was visually rechecked: bit 0 selects nine/eight
dots and bit 3 divides the master clock by two. The original reset diagram
also confirms the differing IBM/CECG halt-bit positions; the test does not
apply the IBM halt value to CECG. The earlier generic fixture lacked explicit
color decode, so its unsuccessful register setup was not a generic timing
proof. The final fixture supplies that necessary input before programming.

Both original Ninja/ccache trees compile the revised test with strict warnings.
The focused contract execution fails on both widths: generic and CECG each
report 24 ticks for both eight- and nine-dot modes, and 48 ticks for both
divided modes. Thus the width selector is confirmed unconsumed; the existing
divide-by-two relation is retained. Each case separately checks that the
currently reported deadline and actual IRQ edge agree, rather than mistaking
agreement between two wrong calculations for source correctness. A transient
diagnostic cast used a nonexistent Types spelling; it was corrected to the
existing bounded `lib_u32` vocabulary before these final builds.

The similar-path review finds one horizontal scale feeding raster geometry,
active-display status and graphics diagnostic sampling. It currently consumes
only bit 3. Board advance and deadline conversion share the one frozen
`vadp_clock`; Miscellaneous Output oscillator selection does not update that
domain. These are two distinct facts: the chip character divider and the
board's selected source frequency. Neither a presenter clock nor an IRQ-only
multiplier is a valid repair. A coherent design must preserve one phase owner,
fractional progress and ceiling deadline conversion, including segmented
advances, status sampling and large elapsed values, with no host-time input.
Any normalization needed to retain the nominal eight-dot logical baseline
must be recorded as a model convention, not an original physical frequency.

This segment changes only the existing Core contract test, its manifest and
NXVM evidence/status. No new production implementation, public interface,
Shared modification or artifact is introduced. The failing regression remains
explicit pending the complete V09 mechanism repair; no full-suite success,
S7 implementation P, closure or physical timing grade is claimed.

### Coherent Character-Divider Candidate And Receiving Proof

The subsequent Core-only repair keeps the nominal eight-dot reference model
explicit in the existing interface comment. The single private phase member
is widened to `lib_u64` and contains master dots for enhanced non-VGA raster;
CGA and retained VGA keep their prior reference units. One private character
width helper combines 8/9 dots with the existing divide-by-two input. Raster
geometry, active status and diagnostic position use that same scale; IRQ
deadline converts back by ceiling division. Advance reduces elapsed ticks
before multiplying, retains the sub-reference phase through wraps, and uses
the same deadline for the interrupt latch. There is no second accumulator,
phase cache, public API, host clock or physical-frequency grade upgrade.
Construction, reset, text-timing initialization and VGA capability selection
preserve the declared unit distinction. Invalid/unprogrammed enhanced raster
does not advance or expose a synthetic legacy-timing display interval.

The existing contract adds the 243-dot/729-dot nine-character case: IRQ is
not early, its 31-tick deadline rounds upward, wrap leaves seven dots, reads
do not advance phase, synchronous halt retains it, segmented 92+17 ticks
matches 109 ticks, and maximum-u64 advance remains bounded and exact modulo
the logical period. Old absolute deadline vectors explicitly select eight
dots rather than accidentally depending on ignored Clocking Mode bit 0.
The CECG monitor test programs valid horizontal/vertical totals and blanking
instead of relying on synthetic timing from unprogrammed CRTC registers.

Actual Board authority tests preserve their existing 1:2 video clock and
slave-PIC assertions. New guest port writes select nine-dot normal/divided
modes; the existing inverse clock predicts source deadlines 74 and 164.
Advancing one source tick less leaves the IRQ lease deasserted, then the next
tick asserts it. Reset restoration and the old eight-dot boundaries remain.
The separate CECG status/IRQ matrix explicitly selects its original eight-dot
baseline; no original status, diagnostic or interrupt assertion is removed.

Final bounded executions pass 20 chip/Board cases per width (1.21/1.51 s),
seven real Core/default/Model40 receivers per width (7.24/0.80 s), and two
additional Board clock/PIC/status receivers per width (3.03/0.32 s).
All launched processes are terminal. Initial failing contract and CECG
fixture attempts are retained, not represented as passing results.
These 29 cases are not full repository or four-profile external qualification.

Cumulative S7 Git numstat for the seven touched character-clock-related
C/H paths is production +137/-35 and tests +355/-12; it includes preceding
font/geometry/cursor work in those same files, not just this clock segment.
All additions retain the existing owner and Types vocabulary. Corresponding
Core manifests are refreshed. Shared trees, all published EXEs, MyNES and
owner INIs/media/snapshots are unchanged. Board oscillator selection,
complete remaining video classes, full final qualification and artifacts
are still required; no implementation P or S7 closure exists.

## Approved Existing Graphics Transport, Not A Shared Extension

The owner rejects the optional nested Lib raster and instructs continuing
with NXVM-generated exact pixels through the existing graphics frame. The
updated presentation review supersedes the preceding pending-Shared proposal.
Only Core snapshot/machine receiving paths and NXVM tests/evidence may change.
No Shared frame field, API, policy, test or manifest is modified.

The Core kind enum appends TEXT_PIXELS without changing old numeric values.
Its existing pixel array grows from 640x350 to directly named 720x512 capacity;
mode sizes, character/font slots and guest register state are unchanged.
Machine validates this output kind through the existing pixel validation and
graphics copy branch. No duplicate copy/format path or nested raster is added.
The original adapter rejects the new kind in the negative-first test; after
the one case admission it produces a valid existing KVM graphics frame.

The receiving test proves 720x512 including the last pixel, stride, palette,
independence from absent logical-text fields, and preserved destination after
an extent exceeds actual Core storage. Final Machine/Mode10 selections pass
2/2 per width (0.21/0.19 s). These are representation and capacity proofs;
the actual nine-dot/fine-scroll/split pixel producer remains to implement.
No actual Window, full suite or advanced-text hardware claim follows yet.

Standalone compiler stack-usage diagnostics use the configured two compilers
and the same optimized strict-warning settings, with outputs retained only
under ignored active research storage. Display publication uses 394192 bytes
x64 / 394164 bytes x86; the initial frame-test main uses 394160 / 394120 bytes.
These are compiler-reported per-function bounds, not a whole-call-chain safety
proof or final executable stack qualification. The first compiler lookup used
only FILEPATH cache spelling and failed before compilation; the corrected
lookup handles the actual GCC cache and resolves its configured tool directory.
No build preset, cache location, production allocation or executable changes.

Cumulative four-file representation-path numstat is source +10/-7, tests
+20/-1; earlier non-planar font commentary in that values header contributes
to this cumulative number. Core manifests are refreshed. Actual producer,
diagnostic reuse, complete receiving/stack/qualification and affected PC
artifacts still precede whole-S7 P delivery; current EXEs remain accepted S6.

The adjacent gate audit confirms CECG Graphics Mode bit 2 is output tristate,
not a reserved bit. It is not removed for simplification. Variant-specific
VGA interpretation and Sequencer-reset CPU ownership remain under source
reconciliation; no unsupported correction is inferred from their names.

## Register-Driven Text Pixel Producer And Real Publication

Negative-first plane-0/1/2 contract testing reproduces the missing producer
with `VIDEO:TEXT-PIXELS:FAIL`. The subsequent Core implementation shares
scanout row/address calculation with graphics, and one text pixel sampler
supplies both capture and diagnostic MUX. It consumes the character width,
actual font banks/glyph rows, start/stride, row preset, horizontal pel pan,
partial character rows and line-compare restart. Source-specific ninth-dot
line characters C0..DF and font spacing remain explicit. No host font,
synthetic graphics register, second pixel buffer or Shared change is added.

The original IBM/CECG font evidence must not be flattened: IBM describes
font selection replacing foreground intensity; CECG printed 4-12 explicitly
requires software palette or plane-enable adjustment for consistent alternate
font colors. Core supplies a foreground-only copied mask, and the existing
Machine cell conversion applies it only to foreground. Background intensity
and palette values remain unmodified. Validation accepts only zero/default,
7 and 15 before output mutation. Pixel and logical-font paths share one font
map helper; logical and pixel cursor shapes share one bounds helper. The
pixel cursor's actual frame-phase blinking is still required, not qualified
by these static shape assertions.

The revised same-owner test verifies both personalities through processor
VRAM writes: ninth-dot repetition, neighboring character pixels, partial
final/no-complete rows, fine horizontal shift, preset scanline and an
intra-character split. Secondary-font selection has different expected color
indices for the two cards. Ordinary eight-dot output returns to text and does
not overwrite the inactive pixel tail. Diagnostic reads select all four MUX
pairs from that same glyph pixel, preserve phase/generation and remain stable
on repeated reads. Invalid width, repeated-scanline capacity and preset
preserve destination and chip acknowledgement. Earlier character-container
limits are no longer claimed as hardware-invalid partial-row cases.

The initial producer candidate has two failures: byte-addressed test text
was written at word-spaced addresses, and old geometry tests still classified
now-representable partial cells as invalid. Correct actual addressing and
replace those invalid assertions with real capacity/preset failures; retain
the positive partial-row proof. A wider chip sweep initially passes 17/19
per width; its CECG cursor and generic text fixtures inherited nine-dot
Clocking Mode while expecting logical eight-dot frames. Explicit eight-dot
inputs preserve their original assertions; corrected affected tests pass
2/2 per width. Subsequent focused contract/CECG/frame conversion passes
3/3 per width (1.03/0.40 s), not a new complete chip-suite claim.

The actual bound worker then exposes a distinct sequencing error in the
test: generic halt value 2 invokes asynchronous reset, correctly clearing
Character Map. Its pixel sequence/size/first pixels are correct but the
secondary font sample reads background 2 rather than foreground 7. Original
IBM/CECG reset diagrams require generic synchronous halt 1 and CECG halt 2.
Correct all three newly introduced character-clock helper writes and the
worker clock transitions; preserve the chip's source-backed asynchronous
map clear. The earlier claim that those test writes were synchronous was
incorrect and is superseded by this actual receiving proof, not erased.

Final worker/contract execution passes 2/2 per width (2.03/0.46 s): 720x400
existing graphics output, primary/secondary pixels, one publication on a
mode change, unchanged-frame suppression and reset/new generation. The
guest remains alphanumeric; there is no observer-side capture or alternate
publication path. The cold unprogrammed one-cell clear frame is retained as
the existing logical reset view, not claimed as physical nine-dot scanout.

Eight currently touched pixel-related C/H files have cumulative S7 Git
numstat source +316/-77 and tests +707/-33. These counts include preceding
RAM/font/geometry work in those same paths, not only this producer increment.
Manifests are refreshed and no Shared/MyNES/artifact or owner input changes.
Blink, complete V09/V18/V20/V28 source dispositions, full receiver/stack
qualification, full units/four-PC integration and current eight PC artifacts
remain required. No complete implementation P, S7 or T555 closure is claimed.

### Character Blink And Guest-Time Publication Design

CECG printed 4-40 fixes character blinking at 32 vertical periods. Generic
EGA uses that same relative period as an explicit L2 reference model, supported
by the local read-only 86Box EGA implementation; no source is imported and no
physical oscillator or IBM exact period is claimed. Cursor timing is a separate
remaining fact and is not inferred from character blink.

Reuse the existing field phase (formerly CGA-named) and dot phase. Capture
selects existing TEXT_PIXELS only when visible attributes actually require
blinking; ordinary text without such attributes retains its Console route.
The existing deadline query may return either a vertical IRQ edge or the next
pixel blink boundary. Advance computes IRQ independently, so a presentation
deadline never asserts an IRQ. Board consumes the query without requiring an
IRQ lease, retaining its qualified-time guard. No host timer or new state/API
is introduced. Expected production +80/-20, tests approximately +100 lines
across Video, Board deadline and existing owned fixtures. Final whole-S7
qualification remains mandatory after this bounded mechanism proof.

The candidate reuses the field counter and IRQ dot-deadline calculation.
Existing owned tests verify both personalities' 16-field half-cycle, one-tick
edge, complete 32-field no-change wrap, no IRQ from blink, intensity mode,
unchanged-capture suppression and ordinary text without blinking attributes.
The real Board receiver has no video IRQ lease: guest port/memory writes
produce a pixel frame, its inverse clock query schedules the update, and
peripheral advance publishes a changed pixel at the final source tick.
The first Board oracle incorrectly assumed setup left raster phase zero;
actual port accesses consumed 16 source ticks and gave 2032, not 2048. The
corrected receiver consumes the actual chip deadline through the existing
clock conversion instead of resetting private phase or weakening the edge.

Final video-chip/Board/worker selection passes 21/21 per width, x64 1.92 s,
x86 1.45 s, in the original Ninja/ccache trees. These are 19 chip cases plus
the Board authority and existing bound worker; the worker does not yet prove
the new blink edge itself. Complete blink/underline/cursor/remaining-source
receivers, full units/integration and eight artifacts remain open. No Shared,
MyNES, protected inputs, artifact or implementation P has changed.

### Cursor Horizontal Skew Design

Rechecked CECG printed 4-30 visually: Cursor End bits 5/6 delay the cursor
signal by zero through three character times. IBM printed 34 describes the
same four delays. The current write mask retains these bits, but both logical
and pixel text ignore them. Keep the existing cursor shape differences and
consume the delay at the visible-column boundary. Logical capture shifts the
matched column and hides a cursor delayed past the row; pixel capture compares
the earlier character address while retaining the current cell's color and
row-scan shape. Existing scanout addressing owns start/stride/wrap/panning.
No register mirror, API, framebuffer or Shared change is needed. Expected
production +12/-4, existing tests +45..70 lines; both personalities and zero
through three delays must be exercised in eight-dot and nine-dot output.

Implemented at those two existing capture boundaries, with no new helper or
state. Both personalities exercise delays 0..3 in logical eight-dot and
pixel nine-dot output; a last-column cursor delayed by one character is
clipped rather than wrapped into another row. Character/font contents remain
unchanged. The focused contract passes x64/x86 1/1, followed by the complete
current 19-chip plus Board/worker selection passing 21/21 per width
(4.27/1.84 seconds). This is still not whole-S7 or manual desktop proof.

Original underline definitions are also visually checked: IBM printed 39
programs one less than the desired scan line; CECG printed 4-35 programs the
scan line directly and hides it when beyond Maximum Scan Line. IBM printed
75 identifies underline for foreground RGB 001 on background RGB 000,
independently combined with blink/intensity. Its full attribute/admission
and receiving implementation remain open; no guessed common offset is added.

### Underline Implementation Design

Use one private underline scan calculation: generic EGA adds one to the
five-bit register, CECG uses it directly. The attribute RGB predicate is
`(attribute & 0x77) == 1`, allowing independent intensity/blink bits. IBM's
printed 75 proves that combination; CECG shares it as an explicit L2
attribute model, while its register offset and out-of-height suppression
are sourced directly. Capture scans actual visible attributes once for both
blink and underline, and selects existing pixel text only when a real effect
cannot be represented by character cells. Pixel sampling resolves underline
before blink suppression and cursor overlay. No font mutation or second
render path is allowed. Expected production +20/-5 and existing tests
+50..80; ordinary glyph fixtures explicitly disable underline when their
attributes would otherwise request it. Remaining S7 qualification is unchanged.

Underline now uses the actual register/attribute producer and is tested for
both offsets, intensity/blink combinations, non-underline colors, out-of-height
suppression and blink suppression. The real bound worker writes the underline
register and plane-1 attribute through its paused Debug lease, then publishes
640x400 graphics with the expected full eight-pixel underline after executing;
there is no direct observer-side publication. Final current 21-case chip/Board/
worker selection passes x64/x86 (0.51/0.67 seconds). One ordinary pixel-contract
fixture now explicitly sets underline outside its two-line glyph height.
No Shared, artifact, owner input or implementation P changes; complete S7
qualification and remaining source classes still need work.

### Cursor Blink Design

CECG printed 5-12 (PDF 156), visually checked, fixes the cursor cycle to
16 vertical periods. Generic EGA uses a documented 32-period L2 reference
cycle supported by the read-only local EGA implementation, not IBM physical
timing. Cursor End bits 5/6 remain the sourced horizontal skew, not that
reference implementation's alternate blink-rate selector. Reuse field phase
bit 3 for CECG and bit 4 for generic cursor visibility. The same existing
shape helper qualifies the deadline and dirty edge independently of the
current hidden phase, preventing a hidden cursor from losing its next wake.
Logical character frames retain their cursor contract; pixel text shares
the same phase. The existing IRQ deadline stays independent. No field counter,
public API or host timer is added. Expected Video production +35..55/-10..20,
existing contract tests +60..90; all original shape assertions remain.

The existing text-phase admission now computes one mask for cursor and pixel
character blink. The query and advance share it; the shape test is independent
of visible/hidden phase, so hidden cursors retain their next deadline. Logical
and pixel capture consume the same cursor phase without changing glyph data.
Tests prove both source-relative periods in eight/nine-dot output, one-tick
edges, hidden wake, complete-cycle no-change, invalid shape, IRQ isolation,
Sequencer freeze and reset field count. Existing CGA/VGA paths remain separate.

The wider current selection passes 21/21 per width (5.07/1.75 seconds).
A subsequent reset test initially assumes dot phase zero and fails both
widths. Actual reset retains the existing logical reference retrace phase,
scaled by eight dots, while clearing the field count. Correct that test's
oracle rather than changing the cold reset behavior. Final corrected contract
passes 1/1 per width. These are bounded code/receiver proofs, not complete
S7 or actual bound-worker blink qualification. Public six, owner inputs and
artifacts remain unchanged; no implementation P is delivered.

### Bound Worker Blink Receiving Design

Extend the existing NXVM worker test, not a second execution fixture. Its
existing trace/pause helper takes the requested instruction count; all prior
cases keep four instructions. Use real paused Debug port/plane writes to
establish a small programmed text raster, then execute bounded short segments
and inspect published frames until the named visible/hidden/visible cycle
is observed. Segment limits are containment only; success requires both
actual output transitions and unchanged paused publication. No direct chip
advance, host-time-generated progress or observer-side publish is allowed.
Exercise logical cursor and pixel character blink separately so one cannot
mask the other. Expected existing App test +80/-10, no production/API/build
change. The entire S7 residual and qualification gates remain unchanged.

Implemented in that existing App receiver. Four-instruction cases retain
their original proof. New programmed one-cell/two-scanline text exercises
logical cursor and pixel character blink in separate real execution cycles.
Each observes visible, hidden and visible-again publication, verifies unchanged
paused sequence and requires a new sequence on the actual transition.
The 128-segment limit is only failure containment, not a passing checkpoint;
both named transitions must occur. All programming uses the paused Debug
lease, all progress uses the bound executor and no test publishes or advances
Video directly. Final focused worker test passes x64/x86 1/1 (1.41/0.13 s).
Its cumulative S7 file numstat is +233/-19, including earlier receiving work.
Complete S7 unit/integration/artifact qualification is still outstanding.

### Retained VGA Diagnostic Recheck

The current status read still toggles `vga_status_diagnostic_high` purely by
read count. V7 printed 3-9 instead connects bits 4/5 to selected Attribute
Controller outputs, with overscan outside display and a two-dot intermediate
nibble distinction for 256-color mode. Its four MUX pairs differ from IBM
EGA hardware, whose documentation swapped selections 01/10. This confirms
the stored flip is not a hardware source. Do not replace it with constant
zero or silently reuse generic EGA MUX/beam units: VGA raster units, real
pixel producer, overscan and the 256-color intermediate stage require a
coherent retained-capability disposition. This remains an open V28 member,
not a newly claimed selected-App guest failure or completed fix.

### CRTC Output Tri-State Admission

Original IBM printed 42 (PDF 125) and CECG printed 4-37 (PDF 131) identify
Mode Control bit 4 as output-driver enable/tri-state. Current output admission
consumes Sequencer, Graphics and Attribute gates plus selected-monitor blank,
but not this CRTC gate. Add it to that single existing output predicate only;
CPU aperture ownership remains independent. Tests must prove blank capture/
status, continuing plane writes/reads, restored capture and no storage loss
for both personalities. Expected production +1, existing tests +20..30;
no API/state or Shared change. Diagnostic override and Restore PAL semantics
remain separate unresolved source questions, not inferred from this repair.

The production change is one condition in the output predicate. Both chip
personalities retain TEXT_PIXELS identity while blank, clear pixel/palette
output, decline display-enable status, preserve CPU memory admission and
accept/read back a VRAM marker. Clearing bit 4 restores visible pixels.
The real Board receiver repeats tri-state, plane-1 writes/read-map readback
and restored capture through actual memory/port routes before its blink
deadline proof. Final contract passes 1/1 per width; Board/worker passes
2/2 per width (2.80/0.31 seconds). This is not full video/repository or
four-profile qualification. No Shared, App artifact or owner file changes.

Further blanking-source inspection confirms that horizontal start/end and
vertical end are distinct five-bit endpoint consumers, and CECG's diagnostic
override must not be conflated with monitor driver selection. Normal blank
endpoints, counter wrapping and the zero-width encoding still need a complete
source-bound admission and receiving matrix. Do not hide these members by
claiming the output tri-state gate qualifies all V18/V20 blanking.

### Programmed Blank Endpoint Design

IBM printed 26/27/40 and CECG printed 4-25/4-36 define full-counter starts
and five-bit end comparisons. Use one private interval calculation over the
existing programmed horizontal/vertical counter. A nonzero interval ends at
the next matching low-five-bit counter value, including counter reset at the
programmed total. Keep source-qualified nonzero edges separate from the
equal-low-bits encoding: treat equal start/end as an empty interval in an
explicit L2 model, consistent with CECG's stated 31-line maximum, not a claim
that IBM's undocumented zero-width priority is reconstructed. No latch,
host timer or additional phase is introduced.

The existing beam/display helper should feed status and existing text/planar
pixel sampling; output driver/monitor admission remains independent. Logical
ordinary text needs pixel presentation when blanking cuts its displayed
cells, otherwise it retains its Console route. Existing cold/unprogrammed
logical projections are not physical raster qualification. Preserve actual
IRQ start-edge calculation separately. Expected Video +45/-10 and existing
contract tests +80..120; source conflicts remain explicit and no Shared change
is allowed. Actual coverage must include low-five-bit wrap, total wrap,
width-one boundaries, untouched storage, stable reads and matching pixels.

The new independent 12-case table runs both personalities using actual chip
advance. It first fails five cases per personality: inside horizontal blank,
horizontal total/low-bit wrap, vertical end recovery and vertical total wrap.
These failures prove the missing endpoint consumers rather than relying on
old positive fixtures. Status now uses one pure interval helper for both
axes, retaining the existing raster phase and independent IRQ calculation.
Equal endpoints are explicitly labelled as the L2 empty model in source/test.
Repeated reads preserve phase/generation. CECG printed 4-25/4-36 were rendered
and visually rechecked before admitting the comparisons.

Final focused contract passes x64/x86 1/1 (0.52/0.16 seconds); the subsequent
change only gives EGA blank indices their own private names instead of using
MC6845 sync names. Pixel consumers and matching pixel proof are still pending,
so this is not blanking, S7 or complete video qualification. Shared six,
owner inputs and artifacts remain unchanged; no implementation P exists.

Pixel parity uses the existing 256-entry graphics palette: private index 16
is black blanking output, distinct from all guest four-bit color indices.
This prevents a nonblack guest palette entry 0 from tinting blank output.
The same interval helper will consume counter coordinates for status, pixel
capture and logical-to-pixel admission; no second blanking state is added.
Incomplete programmed timing retains the existing logical projection, not
a physical scanout claim. Pixel parity must exercise a deliberately nonblack
palette 0 and preserve valid adjacent content. This refinement is Core-only;
no Shared ABI or framebuffer is added. Estimate the receiving increment
approximately production +35/-10 and existing tests +25..45 lines.

Implemented pixel parity through the same interval helper. Programmed blank
cuts choose TEXT_PIXELS for otherwise ordinary text; text and planar graphics
use private black palette index 16 while guest indices 0..15 remain unchanged.
The 12-case table now checks status/pixel agreement with a white guest palette
0, including unchanged adjacent content and preserved stable reads. Separate
320x200 planar vectors prove horizontal and vertical blank/recovery for both
personalities. Their first candidate incorrectly attempted a second one-shot
controller configuration and failed; using actual guest Graphics register
writes corrects the fixture without relaxing that construction contract.
The Machine conversion receiver proves palette 0 remains white while blank
index 16 stays black across the existing validated graphics copy.

Wider current video/Board/worker passes 21/21 per width (3.67/1.57 s), then
final expanded planar contract passes 1/1 per width (0.63/0.15 s) and Machine
frame conversion passes 1/1 per width (0.09/0.08 s). This does not prove
remaining DE skew, diagnostic override/Restore, VGA or oscillator consumers,
full repository/four-App qualification or artifacts. No Shared, owner input,
MyNES or artifact changes; no complete implementation P is delivered.

### Display Enable Skew Design

IBM printed 28 and visually checked CECG 4-25 assign End Horizontal Blank
bits 5/6 to a zero-through-three-character delay of display enable. Interpret
the existing copied frame as the cropped active image: its content/width do
not grow by the delay. Status observes the actual delayed interval; diagnostic
sampling subtracts that interval's origin before sampling existing scanout.
Frame blanking tests the corresponding delayed beam position. A private pure
skew calculation owns this translation; do not add a viewport origin ABI,
second character frame or presenter offset. Ordinary text remains Console
capable when the delayed active image is not cut by blanking. Expected Video
+15..25/-5, existing tests +50..70, with both personalities and all four
delays plus a blank cut held at the same image column. Diagnostic override
outside the cropped active interval remains separately under source review.

The default project's mode setters currently program geometry/stride/fonts
but omit several full CRTC timing inputs. Their prior logical projection is
not real raster/IRQ/blink initialization. Preserve this newly explicit V09
firmware receiving obligation; it must be source-bound and qualified through
actual guest execution, not repaired by a chip default or host clock.

Implemented one private skew translation at status, diagnostic and blank
sampling. Both personalities exercise all four delays: status gates the
correct beam interval, MUX reads sample the same cropped image, ordinary
uncut text stays logical and a blank cut remains at image column one for
every delay. Pixel content/width are not expanded or shifted into a second
surface. Repeated status reads retain phase/generation. Final focused contract
passes per width, followed by the current video/Board/worker 21/21 per width
(3.79/1.35 seconds). Normal DE skew is proved; CECG diagnostic override outside
that interval, Restore, VGA and oscillator/firmware source consumers remain
open, with full final qualification and affected artifacts still required.
No Shared, owner input, MyNES, published EXE or implementation P changes.

### Default Firmware Text Timing Admission

IBM printed 64/65 (PDF 147/148) Mode 3 values are visually checked for the
existing 80x25 eight-scanline text choice: complete horizontal/vertical
totals, blanking and retrace, overflow and underline initialization. Add
those missing guest register writes and use one guest indexed-byte writer
to avoid repeating the same CRTC port mechanics. Retain current font/odd-even/
start/stride/shape preparation and preserved INT 10h registers. No host BIOS
interception, borrowed register defaults or absolute clock upgrade is added.
The real INT 10h mode-return unit must observe active and vertical-retrace
status while actual CPU execution advances guest time. Its instruction limit
is failure containment, not successful qualification by retirement count.
Expected guest assembly +30..45/-1, existing firmware test +25..40; production
ROM stays ignored until final whole-S7 artifact delivery. Other supported
graphics mode timing and selected oscillator inputs still require disposition.

The guest writer and missing Mode 3 timing are implemented. Original font,
character/attribute/stride, INT 10h register preservation, deliberate font
damage and graphics-to-text return assertions remain. The new post-INT-10h
receiver executes a guest loop and observes active, vertical retrace and a
subsequent non-retrace state in order. It neither advances Video directly nor
asserts a physical host-time rate. Final text receiver passes per width
(1.46/0.15 s); independent floppy/build regression passes 2/2 per width
(8.84/8.14 s), including assembly/invalid-font failure publication checks.
Both unit-font ROMs are identical SHA-256
`195DD05D11B99F54180DC0770EA42B639D464B03E14D7CCA22F2A868F3028226`.

This timing increment is guest assembly +31/-1 and existing App test +33/0.
Cumulative S7 Git numstat for those files is +128/-2 and +81/-7 respectively,
including prior live-font work. One guest byte-port helper is necessary;
there is no new C API/state or runtime firmware initialization path. Other
graphics timing, oscillator inputs and complete S7 qualification/artifacts
remain open; Shared six, MyNES, vendor ROMs and owner files stay unchanged.

### Default Firmware Graphics Timing Admission

The same visually checked IBM printed 64/65 tables define the existing D/E/10
choices. Initialize their missing total/blank/retrace/underline registers via
the sole guest CRTC byte writer, with each mode's overflow; D needs its
existing eight-dot divide-by-two input and 10 its programmed clock selection.
Keep current graphic layouts/strides/palette and real mode-return assertions.
Generalize the existing ordered raster receiver to run after every mode,
including final text return. Do not inject host clock defaults or claim Board
oscillator consumption from a guest register write. Expected assembly
+75..90/-3, existing test +5..10/-5. All executable/Shared/public interfaces
remain unchanged until complete qualification and final artifact delivery.

IBM printed 63 (PDF 146) is additionally rendered and visually checked:
D's clock register is 0Bh, including bandwidth, and 10's programmed Misc
value is A7h. Retain the project's selected extended, byte-addressed plane
layout and existing palette instead of mixing in another memory population's
shift-load/word layout. This is specification-based timing initialization,
not a byte-identical vendor BIOS or complete consumption of bandwidth/oscillator
inputs. The current ordered receiver now runs after D/E/10 and final text;
the independent Board/CPU timing consumers remain in the V09 residual.

Completed this guest batch without changing the chip or Board. Mode D uses
the sourced divided character clock, E/10 have their own totals/blank/retrace
and overflow, and 10 writes its clock selection. The sole guest byte writer
is reused. Every original mode picture/stride/palette and preserved-register
assertion remains, and the actual CPU receiver independently observes ordered
active/retrace/exit after all three graphics modes and final text return.
Final firmware text/modes, floppy and builder regressions pass 3/3 per width
(8.28/7.97 seconds). Both unit-font ROMs match SHA-256
`78B76CD8108BDF64CC6AC67FFFE5985E563AE07D749CF7E4E45AAE5CAA99C1D3`.

The guest increment is +78/-3; the test edits are +5/-4 (renaming the reused
receiver and adding its three-mode call). Cumulative S7 Git numstat is
assembly +206/-5 and test +82/-7. No new helper/state/API, Shared change,
protected file, owner file or artifact is added. Physical oscillator/bandwidth
consumers and the remaining complete S7/four-App final qualification stay
open; no complete implementation P or closure is claimed.

### Generic EGA Input Status 0

IBM printed 14 defines generic EGA Input Status 0 bit 7 as the live CRT state:
one while video is displayed and zero during vertical retrace. It is not the
vertical-interrupt latch used by Board IRQ routing. The old implementation
incorrectly exposed that latch. The repaired read samples the existing raster
status instead; the retained VGA branch is untouched. The chip regression
holds the IRQ latch true during active display and false during programmed
vertical retrace, proving that neither latch value determines the result. The
Board receiver reads the same port after its real IRQ edge and now correctly
observes retrace (bit 7 clear). Both focused receivers pass x64 and x86.

IBM also identifies bits 4--6 as board switch/feature-connector inputs. The
Default-PC profile does select generic EGA, but has no physical-board
declaration for those wires. Their current unasserted values are retained only
as an explicit unresolved profile-input gap; Video must not invent an
electrical default. A later descriptor-owned input model must provide the
switch and feature state together with original-board evidence. No Shared
contract, board clock, new state or public API is introduced by this repair.

### Final Residual Dispositions

The remaining rows were re-read against the current selected profiles before
the final qualification run.  A source statement alone is not a license to
invent a second clock, renderer, or board that the selected product does not
declare.

**V08 switch and feature inputs.** IBM's Input Status 0 bits 4--6 are real
physical-card inputs.  The Default-PC profile selects a generic EGA abstraction,
not a documented physical adapter with a switch/feature wiring declaration.
The neutral unasserted read is therefore retained as an explicit L1 absence of
a profile input, rather than fabricated as an electrical default.  The repaired
live bit 7 has a direct chip and Board proof; adding bits 4--6 requires a later
specific adapter profile and its source, not a Video change.

**V09 clocks and sequencer bandwidth.** CECG Table 4-3 provides oscillator
frequencies selected by Miscellaneous Output, while its Clocking Mode specifies
CRT bandwidth, serializer reload, and divide-by-two.  The selected DeskPro
profile's 16 MHz Core axis is explicitly an L2 macro axis, and `vadp_clock` is
the one Board conversion from that axis.  The existing implementation consumes
the two display-visible controls: eight/nine-dot and divide-by-two.  The
remaining bandwidth/reload controls govern RAM-cycle arbitration and serializers
that this direct copied-scanout Core does not model; changing their state cannot
honestly retime a macro axis or create a CPU contention model.  They remain
stored, source-visible register state, with no fabricated physical-clock claim.
No selected App currently has the source-qualified oscillator/monitor topology
needed to replace that single L2 Board clock.

**V20 CECG control.** CECG Control Mode bit 7 directly states that it disables
normal hardware blanking.  It is now implemented in the sole output-admission
predicate and tested for both selected monitor blank bits while retaining all
common scanout gates and CPU aperture access.  Bit 6 is a `RESTORE*` signal into
the PLS173 indexed-CRTC compatibility path (schematic 7-5), not a frame restore,
VRAM reset, or host recovery command.  The available original does not publish
the PLS173 truth table, and the selected DeskPro profile has no secondary VDU
compatibility device that consumes that path.  It is therefore correctly stored
without invented display behavior.

**V28 retained VGA diagnostic bits.** No selected App configures VGA.  V7
printed 3-9 requires Attribute-output and overscan sampling, including a
two-dot intermediate stage in 256-colour mode.  The prior read-count flip had
no hardware source, so it is removed; consecutive reads are now stable.  A
future VGA product must add a coherent VGA raster/Attribute-output producer
before claiming diagnostic bits 5--4.  This disposition does not change any
selected EGA/CECG path or use EGA's differently wired MUX as a substitute.
