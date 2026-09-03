# M5 External-ROM Boot-Contract Baseline And Repair

## Purpose

Restore the declared external-ROM boot paths for IBM 5170 Model 339 and then
Compaq DeskPro 386 Model 40 without returning to per-BIOS symptom patches.  The
work starts from one cross-profile boot contract: external ROM, CMOS seed and
media inputs are frozen; Core remains the sole CPU, controller and guest-time
owner; profile composition supplies only immutable board configuration.

T515 proves that every current session document resolves its external assets
through one YAML request path. Its complete integration run nevertheless
records three timeouts: IBM 5170 with 1.2 MB and 360 KB media, and DeskPro
Model 40 with its declared 1.2 MB media. This candidate repairs that functional
gap without weakening the completed asset boundary.

## Required Shared Contract

Before any owner-local correction, construct a finite reset-to-boot baseline
for the passing default PC/AT and IBM 5160 rows plus the three failing rows.
For each row, retain only neutral checkpoints and the supplied external-input
identities:

- session YAML, ROM/CMOS/media path, size and SHA-256;
- CPU execution location and guest tick;
- PIT output/deadline, PIC request/mask/service state and DMA request;
- KBC output state, FDC phase/status and relevant port transaction;
- terminal, no-progress or timeout disposition.

The implementation owner is determined from that comparison. A generic Core
repair must preserve every selected profile's controller contract.  A profile
repair must be a frozen board input or composition choice, never BIOS
recognition, keyboard injection, generated firmware or a second timing path.

Shared-controller invariants include at least: one PIT output/deadline owner;
one PIC delivery path; one DMA request owner; one KBC output/BAT delivery path;
and port 61h write bits distinct from immutable board input bits.  The task
ledger names the concrete source owner and every profile affected by each
chosen repair.

## Planned Subtasks

1. **S1 - frozen corpus and shared boot-contract ledger.** Record the five
   profile rows, all asset identities, common checkpoints, known terminal
   states and the complete owner/caller inventory.  Establish a bounded
   recorder with time, no-progress and byte limits; it is diagnostic evidence,
   not product behavior.  Resolve whether each observed divergence is CPU,
   Core controller, firmware interaction or immutable board composition before
   code changes.
2. **S2 - IBM 5170 repair.** Use original IBM/controller material and
   repository code, with externally observed emulator behavior only as labelled
   corroboration, to repair the first faulty owner mechanism for both 360 KB
   and 1.2 MB 5170 rows.  Run the affected cross-profile boot corpus after each
   complete owner-local batch; an existing passing row may not regress.
3. **S3 - DeskPro Model 40 repair.** Apply the same ledger and source-first
   method to the remaining Model 40 divergence.  Preserve its external Compaq
   ROM, CMOS seed, declared drives and fixed-disk personality; no F1 bypass,
   synthetic drive state or BIOS-specific Core branch is permitted.
4. **S4 - matrix and closure audit.** Run the complete repository-only unit
   suite and the complete YAML-driven external integration suite.  Review the
   actual source diff for duplicate state, mirrored controller paths, profile
   leakage into Core and any regression in the passing reference rows.

## Non-goals

- Do not alter `build/output` YAML or user-managed assets.
- Do not replace, generate, patch, commit or redistribute third-party ROM,
  CMOS or guest-media bytes.
- Do not relax timeouts, terminal predicates or test rows to turn a failure
  into a pass.
- Do not select an IBM 5170 video-adapter configuration; that remains the
  following video-corpus candidate.
- Do not add a VM clock, a profile-side controller state mirror, firmware
  service fallback or BIOS-specific generic-Core compatibility branch.

## Completion Standard

The frozen corpus records all selected dispositions and points each repair to
its one state owner.  IBM 5170 360 KB and 1.2 MB, then DeskPro Model 40, reach
their existing declared integration terminals through their YAML-declared
external inputs.  Default PC/AT and IBM 5160 reference rows remain green.  The
complete unit and integration suites pass, and the closing audit proves no
duplicate ROM/CMOS/media route or controller-owner divergence was introduced.
