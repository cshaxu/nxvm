# M5 T536 Product-Namespace Layout

## Admission

The owner admitted T536 on 2026-09-22 after arranging the target files on
disk.  The task was to complete that layout across NXVM code, tests, build
rules and current documentation, without retaining old-name compatibility
paths or changing product behavior.

## S1: Product-Namespace Migration

The completed repository has three peer shared roots, `lib`, `common` and
`x86`, and one NXVM product root, `app-nxvm`.  NXVM-only tests and local
product output use matching `test/app-nxvm` and `assets/binary-nxvm` roots.
This makes a later MyNES import an ordinary peer product rather than a second
NXVM-specific architecture.

CMake, generated profile bindings, deployment, static verifiers, INI media
references and current governance documents now resolve that layout.  The
external owner archive has the matching names `profiles-nxvm` and
`media-nxvm`.  No firmware, disk image or executable is committed.

The [S1 evidence](../etc/evidence/t536-s1-product-namespaced-layout.md)
records the path sweep and verification.  Repository-only unit passes
336/336.  All 20 external integration rows register and skip because their
owner-provided external assets are unavailable in this workspace; they are not
reported as behavioral successes.
