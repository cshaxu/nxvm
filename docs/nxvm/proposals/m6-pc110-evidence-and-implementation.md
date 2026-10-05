# PC110 Evidence And Component Implementation

## Goal

Implement the actual IBM Palm Top PC 110 after the four retained machine families
through the existing Core and Common runtime.

## Current Product Context

PC110 is a later fifth independent App, `app-mypc110`, after the
[four-App migration](../history/M5-T543-four-pc-apps-proposal.md), not a runtime option of XT, AT,
DeskPro or default PC/AT. It retains one fixed composition, a lawful external
BYOB asset root and an adjacent INI for runtime media/presentation. Its product
scope and sole artifact directory must be admitted under the App cutover
governance, not assumed to be the current `assets/nxvm/<profile>/` route.
Nothing here claims that the current four-product sources or assets implement
PC110 yet.

## Evidence First

Freeze board revision, CPU variant, chipset, memory map, display/LCD, storage,
PCMCIA, keyboard/pointer, audio and power behavior plus every firmware slot.
Distinguish IBM/manufacturer originals, observed dumps, reconstructed documents,
emulator models and placeholders. The current external PC110 research tree
does not prove complete original chipset documentation or a bootable emulator.

## Planned Batches

1. Build original function/timing List 1 and current implementation List 2,
   with source provenance, ROM layout, CMOS/configuration seed semantics and
   explicit gaps; protect vendor bytes outside the repository.
2. Admit the required 486 work through the CPU proposal. For each missing
   chipset/device admit a separate bounded implementation task, including
   coupled downstream repairs; no incremental BIOS-compatibility patches.
3. Compose selected hardware under `src/app-mypc110`, reusing qualified
   `x86/core`, `x86/chips` and matching flat `ibmpc/board-*` mechanisms with one guest
   timeline, media path and display owner. Share only where semantics match;
   do not inherit another machine's board identity or copy its runner.
4. Qualify the PC110 product through real firmware/media and both host
   architectures, then close the finite power/input/display/storage/lifecycle
   ledger. Unimplemented board functions remain explicit, not stubbed success.

## Stop And Exit

Do not claim physical accuracy from a reconstruction or from an emulator's
placeholder. Missing sources may support labelled L2 with evidence, never
invented L3. Source gaps requiring product exclusion or owner choice are reported.
A build alone cannot close PC110; accepted guest and lifecycle checkpoints
are required under the normal unit/integration/artifact rules.
