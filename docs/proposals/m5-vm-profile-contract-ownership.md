# M5 VM Profile-Contract Ownership Migration

## Purpose

Move concrete CPU and machine-profile selection out of `core/machine` and
into `vm` profile/composition ownership before later baseline-machine work
multiplies profile-specific branches. Core retains one shared decoder,
execution engine, transaction lifecycle, and private injected CPU capability
and timing contract; it must not select an IBM, Compaq, XT, or other concrete
machine identity.

## Required scope

- Inventory every `core_machine_cpu_profile` selector, CPU-profile switch, and
  profile-specific configuration path in production code, tests, build graph,
  and documentation.
- Define the minimal private core contract required to execute a selected CPU
  family without core choosing a concrete VM profile.
- Make `vm/profile` and `vm/composition` select and bind the 8086, 80186,
  80286, 80386DX, and later 8088 contracts. Preserve existing CPU semantic and
  source-timing ledgers exactly during the migration.
- Keep IBM 5170, Compaq DeskPro 386, and IBM 5150/XT board topology, firmware,
  device, bus, and media choices exclusively in VM composition/profile code.
- Prove that the resulting composition has one shared core execution and
  CPU/DMA transaction path, with no per-profile duplicate executor or timing
  publisher.

## Dependencies and verification

This candidate follows the active T366 PC/AT work and precedes the queued
device-service, 80386 baseline, and 8088 baseline candidates. It must not
activate until the active T366 is closed or explicitly transfers this
cross-cutting ownership migration. Admission requires a repository-wide
ownership inventory, focused profile-selection and timing regressions for all
existing CPU families, the current smoke gate, documentation governance, and
an explicit before/after proof that core no longer selects concrete machine
profiles.

## Non-goals and stop conditions

No CPU semantic, timing-value, device, ROM, firmware, guest-media, Windows,
or physical-cycle claim changes. Stop and return a finding to Queue/TODO if a
purported profile difference is actually a shared CPU mechanism, or if moving
the selector changes a source-backed timing or instruction-retirement result.
