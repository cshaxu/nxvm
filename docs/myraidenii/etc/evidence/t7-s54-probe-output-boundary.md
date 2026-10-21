# T7 S54 — Probe output boundary

## Decision

The opt-in selected-set startup probe is an external-ROM integration tool, not
a guest debugger.  Its checked-in output must therefore retain only neutral
execution outcomes and aggregate observations.  It must not format an internal
fault detail, linear location, COP register/operation values, COP physical
field, or Board-control physical field.

This correction removes those formatter fields and the corresponding probe
observation reads.  It changes neither the CPU, Board, COP, Machine nor
external-ROM execution semantics.

## Retained output

The probe still reports its bounded terminal category and generic stop reason,
input delivery counters, execution/tick totals, frame aggregate summaries,
PCM aggregates, Sound aggregate counters, video aggregate counters and
sprite-protection aggregate counters.  These remain neutral diagnostics; none
is a guest byte stream, address, register snapshot or instruction trace.

## Verification

- Existing `myraidenii-selected-set-startup-probe` targets rebuilt in the
  stable x64 and x86 Ninja trees.
- Source sweep found no removed raw-field formatter token remaining in the
  probe source.
- No owner-ROM archive was read and no external staging directory was created.

## Boundary

This S does not classify S53's finite terminal.  If classification still
requires observing a selected provider failure, that requires the separately
reviewed X86 Core route-observer contract; it is not smuggled into a probe
formatting correction.
