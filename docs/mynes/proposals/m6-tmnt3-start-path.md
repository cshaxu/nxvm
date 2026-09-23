# M6 TMNT3 Start-Path Diagnosis And Correction

## Outcome

Establish a reproducible, bounded cause for the owner-reported failure to
leave the title flow of the owner-local Mapper-004 game, then correct one
demonstrated MyNes mechanism and retain a project-owned regression where that
is possible.  The outcome is a specific disposition, not a renewed claim that
all Mapper-004 games are compatible.

## Scope

- Exercise the existing ignored owner-local input through the production
  Driver, controller, CPU, PPU and cartridge path with a finite Start-input
  scenario.
- Distinguish controller-event delivery from guest execution, MMC3 IRQ/A12
  qualification and PPU timing/publication observations.
- Add or correct only the responsible `app-mynes` Core/Product mechanism and
  project-owned fixture coverage revealed by that evidence.
- Rebuild and run the affected MyNes tests on x64 and x86; record bounded
  owner-local observations without paths, bytes, hashes or raw traces.

## Exclusions

- No protected ROM, machine-local ROM path, ROM hash or gameplay trace enters
  the repository.
- No shared `lib`/`common` change, NXVM-product change, broad mapper rewrite,
  new compatibility claim or release artifact refresh is implied.
- No repair is selected from a title-screen frame, a finite instruction count,
or an unqualified optional-ROM probe alone.

## Corrective Continuation

The original bounded result is insufficient for a gameplay conclusion.  A
post-closure synchronized frame probe establishes that Start changes guest RAM
while both trajectories retain the same black published frame.  Corrective S3
therefore diagnoses the finite CPU IRQ, MMC3 IRQ/A12, PPU register/bus and
frame-publication matrix before any repair is selected.  A later corrective S
may change only the demonstrated MyNES owner and must prove a visible,
input-dependent state transition with an ignored probe plus project-owned
regression coverage.

S3 selects the Cartridge's MMC3 counter/reload/A12 contract as the next
receiver: `$C001` currently requests reload without clearing the live counter.
The next corrective task will repair that finite state transition and its
counter/acknowledge regression before making another visual claim.

S4 repairs that transition and retains its owned counter/acknowledgement
regression, but the ignored synchronized visual probe still has identical
black frames after Start.  The next diagnostic receiver is the PPU rendering
fetch schedule's A12-level sequence, not the already-corrected reload write.

S5 measures that sequence and finds zero qualified rises in the selected
background-low/sprite-high layout because empty sprite slots return before
their required dummy pattern fetch.  S6 owns the narrow PPU fix and a
project-owned empty-sprite Mapper-004 edge regression.

S6 restores one qualified edge on each observed visible scanline by retaining
dummy fetches and continuous raw A12 state across short gaps.  Its direct frame
comparison is still black after Start, so the next task must diagnose the PPU
pixel/scroll pipeline or the now-timed guest path rather than reopening the
scanline counter.

## Completion Standard

The task records a finite decision result: Start reaches the controller serial
port with a proven guest-side effect, or the exact upstream execution/IRQ/PPU
mechanism preventing that effect is demonstrated and repaired.  If the
owner-local input cannot provide a stable semantic checkpoint, the result
records that limitation and transfers the unresolved compatibility claim rather
than treating continued execution as success.
