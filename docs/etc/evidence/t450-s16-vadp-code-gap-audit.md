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
| VADP-R1 | VADP owns CGA status/mode/color and Compaq-specific 3C2/3BA/3DA routes; generic EGA config registers only attribute, sequencer and graphics groups. IBM EGA Miscellaneous Output/Feature/Input Status 0 is absent. | `core-machine-vadp-text-status-smoke`, Compaq CECG smokes. | Manual L3 source, current IBM-port code gap; sole receiver VADP. Board decode remains fallback to L2. |
| VADP-R2 | Five sequencer registers, masks and defaults exist; planar memory is configured once. Async/sync halt, high-impedance and RAM-preservation sequencing are absent. | Sequencer and planar smokes. | Manual L3 source, current reset/clocking code gap; sole receiver VADP. Board clock propagation fallback to L2. |
| VADP-R3 | CRTC index/data, masks, cursor/start capture and CGA/Compaq routing are centralized. Full IBM EGA CRTC, timing and vertical interrupt are absent. | CRTC boundary and text/status smokes. | Manual L3 source, current CRTC code gap; sole receiver VADP. Selected raster clock fallback to L2. |
| VADP-R4 | Graphics registers, masks, latches, rotate/logical operation, map selection and planar route exist. | Graphics-controller and planar smokes. | Manual L3 source; bounded current implementation leaves remaining IBM modes as VADP code gap. |
| VADP-R5 | Attribute phase, masks, Status-1 flip-flop reset, palette selection and captured palette exist; external status is logical/Compaq-oriented. | Controller/planar and mode-10 smokes. | Manual L3 source, IBM external-status code gap; sole receiver VADP. |
| VADP-F1 | One VADP owns CGA RAM and four planar buffers; map selection chooses apertures. IBM 64/128/256-KiB expansion is absent. | EGA sequencer/controller/planar smokes. | Manual L3 source, selected IBM memory-card code gap; sole receiver VADP. |
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
