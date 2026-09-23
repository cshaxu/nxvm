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

## Completion Standard

The task records a finite decision result: Start reaches the controller serial
port with a proven guest-side effect, or the exact upstream execution/IRQ/PPU
mechanism preventing that effect is demonstrated and repaired.  If the
owner-local input cannot provide a stable semantic checkpoint, the result
records that limitation and transfers the unresolved compatibility claim rather
than treating continued execution as success.
