# M5 Core IBM CGA Completeness

## Purpose

Close the selected IBM Color/Graphics Adapter (CGA) contract through the one
Core VADP owner: original-source archive, finite function/timing ledger,
code-gap ledger, sourced implementation, guest-path proof and closure audit.

## Admission And Dependencies

This candidate follows the active EGA VADP task and precedes HDC/ATA.  It
consumes the existing VADP owner, checked B8000h memory route, copied display
snapshot boundary and profile/firmware port writers; it does not create a CGA
renderer, memory owner, scheduler or host-time path.

S1 must first admit the original IBM CGA adapter reference and its applicable
IBM PC system-board/BIOS reference into the owner-managed controller-manual
archive, with title, edition, page range, file hash, scan/OCR assessment and
provenance recorded in Checklist 1.  The
Motorola 6845 reference is an additional primary source only for behaviour the
IBM adapter document delegates to that CRTC.  No manual, third-party code,
firmware or guest media is copied into this repository.

86Box, MAME, PCjs, Bochs and QEMU are read-only corroborators.  A row is
`Manual L3` only when the original material establishes it; it is `Other L3`
only when the external models agree on an otherwise absent but bounded CGA
model; otherwise it is explicitly `fallback to L2`.  An external implementation
never overrides an IBM rule or becomes product source.

## Coverage And Completion

The frozen ledger universe is the selected IBM CGA's reset/default, 6845
index/data grammar and masks, `3D8h` mode control, `3D9h` colour select,
`3DAh` status, B8000h aperture, text/40- and 80-column layouts, every
manual-defined digital graphics layout and page/interlace addressing,
palette/video-enable effects, cursor/start/scan/retrace relations, light-pen
interface, and adapter-to-board/monitor signals.  Each row records source,
reset/cancellation, state owner, consumer, timing relation, L3/L2 disposition,
implementation status, proof and any earliest receiver.

Checklist 2 audits the same frozen row IDs against `vadp.c`, `vadp.h`, Core
memory/clock integration, default-profile firmware, profile topology and
focused/guest tests.  Every admitted Manual/Other/board L3 gap is implemented
through the existing VADP owner and verified.  A non-implementable row may
close only as L2 with the source insufficiency and board/profile receiver named;
it may not disappear behind a generic display fallback.

The task closes only after both checklists are exhaustive and mutually linked,
all eligible L3 rows have a sole implementation path and focused proof, all L2
rows have a precise reason/receiver, the similar-issue sweep covers every
existing CGA port/memory/snapshot caller, and the closure audit verifies no
duplicate VRAM, palette, mode, raster or presentation state was introduced.

## Planned Subtasks

1. **S1 - source archive and cross-model matrix.** Verify the original PDFs'
   textual/OCR quality and ambiguities; compare the finite manual rows against
   86Box, MAME, PCjs, Bochs and QEMU; create Checklist 1 with every source and
   L3/L2 disposition.
2. **S2 - implementation-gap audit.** Create Checklist 2 with the same row
   IDs; trace each current writer, VADP state, Core-time input and copied-frame
   consumer; record all missing or contradictory behaviour before repair.
3. **S3 - port/register/reset closure.** Repair the complete admitted
   `3D4h`--`3DAh` grammar, reset/cancellation and status/light-pen disposition
   in the single VADP owner, with a complete port sweep.
4. **S4 - digital memory/mode/palette closure.** Repair every admitted text
   and graphics layout, B8000h page/interlace/address relation and palette/
   video-enable effect through the existing memory and snapshot route.
5. **S5 - raster and board-timing closure.** Implement only sourced Core
   digital timing/state relations.  Export or consume the established board
   timing input where available; retain unsourced physical clock, contention,
   monitor and composite facts as explicit L2 rather than inventing cadence.
6. **S6 - firmware/profile and guest-path proof.** Reconcile generated BIOS
   mode writers and selected profile topology with the VADP contract; prove
   reset, text/graphics mode switches, memory effects and copied-frame
   isolation through direct and guest-path tests.
7. **S7 - complete closure audit.** Reconcile both ledgers row-by-row, perform
   the similar-issue sweep, run affected regressions and the current gate,
   produce the required stripped Release artifact, and transfer only explicit
   L2 boundaries.

## Architecture And Minimalism

`vadp.c` remains the only mutable CGA register/raster/palette/snapshot owner;
Core memory remains the only ordinary-RAM owner; VM/profile firmware only emits
guest-visible port and memory operations; VM/platform only consumes copied
frames.  Shared EGA/CECG machinery may be factored only when the same CGA
semantics, lifecycle and failure boundary apply and the old duplicate code is
deleted.  Similar-looking adapter-specific behaviour remains local.  No BDA
mode mirror, second decode path, compatibility renderer, live profile setter or
host-clock-derived guest time is eligible.

## Boundaries

VGA/VBE/SVGA, arbitrary EGA breadth and unsourced composite/NTSC renderer work
are not silently included.  The latter is either a Manual/Other L3 row with a
selected model and receiver or an explicit L2 disposition; this task never
claims physical monitor waveforms or host presentation cadence from a digital
CGA register fact.
