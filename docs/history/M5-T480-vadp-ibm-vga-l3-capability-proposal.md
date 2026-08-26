# M5 VADP IBM VGA L3 Capability

## Purpose

Extend the existing Core-owned VADP with a source-qualified IBM VGA controller
capability.  VGA reuses the proven VADP owner and EGA-common planar mechanisms,
then adds only genuine VGA state and behavior: DAC, palette mask, VGA memory
mapping, chain-4, 256-colour output and VGA-qualified raster/deadline facts.
It is not an alternate video path and does not retroactively claim that an IBM
5170 or DeskPro Model 40 shipped with native VGA.

## Required Scope

VADP remains the only owner of guest video ports, mapped memory, CRTC and
controller state, planar or VGA memory, and copied display snapshots.  Firmware
or guest port/memory writes continue through that one state to one immutable
snapshot consumed by VM presentation.  Internal source files may divide common
CRTC/text snapshot, CGA, EGA and VGA responsibilities only when the division
does not create a second mode, VRAM or frame truth.

The admitted controller target is IBM VGA register-and-memory semantics.  S1
must identify the exact primary documentation and determine whether an
available profile has a source-qualified board/card route.  Core may close a
controller-level capability without falsely enabling it for an existing
profile; board routing, option ROM/firmware programming, physical monitor
signals and unproven waits remain explicit boundaries.

## Dependencies And Completion

Follows the AT fixed-disk-controller standardization candidate so the two
machine-visible adapter families are settled before the global completeness
audit.  Consumes T467's selected IBM CGA closure, T352's selected EGA planar
paths, T386's CECG receiver and the current VADP copied-snapshot boundary.  It
does not reopen their accepted rows.

Completion requires a row-complete primary/cross-model ledger; a focused VADP
reuse audit; one VADP state and snapshot production path; source-backed VGA
register, aperture, DAC, chain-4 and selected 256-colour behavior; per-row
L3/L2/unsupported truth; and controller plus guest-path proof.  No profile may
claim VGA until its immutable board declaration proves a route.

## Planned Subtasks

1. **S1 -- IBM VGA source and cross-model ledger.** Verify primary IBM VGA
   documentation quality/page ranges, select the finite controller universe,
   and compare read-only 86Box, MAME, PCjs, Bochs and QEMU implementations.
   Distinguish a manual fact from bounded Other-L3 corroboration, L2 fallback
   and unsupported behavior; record every document ambiguity instead of
   inferring it from a later SVGA model.
2. **S2 -- VADP boundary and gap audit.** Reconcile every row with the present
   CRTC, EGA planar, port routing, memory mapping, clock/deadline, frame-copy
   and VM presentation paths.  Prove that a shared EGA/VGA mechanism can retain
   one owner, or identify the smallest owner-local replacement needed to do so.
   Do not re-audit T467/T352/CECG rows outside their direct compatibility edge.
3. **S3 -- common EGA/VGA controller closure.** Consolidate the selected common
   CRTC, sequencer, graphics, attribute, latch and aperture behavior in the
   existing VADP state.  Delete an obsolete duplicate path if one is found;
   profile code remains construction-only and cannot mutate controller state.
4. **S4 -- VGA-only closure.** Implement source-qualified DAC/palette-mask,
   memory-map, chain-4 and 256-colour semantics, together with real
   source-qualified output/deadline behavior.  Each unproven physical/board
   parameter stays L2 or unsupported; no guessed video timing is accepted.
5. **S5 -- profile admission and closure audit.** Bind VGA only where a selected
   profile/card contract establishes its port, memory, IRQ/clock and firmware
   route; otherwise retain the Core capability unselected.  Run focused port,
   memory, snapshot and guest-path proof plus the full current gate; publish a
   release and transfer only exact residuals.

## Architecture And Minimalism

The target data flow is exactly:

```text
guest or firmware port/memory write
  -> VADP sole mutable state
  -> copied display snapshot
  -> VM presentation
```

Do not create a `vga_state` beside VADP, a renderer-owned mode/VRAM cache, a
parallel BIOS-video path, a VBE/SVGA framework, or a generic live register
setter.  A common helper is allowed only when it deletes duplicate EGA/VGA
controller code while operating on the one VADP state.  VGA-specific code must
be an explicit VADP sub-responsibility, not a peer device or VM feature.

## Non-goals And Stop Conditions

This task excludes VBE, SVGA vendor extensions, accelerated graphics, analog
monitor/composite fidelity, raw DAC waveform simulation, arbitrary ISA/MCA
card catalogues, unselected option ROMs, and L4 physical timing.  It does not
make current AT/DeskPro profiles claim VGA hardware.

Stop and transfer a row if no primary controller or selected-board source
defines it, if its only support would require a second VADP truth, or if a
profile-specific board mechanism cannot be represented by one immutable
declaration and existing Core owner routes.
