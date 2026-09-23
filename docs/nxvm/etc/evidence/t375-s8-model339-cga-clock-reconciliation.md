# T375 S8: Model-339 CGA Clock And Status Reconciliation

## Primary Facts And Boundary

IBM's [Options and Adapters Technical Reference, Volume 2](https://bitsavers.org/pdf/ibm/pc/cards/Technical_Reference_Options_and_Adapters_Volume_2_Apr84.pdf),
Color/Graphics Monitor Adapter pages 15--21, identifies the Motorola 6845 as
the raster-scan controller, describes its 19 accessible registers, and assigns
CGA status bit 0 to the condition in which a regeneration-buffer access can be
made without display interference; bit 3 reports vertical retrace. The IBM
PC/AT Technical Reference [6139362](https://bitsavers.org/pdf/ibm/pc/at/6139362_PC_AT_Technical_Reference_Sep85.pdf),
System Board 1-41, identifies the 14.31818 MHz OSC signal used for color
burst. These are adapter/system facts, not an authorization to invent a
source-to-current-VADP conversion.

The primary material does not supply the complete lock-step mapping from
Model-339 source ticks through CGA timing-generator divisions, CRTC R0--R9
programming, monitor timing, status windows and ISA display contention. The
current project VADP stores but does not make R0--R9 observable, and its
`{48,8,8}` timing is a synthetic three-window model. Therefore no 14.31818 MHz
profile ratio or frame scalar is admitted here. PCjs, 86Box and MAME remain
secondary cross-check candidates only after a bounded primary measurement
contract; none was used for a numeric result.

## Implemented Status Repair

The old CGA path reported status bit 0 during active display, the inverse of
the IBM buffer-access semantics. `core_machine_vadp_status` now reports it
during horizontal blank and vertical retrace, and reports bit 3 concurrently
in vertical retrace. The existing EGA controller-configured route retains its
display-enable interpretation; this is a CGA-specific correction rather than a
generic VADP semantic rewrite.

The change retains the one VADP owner and scheduler path. It creates no
device-local timer, host-display clock, test-only API, ROM/media dependency or
contention/waveform claim.

## Focused Proof

The adjusted CGA text/status proofs cover active-display interference,
horizontal blank buffer access, vertical-retrace plus buffer access, stable
same-phase reads and reset phase. Existing CGA 640 and EGA controller port
tests preserve adjacent behavior. Local replay passed:

```text
M5:T193:S2:VADP-TEXT:OK
M5:T266:S3:VADP-TEXT-STATUS:OK
M5:T254:S2:CGA-640:PORT:OK
M5:T236:S1:EGA-CONTROLLER:PORT:OK
```

## Transfer

T375 still needs a separately admitted 6845/raster mechanism that makes the
selected R0--R9 geometry and source-derived CGA timing observable, with reset,
mode-change, status-window and copied-presentation proofs. CPU/ISA contention,
composite output, analog monitor behavior and physical signal phase remain
outside this result. This repair does not establish Model-339 L3.
