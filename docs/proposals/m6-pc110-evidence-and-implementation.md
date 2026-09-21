# PC110 Evidence And Component Implementation

## Goal

Implement the actual IBM Palm Top PC 110 as the second fixed-machine product
through the existing Core and Common runtime.

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
3. Compose selected hardware under `core/profile/pc110`, using one Core plan,
   timeline, media path and display owner. Share a chip only where semantics
   match; do not inherit Standard's board identity or copy its runner.
4. Qualify the second product through real firmware/media and both host
   architectures, then close the finite power/input/display/storage/lifecycle
   ledger. Unimplemented board functions remain explicit, not stubbed success.

## Stop And Exit

Do not claim physical accuracy from a reconstruction or from an emulator's
placeholder. Missing sources may support labelled L2 with evidence, never
invented L3. Source gaps requiring product exclusion or owner choice are reported.
A build alone cannot close PC110; accepted guest and lifecycle checkpoints
are required under the normal unit/integration/artifact rules.
