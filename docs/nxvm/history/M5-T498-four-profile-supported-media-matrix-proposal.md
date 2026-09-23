# Four-Profile Supported DOS Media Matrix

> Retained proposal for the completed T498 task.

## Objective

Produce reproducible Release BYOB evidence for every supported combination of
the selected IBM 5160, IBM 5170 Model 339, DeskPro 386 Model 40 and
`default-at` profiles, their admitted CPUs, and their admitted floppy media.
Each row ends at a DOS prompt, date input, or installer-ready semantic terminal,
or names its first owner-qualified failure batch.

## Frozen Supported Surface

| Profile | CPU surface | Admitted floppy surface |
| --- | --- | --- |
| IBM 5160 Model 268 | 8088 | 360 KB |
| IBM 5170 Model 339 | 80286 | 1.2 MB native; 360 KB compatible |
| DeskPro 386 Model 40 | 80386 | 1.2 MB |
| `default-at` | 8086, 80186, 80286, 80386 | 360 KB, 720 KB, 1.2 MB, 1.44 MB |

Unsupported profile/media pairs remain explicit rejections, not fallback
aliases. This task does not make a DOS image compatible with a CPU it does not
support; a CPU-era mismatch is an external-test-input disposition, not a Core
decoder repair.

## One-Path Constraint

The profile fixes physical machine and drive personality; the session carries
one explicit media-format request; the FDD owns mounted geometry; Core owns
FDC/DMA/PIC/CPU execution. Tests may observe copied snapshots and diagnostic
facts but must not add profile-side controllers, BIOS shortcuts, image-size
inference, RAM aliases, or alternate boot paths.

## Subtasks

1. Freeze a row-complete matrix with each profile/CPU/media disposition,
   available owner-supplied input and semantic checkpoint.
2. Run every presently supplied valid row in Release mode, recording terminals
   and complete first-failure facts.
3. For each owner-qualified failure class, audit the full affected mechanism
   and implement one owner-local batch; never patch a single replay symptom.
4. Re-run all affected rows and record remaining asset-only gaps.
5. Independently review the matrix, focused gates and stripped Release
   artifact; transfer any missing external inputs without declaring them pass.

## Acceptance

Every supported row is either backed by a semantic Release terminal, explicitly
blocked only by a named missing owner-supplied input, or transferred with its
first complete owner batch. Every CPU and medium in the frozen surface has at
least one executed representative; every profile has its own lifecycle result.
No protected DOS, firmware or local path is committed.
