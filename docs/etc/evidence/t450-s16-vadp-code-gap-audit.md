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

## Row Dispositions

| S15 ID | Current owner and observed behavior | Focused proof | Disposition and sole receiver |
| --- | --- | --- |
| VADP-R1 | VADP owns CGA status/mode/color and Compaq-specific 3C2/3BA/3DA routes; generic EGA configuration registers only the attribute, sequencer and graphics groups.  IBM EGA Miscellaneous Output/Feature/Input Status 0 behavior is therefore not a generic EGA implementation. | `core-machine-vadp-text-status-smoke`, Compaq CECG smokes. | Partial; receiver must implement IBM EGA ports only with selected personality evidence. |
| VADP-R2 | Five indexed sequencer registers, masks and reset defaults exist; planar memory is configured once through `core_machine_vadp_configure_ega_sequencer`.  Reset does not model documented async/sync halt/high-impedance or RAM-preservation sequencing. | `core-machine-ega-sequencer-port-smoke`, planar smoke. | Partial; receiver owns reset and clocking semantics. |
| VADP-R3 | CRTC index/data state, masks, cursor/start capture and CGA/Compaq port routing are centralized in VADP.  It does not implement the entire IBM EGA CRTC register/timing/vertical-interrupt universe. | `core-machine-ega-crtc-boundary-port-smoke`, VADP text/status smoke. | Partial; receiver owns the remaining EGA CRTC contract. |
| VADP-R4 | Indexed graphics registers, masks, latches, rotate/logical operation, map selection and planar read/write route are implemented. | `core-machine-ega-controller-port-smoke`, `core-machine-ega-planar-port-smoke`. | Implemented only for current bounded planar modes; unproven register/write-mode variants transfer once. |
| VADP-R5 | Attribute index/data phase, masks, Status-1 phase reset, palette selection and captured palette are implemented.  Current status source is logical/Compaq-oriented, not a complete IBM EGA external-status model. | Controller and planar port smokes; `core-machine-ega-mode10-contract-smoke`. | Partial; receiver owns unimplemented EGA attribute/status details. |
| VADP-F1 | One VADP object owns CGA RAM and four planar buffers; Graphics Controller map selection chooses current apertures, and reset clears planar transient storage.  Installed 64/128/256-KiB EGA expansion is not modeled as the IBM manual describes. | EGA sequencer/controller/planar port smokes. | Partial; receiver owns selected EGA memory-size/expansion facts. |
| VADP-F2 | Core captures CGA graphics, high-resolution and EGA planar snapshots; mode-10 tests prove a current planar route.  BIOS mode tables, all IBM EGA modes/pages/character generator behavior and IBM EGA identity are not complete. | `core-machine-ega-mode10-contract-smoke`, `vm-ega-mode10-boot-smoke`, CGA controls. | Partial; receiver owns remaining selected mode semantics. |
| VADP-F3 | Scheduler advances VADP through its own clock ratio, while VADP uses configured logical text/CGA raster periods.  No derivation from IBM EGA 14/16-MHz choices, CRTC counts or monitor frequencies exists. | `vm-model-339-clock-contract-smoke`, text/status smoke. | Partial; receiver owns a sourced clock-to-Core-tick contract. |
| VADP-F4 | Current code deliberately permits VADP memory access through mapped handlers and has no VADP-local bus arbitrator.  Therefore the manual's two-of-five/four-of-five allocation is neither modeled nor falsely duplicated. | EGA controller/planar tests prove mapped access; no cycle-allocation proof exists. | L2 gap; receiver coordinates only with the existing board transaction owner. |
| VADP-F5 | Compaq control/light-pen/environment ports and logical status bits exist; the snapshot output is copied for consumers.  Direct-drive, feature connector, physical light pen and monitor electrical signals are unmodeled. | Compaq CECG tests and `core-machine-display-authority-smoke`. | Partial; receiver retains physical-host boundary unless selected sources require Core-visible terms. |
| VADP-T1 | `machine_display.c` validates one display configuration, configures VADP once, and VM freezes profile-derived topology.  It does not configure an IBM EGA card ROM/decode personality as described by S15. | `core-machine-display-authority-smoke`, `vm-display-composition-s5-smoke`. | Partial; receiver needs selected board/adapter evidence. |
| VADP-T2 | VM profile/composition selects generic display or the Model-40 Compaq Enhanced Color personality; configuration is frozen before Core execution.  There is no selected IBM EGA switch/jumper/monitor profile. | Model-40 CECG and IBM-5170 CGA topology smokes. | Blocked by selected IBM EGA configuration facts; receiver only. |
| VADP-T3 | The current Compaq profile supplies CECG values, while VADP maps to host-independent snapshots.  No selected IBM EGA monitor timing/electrical acceptance is present. | Model-40 CECG and EGA system smokes establish only current selected behavior. | Blocked; receiver requires a primary selected monitor/adapter source. |
| VADP-T4 | Scheduler and Core board transaction layers remain separate from VADP; VADP does not create a second bus/present loop. | `vm-model-339-clock-contract-smoke`, display-authority smoke. | Current ownership conforming; selected visibility/arbitration phase transfers once to receiver. |
| VADP-T5 | Core owns mutable VADP state; VM submits frozen configuration and publishes copied snapshots.  This is a single directed data path. | `core-machine-display-authority-smoke`, `vm-display-composition-s5-smoke`. | Current ownership conforming; unallocated adapter/clock/monitor inputs transfer once. |

## Completeness, Minimality And Transfer

All `VADP-R1`--`VADP-R5`, `VADP-F1`--`VADP-F5` and `VADP-T1`--`VADP-T5` are
present exactly once.  The audit retains the existing single VADP state owner,
single mapped VRAM path, single scheduler entry and copied consumer boundary;
it proposes no parallel IBM-EGA parser or renderer.  S16 is documentation
only, so P review requires `git diff --check` and documentation governance,
not a build.
