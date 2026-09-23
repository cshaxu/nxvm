# Hardware And Presentation Contract

Supporting [Architecture](../design/ARCHITECTURE.md); hardware references are
indexed in [References](hardware-references.md). These contracts cover the whole
App/Core component design. Their runtime implementation follows M3/M4/M5
admission; this document creates no task queue for those milestones. M2 exposes
unsupported capabilities explicitly. Product palette, text conversion, buffering
and keyboard defaults below are design choices, not Nintendo specifications.

## Machine Scheduler And Device Contracts

Use one machine-owned scheduler, no device threads. The initial gameplay profile
is NTSC RP2A03/RP2C02 with master frequency 236250000/11 Hz, CPU divider 12 and
PPU divider 4. u64 master ticks are the time authority; rational accumulators
convert to audio samples and host deadlines without per-frame rounding drift.
Fixed initial phase is an emulator policy: start counters at zero, CPU transfer
at master tick divisible by 12 and PPU step at ticks divisible by 4. At equal
ticks: PPU event, DMA/CPU bus transfer, APU tick, then IRQ/NMI sampling. This is one
declared alignment, not coverage of every real power-on CPU/PPU phase relationship.

Every device interface is executor-only and has opaque ownership or a private
value embedded by its sole owner. `core_ppu_tick(ppu,bus,out_events)`,
`core_apu_tick(apu,out_events)`, CPU/PPU register read/write, controller latch/read,
and `core_cartridge_cpu/ppu_read/write` return typed values; no device discovers
or mutates a peer. Machine connects events (NMI, IRQ, DMA, frame/sample ready) and
bus routing. PPU owns CIRAM/OAM/palette/scroll pipeline; cartridge supplies CHR
and mirroring. The bus owns CPU RAM/data latch and routes register effects.
Read side effects and side-effect-free observation are distinct operations.

OAM DMA owns source page, offset, latched byte and alignment phase in machine.
It requests the same production bus and stalls CPU advancement while PPU/APU
continue. DMC later requests bus slots through that same arbiter; it cannot read
ROM through a private shortcut. Arbitration order is reset, required DMC read,
OAM transfer, CPU. Read/write phase legality and the DMA overlap acceptance
vectors follow NESdev's DMA research; a write cycle cannot be halted as a read.
No constant cycle penalty substitutes for ordered reads/writes. A callback is
also serviced during long DMA work at most every 1024 CPU cycles; hardware clock
state survives callback parking. Pacing never drops emulated bus cycles.

## PPU Contract

The PPU is dot-stepped. The initial scope covers background/sprites, register
mirrors, scroll/address latches, OAM DMA, palette and nametable mapping, vblank
and NMI. A completed image is 256x240 immutable samples (6-bit palette index plus
3 emphasis bits per pixel); the renderer is not the GUI or an ASCII renderer.
PPU double-buffers images and swaps only after the last visible pixel. Host
conversion borrows the completed image only until the next machine advance.

| Owned mechanism | Behavioral contract |
| --- | --- |
| Frame timing | NTSC 262 scanlines, 341 dots with rendered odd-frame shortening; scanline/dot state is explicit. Do not equate one frame with a rounded host 60 Hz timer. |
| Registers | $2000-$2007 mirrored through $3FFF; status read effects, write toggle, PPUDATA read buffer/increment and palette-read exception belong to PPU. No App-maintained register cache. |
| Scroll | PPU owns v/t/x/w; register writes and rendering increments/copies share those fields. No separate "camera" state. |
| Background | Fetch nametable/attribute/pattern bytes through PPU bus, load shifters and choose pixels at the dot boundary. Left clipping and rendering-enable flags apply there. |
| Sprites | Own primary/secondary OAM, evaluation/fetch and eight sprite pipelines, priority, flips, 8x8/8x16 selection and sprite-zero hit. Overflow behavior follows the documented evaluation algorithm rather than simply counting ninth sprites. |
| Palette | Own 32-byte palette RAM and mirror rules; grayscale/emphasis recorded at pixel production so mid-frame changes are retained. |
| NMI | Export the current output line from vblank flag and enable; machine passes its level to CPU edge detection. Status access races are tested at surrounding dots. |

No full analog composite, OAM decay or arbitrary silicon-revision claim follows.
The fixed digital register/pipeline model is the design target; hardware tests
at timing boundaries are prerequisites for compatibility claims. M3 includes
register/scroll/sprite/frame fixtures and both real presentation routes; M4's
named games determine additional hardware accuracy qualification, not a rewrite
of ownership or another PPU engine.

## Pixel Representation Within Existing Lib Limits

Common's imported Window payload is indexed 8-bit, palette[256], maximum
1280x768. Core emits 256x240, stride 256. NES base color plus per-pixel emphasis
can exceed 256 distinct RGB values; this is not silently assumed to fit.
Core machine driver first builds deterministic RGB from the completed samples, then:

1. Enumerate distinct RGB in raster order. If at most 256, use that exact palette.
2. Otherwise use the 216-color cube with channel levels 0,51,102,153,204,255,
   nearest-channel rounding (ties lower), index `36*r + 6*g + b`.
3. Clear unused palette entries. Publish a complete image with valid=1/graphics=1.

This is a documented color approximation only in the second case, never a Core
hardware/timing change. No MyNes extension to Lib's pixel ABI is proposed. App
status can report the selected palette policy, not claim a calibrated CRT image.

Initial built-in RGB policy uses Nestopia's established 64-color NTSC host
palette, then generates the immutable 512-entry table by applying each captured
2C02 emphasis combination. Each emphasis bit attenuates the other two channels by
0.75; combined attenuation clamps to [0,1] and rounds to nearest 8-bit (half up).
This is a host presentation choice: the 2C02 produces composite video and has no
single calibrated RGB output. The selected base values provide a saturated,
recognizable NES image without changing PPU palette indices, timing or hardware.

## Text Conversion And Surface Switching

Core machine driver converts the same completed RGB image into `common_machine_frame`
with valid=1/graphics=0; text extent 1..80 columns and 1..25 rows; cursor hidden,
font_height=16. Stored row stride is always 80 cells. Character maps explicitly
map ASCII codepoints in both banks; unused entries are spaces, never CP437 guesses.
Window font data in this inactive text route is zero-initialized; MyNes always
uses the automatic Console route for this representation.

Default viewport is 80x25 with display aspect 4:3 and cell aspect 1:2 (product
approximation). Fit the largest centered rectangle with content columns
`min(columns,floor(rows*8/3))` and content rows
`min(rows,floor(columns*3/8))`, each clamped to at least 1; remaining cells are
black spaces. For 80x25 this is 66x25. Source rectangles partition all 256x240
pixels with integer floor boundaries. Average RGB over each nonempty rectangle;
derive luminance `(77R+150G+29B+128)>>8`. Glyph ramp is ` .:-=+*#%@`, indexed by
`floor(luminance*9/255)`; choose the nearest of the fixed 16-color RGBI palette
by squared RGB distance (tie lowest index), black background. Monochrome uses
white foreground. Both conversion and maps are bounded, deterministic and Core
owned. Text output loses detail; no status page substitutes for gameplay pixels.

The fixed 16-color palette uses RGB bits 2/1/0 at intensity 170, adding 85 to
each channel for intensity bit 3; color 6 is the conventional brown $AA5500.
All conversion buffers are allocated with the gameplay adapter before publication;
normal frame conversion performs no allocation. It scans each source pixel once
for averaging and at most 16 colors per output cell, bounded by 61440 pixels and
2000 cells. Fixed maps/palette tails are initialized, never uninitialized padding.
Large image/frame storage lives in owned heap buffers; it is never a Common
worker stack local. The driver fills Common's supplied staging frame and retains
neither that pointer nor a borrowed PPU image across a callback/machine advance.

OUTPUT_SET validates/copies all options on the paused executor, prepares any
needed fixed buffers before commit, invalidates the representation cache, and
preserves hardware/guest time. Existing Common does not publish frames while
parked in its paused debug loop: therefore selection is committed immediately,
but the new visible frame/route takes effect on explicit resume. Report this to
the user. Do not synthesize lifecycle facts or call Common's private publisher.
Conversion errors before settings commit preserve old settings. Native surface
failure after publication is a terminal host failure under Common, not a claimed
rollback of native resources or a silent fallback to the other backend.

Core publishes changed complete frames; Common owns its u32 sequence and run
generation. `frame_published` may record the last observed sequence solely to
guard the next publication from overflow: at UINT32_MAX, refuse another frame
and report the lifetime limit. This observer is not a second sequence allocator.
No frame sequence wrap is promised by the current Common consumer comparison.
Reset/load invalidate the Core cache; generation filtering remains Common-owned.

## Keyboard, Controller And Hotkeys

Default player 1: W/A/S/D directions, J=B, K=A, Enter=Start, Right Shift=Select.
Bindings contain normalized `kvm_key` plus optional scan-code/extended match;
Right Shift uses normalized Shift with scan code $36 and no extended flag on
the supported Windows host. Portable key identity is preferred for letters.
Reject duplicate bindings and hotkey collisions; no hidden text-to-button path.
Button numbering in BIND_SET is A,B,Select,Start,Up,Down,Left,Right = 0..7.
Cooked input, text events and mouse events do not reach the NES controller.
The `bind` KEY vocabulary is A-Z, 0-9, Enter, Space, RShift and arrow names;
letters normalize to uppercase key identities. RShift carries the scan filter;
other names match normalized key identity without a scan filter. Unknown names
or unsupported modifier flags reject before clearing existing held state.

Core machine driver owns physical held ledgers keyed by source_identity, scan code/key and
extended flag. It tracks bound keys only. Repeats do not add presses; release
removes the matching key and an empty source ledger is released. At most two
active KVM source ledgers with eight bound keys each; overflow is a host input
fault, never a stuck-key success. Union held bindings across valid sources,
then neutralize opposing directions. Common translates source retirement into
source-specific synthesized releases while running; the driver cannot assume
that the retirement event itself is forwarded. On rebind/full reset clear all.
App never manufactures a second input queue.

Existing Common does not forward key releases to the driver while PAUSED and
does not clear its input queue on pause. Consequently composition signals
`core_driver_request_input_reset` on PAUSED before posting the completion. Core discards
input makes while that reset is pending and clears held hardware levels before
paused debug operations. On return from the parked callback after resume, it
services sequential (not recursive) callbacks until the wake latch is quiescent,
clearing the wake latch before each callback and input after each; Common
requests another wake when its input queue
still has entries. Only then clear the pending reset and execute guest work.
Limit this drain to 256 callbacks; persistent flooding becomes a host failure.
All pre-pause control events precede the PAUSED completion that enables resume,
so their machine-queue entries are discarded by this boundary. Input arriving
during that short resume cleanup may be discarded; the user must press controls
again after a pause. No stale held button may survive into the first resumed
instruction. This uses the actual public callback/wake path and no shared edit.

Controller hardware owns strobe and shift position; high strobe reflects live A,
falling strobe captures eight buttons, subsequent reads shift A/B/Select/Start/
Up/Down/Left/Right then return ones. Core bus applies controller/open-bus bit
composition. Player 2 is disconnected in the initial gameplay profile; four-player
adapters, microphone and Zapper are unsupported capability choices.

Reserved hotkey is Esc=pause/resume when a KVM
source is active. It is registered at UI construction and is not rebound in
the initial product; user bindings cannot consume them. UI close follows Common's
pause/suppression path. Raw Console activation requires Esc registration success.
The immutable shared hotkey registry has no assumed live update API. Controller
binding updates happen paused through BIND_SET and take effect on explicit resume.

## APU, Audio And Host Pacing

APU owns two pulse channels, triangle, noise, DMC, frame sequencer and channel
status/IRQ. Register writes go through the same CPU bus. Channel timers, envelopes,
length/linear/sweep counters and DMC reader advance on their defined clock phases;
do not drive the APU with host audio callbacks. DMC memory uses the machine DMA
arbiter; CPU continues to observe elapsed stall cycles. APU returns digital
channel levels plus IRQ/DMA requests; Core machine driver owns final sample conversion.

Mix pulse and TND using the nonlinear formula in Blargg's APU reference, with
zero-denominator groups producing zero. Integrate output over rational sample
intervals, 48000 Hz mono signed-16 PCM; use fixed-point accumulators/tables and
clamp only at conversion. No whole-frame audio synthesis or drop of CPU cycles.
Optional analog filter/calibrated-output accuracy is not implied by this initial
digital-output contract. APU hardware tests compare channel state before mixer
approximation; host tests separately compare bounded sample counts and continuity.

MyNes Lib now owns a neutral copied bounded PCM
stream: create(48000,mono,s16,capacity=4096), nonblocking enqueue up to 512 samples,
signal-only set_active/flush, and stop/join/destroy with retained-on-failure
semantics. Enqueue returns copied accepted sample count; partial acceptance is
OK, a full queue returns LIMIT_EXCEEDED and zero accepted. No borrowed samples
are retained and the host thread never reenters Core. It is MyNes-local pending
a later identical shared-corpus recovery; no SoftPC source is modified here.

Composition owns the audio device lifetime and passes its neutral Lib handle to
Core machine driver. Runtime sink toggles only the neutral stream active flag on completed
RUNNING/PAUSED/STOPPED/ERROR facts; it does not mutate guest hardware. Inactive
flushes pending host samples, including the last paused tail. This ensures pause
does not depend on a Core loop currently parked inside Common. Queued samples
are copied; no borrowed Core buffer crosses the audio worker boundary.

Core machine driver paces at 1x from rational guest cycles against Lib monotonic time.
Use stop/wake-aware waits of at most 5 ms; service Common callbacks between waits.
Cap lead at 20 ms; when host is behind by over 100 ms rebase wall-time origin
and report underrun/slow-host stats instead of simulating an unbounded catch-up.
Time spent parked in an executor callback is excluded by rebasing on return
after a delay over 20 ms. Audio queue backpressure waits through the same pacing
mechanism; no second guest scheduler. Audio underrun yields silence in Lib, never
uninitialized memory. M2 intentionally runs unthrottled bounded slices.

## Persistence Boundary

Cartridge-owned persistent RAM, when a later mapper/profile admits it, is copied
at a paused executor boundary and saved by host adaptation through Lib storage.
Identity includes ROM content digest, mapper/profile and format version, not only
filename. Write-to-temp/atomic replacement requires a real upstream neutral
storage capability; do not pretend current truncate/append is an atomic save.
App config save follows the same explicit policy. Neither feature is exposed in
M2. Save states remain excluded: Common's byte-stream hooks are intentionally
unsupported and no raw C struct dump is designed as a future file format.
