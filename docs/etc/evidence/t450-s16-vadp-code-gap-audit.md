# T450 S16 VADP Current-Code Gap Audit

## Scope And Method

This is the code-side companion to S15's finite IBM EGA universe.  It inspected
the one `core/machine/vadp` state owner, `machine_display` configuration and
snapshot boundary, the scheduler clock route, VM composition/profile bindings,
and focused CGA/EGA/CECG tests.  No runtime source, test, asset or external
reference changes are made here.

The implementation has one Core VADP/VRAM owner and one copied display-snapshot
output route.  Its current EGA surface is intentionally bounded and its only
named EGA personality choice is generic or Compaq Enhanced Color.  Thus an
implemented register or planar behavior is not evidence of full IBM EGA
compatibility.  Every partial, missing or source-blocked row below transfers
once to the **M5 VADP Phase Contract** receiver, which must extend that single
owner path rather than create a second renderer, register file or scheduler.

T466 S1 cross-checks the same rows against 86Box, Bochs, PCjs, MAME and QEMU.
86Box's EGA card model corroborates only labelled IBM-like state; Bochs and
QEMU are later VGA models; PCjs and MAME provide no selected IBM-EGA device
authority. Therefore none converts a partial implementation into L3 or permits
a vendor extension or host-rendering policy. The rows use IBM Manual L3 only
for directly stated behavior, Other/board L3 only for an immutable selected
adapter input, and fallback to L2 for every remaining term.

## Row Dispositions

| S15 ID | Current owner and observed behavior | Focused proof | Disposition and sole receiver |
| --- | --- | --- |
| VADP-R1 | One VADP owner now holds IBM EGA Miscellaneous Output and Feature Control state, maps the selected mono/color CRTC and Status-1 route, exposes Input Status 0 CRT state, and clears both outputs on reset. CECG retains its distinct 3C2/3BA/3DA semantics in the same owner. | `core-machine-ega-external-port-smoke`; VADP text/status and all affected CECG smokes. | Implemented Manual L3 register/reset/routing subset. Switch-sense, feature-pin and selected-board decode inputs remain fallback to L2 at the existing board/profile boundary. |
| VADP-R2 | Five sequencer registers, masks and defaults exist; one `ega_planar_active` predicate now requires both Reset bits before the existing planar provider and EGA snapshot operate. A halted provider declines to ordinary RAM and resumes the retained plane data when both bits return to one. | Sequencer, planar, mode-10, external-port and VADP status smokes. | Implemented Manual L3 Reset/halt/high-impedance relation through one owner predicate. Clocking Mode propagation, async-reset DRAM loss and board clock conversion remain fallback to L2. |
| VADP-R3 | One VADP CRTC bank now covers IBM EGA 00h--18h with manual write masks. EGA readback is limited to Start Address/Cursor 0Ch--0Fh; 10h/11h light-pen reads are zero without a selected input, and other write-only reads no longer mirror stored state. Existing CRTC reset and snapshot consumers use this same bank. | EGA CRTC boundary, planar, mode-10, VADP text/status and affected CECG smokes. | Implemented Manual L3 register grammar/access/masks. CRTC raster-to-Core time conversion, vertical IRQ delivery and light-pen/monitor inputs remain fallback to L2. |
| VADP-R4 | One VADP planar provider now implements write modes 0--2, rotate/logical/set-reset/bit-mask handling, read-map or color-compare latch reads, Test Condition high impedance, manual masks and write-only Graphics data reads.  No second memory route, latch bank or graphics state exists. | Graphics-controller, planar, EGA CRTC/mode-10, CECG and VM EGA system smokes. | Manual L3 for the selected 00h--08h data path.  Read-map 4--7, Odd/Even/Shift Register, invalid write mode 3, serializer/clock and board arbitration remain fallback to L2. |
| VADP-R5 | One VADP Attribute phase machine now accepts only 00h--13h, applies the 6/4/6/4/4-bit masks, is reset by Status-1 and keeps the existing palette/plane snapshot consumer.  3C1h returns zero because IBM Attribute state is write-only; 14h--1Fh do not create state. | EGA controller, external-port, planar, CRTC/mode-10, CECG and VM EGA system smokes. | Manual L3 for 00h--12h and phase grammar; Other L3 for 13h because the IBM page self-conflicts and all three compared models select 13h. External color/status pins, monitor effects and panning/raster conversion remain fallback to L2. |
| VADP-F1 | One VADP owns the four planar buffers and Graphics map selection; it currently retains its historical unconditional 256-KiB backing. The manual names 64/128/256-KiB card capacities and aperture locations, but not the complete extended-bank address formula. | EGA sequencer/controller/planar smokes; S7 rendered IBM memory/Memory Mode pages and read-only 86Box/Bochs/QEMU comparison. | Manual L3 capacity/location facts; fallback to L2 for the unselected card-capacity input and exact extended-bank mapping. 86Box's related 128-KiB mapping branch explicitly requests hardware confirmation, so it is not Other L3. Sole future receiver remains the VADP construction configuration and provider, not a second memory path. |
| VADP-F2 | Core captures current CGA and planar snapshots; mode 10h is proved. Complete IBM modes/pages/character generation are absent. | Mode-10 and CGA smokes. | Manual L3 source, remaining IBM mode code gap; sole receiver VADP. CECG is Other/board L3 only when selected. |
| VADP-F3 | Scheduler advances VADP by its existing ratio and text/CGA periods, not IBM EGA 14/16-MHz/CRTC formulas. | Model-339 clock and text/status smokes. | Manual L3 source values; Core conversion fallback to L2 pending selected board input. |
| VADP-F4 | Mapped handlers access VADP memory; no VADP-local arbitrator exists. | EGA controller/planar smokes. | Manual L3 adapter allocation; ISA/CPU arbitration fallback to L2 at existing board transaction owner. |
| VADP-F5 | Compaq ports/status and copied snapshots exist; direct-drive, feature connector, light pen and monitor electrical signals do not. | CECG and display-authority smokes. | Manual L3 signal relation; physical monitor/host boundary fallback to L2. |
| VADP-T1 | Display config is validated once and profile topology frozen; IBM EGA card ROM/decode personality is absent. | Display-authority and composition smokes. | Manual L3 IBM topology, but selected board/card integration fallback to L2. |
| VADP-T2 | Generic or Model-40 CECG personality is frozen; no IBM EGA switch/jumper/monitor profile exists. | Model-40 CECG and IBM-5170 CGA topology smokes. | Manual L3 IBM option facts; absent immutable profile fallback to L2. |
| VADP-T3 | Current Compaq profile maps to host-independent snapshots; no IBM EGA monitor acceptance exists. | Model-40 CECG and EGA system smokes. | fallback to L2: no selected monitor/adapter source. |
| VADP-T4 | Scheduler and board transaction stay separate; VADP creates no bus or present loop. | Clock-contract and display-authority smokes. | Manual L3 adapter boundary; selected board visibility/arbitration fallback to L2. |
| VADP-T5 | Core owns mutable VADP state; VM submits frozen config and receives copied snapshots. | Display-authority and composition smokes. | Conforming one-owner path; unselected adapter/clock/monitor inputs fallback to L2. |

## Completeness, Minimality And Transfer

All `VADP-R1`--`VADP-R5`, `VADP-F1`--`VADP-F5` and `VADP-T1`--`VADP-T5` are
present exactly once.  The audit retains the existing single VADP state owner,
single mapped VRAM path, single scheduler entry and copied consumer boundary;
it proposes no parallel IBM-EGA parser or renderer.  S16 is documentation
only, so P review requires `git diff --check` and documentation governance,
not a build.
