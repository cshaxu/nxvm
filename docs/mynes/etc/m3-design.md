# M3 Graphics And Input Design

Supporting [Architecture](../design/ARCHITECTURE.md) and the numerical
[hardware/presentation contract](hardware-presentation.md). This document fixes
M3 integration and acceptance decisions; it does not report implementation.
The owner limits the current design task to documentation and candidate planning.

## Scope And Delivered Baseline

Final M3 profile: NTSC RP2A03/RP2C02, existing strict mapper 0, CHR ROM or 8 KiB
CHR RAM, horizontal/vertical mirroring, one standard controller, background and
sprites, OAM DMA, both presentation routes. APU/DMC, PAL, new mappers, save
states, analog output and commercial-game qualification remain outside M3.
OAM DMA is included; the earlier proposal's blanket DMA exclusion is superseded.
No host-speed or audio accuracy claim follows from M3's interactive fixtures.

At design baseline `2f6126d`, actual source has CPU instruction execution and
Common integration. It has no PPU/controller clock, CHR bus, frame conversion,
input consumer, registered hotkeys or OUTPUT_SET/BIND_SET implementation.
`input_reset_requested` is settable but not consumed. None is treated as already
delivered merely because earlier architecture describes it.

The first implementation package also repairs these observed integration gaps:

| Observed source | Required repair and proof receiver |
| --- | --- |
| App command stores a Core driver and calls has_cartridge | Keep accepted media identity in App after successful Common media transaction; composition alone owns Core handle. App debug uses value-only protocol definitions. Header/dependency and production command tests. |
| App composition destroys sinks/driver even after shutdown failure | Move callback context to an owned composition object. Failed machine join retains all borrowed dependencies; failed UI join retains its session sink. Inject each construction and teardown failure; a returning stack frame must never invalidate a live callback. |
| App path parser accepts 4095 path bytes; Common capacity is 1024 | Restore the existing contract: 4095 command bytes, 1023 printable ASCII path bytes. Test boundary acceptance through a real media request, not only a null-machine failure. |
| No complete pending-command guard or configuration transaction | Common STARTING and App outstanding intent both gate machine commands. Publish effective settings only after successful paused protocol response. Test unknown, pending and host-error transitions. |
| CPU bus hook derives its time from bounded trace count | Introduce one real machine cycle owner; diagnostics cannot be the clock or stop advancing after their buffer fills. All CPU, interrupt and DMA transfers use it. |

The owner additionally requires all M2 architecture/boundary deviations to be
corrected in M3. The complete [remediation ledger](m3-remediation.md) records
R01-R12, their production fixes and receiving candidates. The table above is
only an integration overview; the ledger covers the full product-owned surface.
M2 history is retained without treating its closure claims as present proof.

## Ownership And Contracts

Keep flat `src/core` and `test/core`, following NXVM's adjacent device/private
header style. App owns policy; Core driver owns adaptation; Core machine owns
hardware. Common/Lib and their tests stay byte-identical to the adopted manifests.
No core/host, core/machine, test/support or product-specific Common branch.

| Owner/file group | State and operation contract | Failure/lifetime |
| --- | --- | --- |
| machine/clock | Own master time, CPU slots, PPU phase and wiring. Advance one slot around each production bus transfer. | Checked u64 counters; callback parking advances no guest time. |
| bus/cartridge | CPU MMIO routing; 14-bit PPU addressing; cartridge CHR access and mirroring fact. | CHR ROM writes do not mutate ROM; CHR RAM initialized at construction. No second cartridge image. |
| ppu | Own CIRAM, palette, OAM, v/t/x/w, fetch/evaluation pipelines and two 256x240 u16 sample buffers. | Create all buffers before exposure. Completed image borrowed only until next advance; public observation copies bounded values. |
| controller | Own strobe, captured 8-button state and serial index. Receive copied button levels. | No kvm_key, host event or Common type in hardware. Read side effects separate from observation. |
| driver/input | Translate selected-source keys through copied bindings, union held levels and reset latch. | Executor mutates hardware; asynchronous callbacks only set atomic reset/wake flags. |
| driver/display | Own presentation options, RGB scratch, immutable 512-color table and conversion cache. Fill supplied Common staging frame. | Allocate large buffers on create; no frame-time allocation; never retain Common staging pointer. |
| App config/keyboard/debug/command | Own defaults, grammar, pending intent, accepted media and effective config. Encode protocol 8/9 through Common lease. | No driver/hardware pointer outside composition; reject invalid candidate before mutation. |
| App composition | Own heap callback context, driver, Common machine/session/UI and immutable hotkey registry. | Stop/join producers before destroying sinks; retain dependency graph on failed join and report failure. |

Machine exposes a read-only completed-image view to its driver plus a copied
controller-level operation. Device implementation headers stay private. Debug
retains version 1 and operation numbers 1..9: OUTPUT_SET payload is 8 bytes,
BIND_SET 12, each response 12 including header/domain. All fields decode explicitly
little-endian; invalid mode, geometry, color, flags, binding conflict or capacity
rejects before mutation. Existing state save hooks remain unsupported.

## One Clock And Hardware Map

The existing instruction executor remains the single CPU engine. Each real or
dummy transfer calls the machine clock once; PPU advances three dots per CPU
slot using the fixed phase in Hardware And Presentation. CPU totals become
observations of that clock rather than a second instruction-end advance. Reset
and interrupt entry use the same mechanism. Diagnostic trace overflow is explicit
truncation or a bounded chunk drain, never lost guest cycles. DMA progress is
machine-owned and survives a run-slice budget; never append a 513-cycle lump.
STEP counts instructions, not DMA/interrupt work, while reporting all elapsed
cycles. Scheduling callbacks is forbidden midway through mutation of a bus slot.

| Address | M3 route |
| --- | --- |
| CPU $0000-$1FFF | Existing mirrored 2 KiB RAM. |
| CPU $2000-$3FFF | PPU register index address & 7, including dummy access side effects. |
| CPU $4014 write | Latch OAM DMA page; next haltable CPU read starts DMA. |
| CPU $4016 write/read | Strobe latch / player-one serial data. |
| CPU $4017 read | Disconnected player-two electrical result for the declared NES profile, not an invented second pad. |
| CPU other $4000-$401F | Existing unsupported device result; no pretend APU. Fixtures avoid these accesses. |
| CPU $4020-$FFFF | Existing open-bus/cartridge routes. |
| PPU $0000-$1FFF | Mapper-0 CHR ROM/RAM. |
| PPU $2000-$2FFF | 2 KiB CIRAM with cartridge H/V selection. |
| PPU $3000-$3EFF | Nametable mirrors. |
| PPU $3F00-$3FFF | 32 palette entries, universal-color aliases at $10/$14/$18/$1C. |

Separate PPU I/O latch, PPU read buffer and CPU open-bus latch; do not merge them.
CPU debugger PEEK continues rejecting MMIO as a whole without changing output or
device state. Device inspection must use copied observation, never MMIO reads.

PPU target behavior retains the full dot model in Hardware And Presentation:
register buffering/toggles, fine/coarse scrolling, clipped background, sprite
priority/flips/8x16/zero-hit and documented overflow evaluation, odd-frame skip,
vblank/NMI edges. Power-on suppression and reset policy must be tested under the
fixed phase, with seeded volatile memory explicitly labeled emulator policy.
Warm reset preserves owned memory and elapsed clock while executing the seven
CPU reset transfers; power reset reinitializes the machine. No phase is inferred
from frame-count rounding. PPU control/NMI reads around vblank require named
dot checkpoints; unsupported analog/sub-dot behavior is not a cycle-accuracy claim.

OAM DMA copies 256 bytes through production reads and $2004 writes, with halt and
optional alignment giving 513/514 slots without DMC. Writes cannot be halted:
RMW writes to $4014 choose the last page before the haltable read. Continue PPU
and NMI during stalls. Tests include OAMADDR wrap, parity, pending stop and read
side effects. DMC collision arbitration is a later receiver, not a shortcut in
the M3 engine.

## Frame Publication And Configuration

Use the existing numerical RGB/quantization and ASCII algorithms in
[Hardware And Presentation](hardware-presentation.md#pixel-representation-within-existing-lib-limits).
They are MyNes approximation policies, not measured NTSC colors. Generate/review
the integer palette once, then use identical data on x64/x86. Source samples are
6-bit palette plus 3 emphasis bits captured when each pixel is produced.

Publish only complete PPU frames. Core tracks completed-image revision and an
options revision; Common owns published sequence and run generation. A different
representation invalidates Core's conversion cache even if the image is unchanged.
The first ready image after reset/replacement must publish. No ready image returns
OK/invalid; conversion failure returns error, never a partial ready frame. Image
revision overflow and Common sequence exhaustion stop explicitly before wrapping.

Window output is 256x240 stride 256 indexed pixels; Console output is text only,
up to 80x25, stride 80, ASCII maps, hidden cursor. One RGB image feeds either
adaptation without changing emulated state. Fill all declared tails/resources;
black bars and unused cells are deterministic. The inactive representation must
not inherit stale resources. Identical image/options yield identical frame bytes.

Keep Common Session configured with CONSOLE display and console_control=true:
pixels select Window plus monitor, text selects the raw Console endpoint. Do not
add a format negotiation API or use Common's graphics-status text as gameplay.
Register Esc before the first raw activation and install handle_hotkey.

`mynes.ini`, placed beside the executable, is the sole startup configuration
source. It accepts `rom = PATH` and `display = window|console`; blank lines,
ordinary non-assignment lines, and text after `;` or `#` are ignored. Quoted
ROM paths may contain spaces. Keys and values are exact ASCII; unknown or
malformed assignments reject startup before Common construction. A
missing `rom` starts the cooked monitor with no cartridge; `display` defaults to
`window`. App copies the parsed choice into Driver creation, so Core owns the
selected publication representation without a runtime setting protocol. `rom
insert` and `rom eject` remain the only live media changes.

The configured representation first publishes after an explicit start/resume.
Restarting with a changed ini starts a fresh process: configuration is a startup
fact, not a hidden reset/reload operation. Native activation failure is terminal
host failure with retained dependencies.

## Controller And Management Flow

Use the existing W/A/S/D, J=B, K=A, Enter=Start, RShift=Select defaults. Driver
tracks at most two sources and eight bound keys per source; repeat is idempotent,
opposites neutral, releasing one source cannot clear another. Common validates
generations and synthesizes retirement releases; Core does not rely on receiving
the retirement event itself. TEXT/MOUSE/cooked editing never presses buttons.

Strobe-high reads live A; strobe falling freezes A/B/Select/Start/Up/Down/Left/Right;
after eight serial reads return one. CPU bus composes driven controller bits
with the declared NES-001 electrical profile, independently of the shift index:
$4016 returns (previous CPU bus & $E0) | pad_bit; disconnected $4017 returns
previous CPU bus & $E0. Unconnected D1-D4 read zero, and disconnected D0 is zero,
unlike a connected pad's post-eight one. This follows the
[controller reading table](https://www.nesdev.org/wiki/Controller_Reading).
NES-101 and Famicom microphone/expansion semantics are excluded.

Composition posts atomic input reset before PAUSED/STOPPED/ERROR completion.
Executor clears held levels before paused debug and discards queued makes while
reset is pending. On resume it clears wake, services sequential callbacks and
clears input until quiescent, maximum 256 callbacks; persistent flood is a host
fault. Reset flags are released only after draining. Tests must exercise the real
Common paused-release suppression and queued-input path.

Esc toggles a live KVM's pause/resume through the same App policy; on Console
pause it returns raw Console to the monitor. Window close pauses. Configuration
never silently resumes. Pending/STARTING rejects machine changes; help/exit
remain usable. Host ERROR permits help/exit, as required by the existing
UX contract; repair the delivered guard rather than lowering that requirement.

## Complete M3 Acceptance And Task Receivers

Candidate names are proof owners, not allocated numeric task identifiers.
Vertical means [Interactive Dual KVM Vertical](../history/M3-T11-interactive-dual-kvm-vertical-proposal.md);
PPU completion means [PPU And Clock Completion](../history/M3-T12-ppu-clock-completion-proposal.md);
input reliability means [Input And Management Completion](../history/M3-T13-input-management-completion-proposal.md);
the completed delivery is split across [Window-First Presentation Qualification](../history/M3-T16-window-qualification.md), [Product Code And Release Qualification](../history/M3-T17-release-qualification.md), [Console Presentation Qualification](../history/M3-T18-console-qualification.md), and [M3 Final Reconciliation And Closure](../history/M3-T19-final-reconciliation.md).
All runtime rows below are planned/unexecuted. Design completeness means that
each has a mechanism, receiver and observable oracle, not a green test claim.

| ID | Scope and direct oracle | Initial / final candidate owner |
| --- | --- | --- |
| G01 | Public header/link isolation; retained-on-failure heap composition; 1023 path/4095 line, pending commands and settings atomicity | Vertical / input reliability |
| G02 | All CPU/dummy/reset/IRQ slots advance PPU; run/step partition equivalence; trace saturation cannot freeze time | Vertical / PPU completion |
| G03 | All 8 registers and mirrors; read buffer/latches; CHR ROM/RAM; H/V nametables; palette aliases and reset | Vertical / PPU completion |
| G04 | Dot-driven background, scroll wraps/copies, clipping and rendering toggles; independently calculated pixel coordinates | Vertical / PPU completion |
| G05 | All sprite modes, overlap/priority/flips/zero-hit/overflow positive and negative cases; status dot boundaries | PPU completion / PPU completion |
| G06 | Frame length/odd skip/vblank/NMI transitions and status races under fixed phase; both OAM DMA parities/halts/wraps | PPU completion / PPU completion |
| G07 | Same completed pixels reach real Window and Core text reaches real Console; no graphics sent to Console | Vertical / presentation reliability |
| G08 | Black/white/checker/color-edge; 1x1 and 80x25; aspect bars/glyph thresholds/color ties; >256-color fallback; no padding leaks | Vertical / presentation reliability |
| G09 | All 256 pad states, strobe/live/frozen/after-eight reads; driven/open/disconnected bus bits | Vertical / input reliability |
| G10 | Repeats/opposites/two sources/focus/retirement/rebind; no stale key at first resumed instruction, 256-drain bound | Vertical / input reliability |
| G11 | Every configuration/hotkey in allowed, pending and error states; invalid candidate preserves settings; accepted one takes effect on resume | Vertical / input reliability |
| G12 | Window -> pause -> text -> Esc -> Window with unchanged guest; raw/cooked reader exclusivity, prompt recovery, native failure retention | Vertical / presentation reliability |
| G13 | Original ROM renders a pad-controlled marker in both backends; scripted guest checkpoints plus actual native observation on x64/x86 | Vertical / presentation reliability |
| G14 | All repository tests through registered CTest, shared manifest/independent gates, fresh task-version artifacts and PE/hash evidence | Every candidate / presentation reliability |

Original fixtures initialize through CPU/MMIO and contain no imported ROM data.
The vertical fixture uses background tile/attribute changes for a controller-driven
marker; later fixture adds sprite motion, scroll and DMA. Native tests verify
actual host pixels/cells, key response and monitor restoration separately from
in-memory publication. Shared fake-provider success cannot replace that evidence.
Each integration case has named guest checkpoints and a finite containment budget
(default ten guest frames for a scripted checkpoint, 30 seconds for automated
native completion); reaching the budget is failure. Manual observations report
what was actually observed and cannot masquerade as automation.

Full-domain claims use finite ledgers: register x operation x phase, pad bitmap
x strobe/read position, pixel pattern x representation x dimensions, lifecycle
transition x input-source state. Record untested members explicitly. Successful
builds or a title screen cannot close a row. Every final owner repairs defects
in its batch and adds S work until all required members have direct evidence.

## Research And Review Boundary

Read local public contracts at design baseline, plus the NESdev research pages
registered as PPU-REG, PPU-TIME, PAD, DMA and CLOCK. Their original observations
are hardware research, not Nintendo-issued specifications. The consultation
record is [M3 sources](m3-sources.md). No code, diagrams or external ROMs copied.
Reference emulator output is corroboration only. Exact bus/phase claims require
the corresponding original-fixture experiment before implementation acceptance.
