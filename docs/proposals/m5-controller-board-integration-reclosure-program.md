# M5 Controller Manual And Board-Integration Reclosure Program

This is shared admission context, not a Queue candidate.  It corrects a
common omission in earlier controller closure: a port/register ledger alone
does not prove that a ROM-driven machine can traverse a controller's complete
state, signal and asynchronous-event path.

## Required Evidence Per Controller Task

Each controller is one independent implementation T.  Before changing code,
the task produces and reconciles two finite lists:

1. **List 1 - normative behaviour and timing.** Every relevant command,
   register, signal, reset state, state transition, deadline and externally
   observable result is sourced to the original chip/board manual where
   available. Exact constants/formulas are Manual-L3; ranges, compatible
   emulator models and proportional board models are labelled L2; a remaining
   logical-only path is L1 and blocks closure unless explicitly transferred.
2. **List 2 - implementation gap map.** For every List-1 row, name the sole
   Core owner, producer, consumer, profile/board input (if any), current
   implementation state, regression and disposition.  A cited external
   emulator can corroborate a source or provide a labelled L2 model; it never
   silently becomes an L3 authority.

The implementation then proves the complete path rather than isolated port
effects: guest/firmware operation -> controller state transition -> device or
board signal -> Core deadline/interrupt/DMA/display result -> guest-visible
observation.  The test matrix includes every affected supported profile plus a
real-ROM integration row where that controller participates in POST or boot.
No task may add BIOS recognition, VM-side state, a second timer, direct FIFO
injection or a profile-name branch to satisfy one ROM.

## Architecture Boundary

Core remains the sole owner of controller state, transitions, deadlines,
interrupts and device-visible results.  VM freezes selected board wiring,
personality and externally sourced configuration at session construction; it
does not reproduce controller state or advance time.  A board fact is admitted
only when selected hardware documentation or an explicitly labelled L2 model
defines it.  Cross-controller wiring is expressed once at the owning boundary,
not reimplemented by every consumer.

## Common Exit Standard

A controller T closes only when its two lists are complete, every discovered
L1 is repaired to L2/L3 or transferred truthfully, the sole-owner/signal-chain
audit has no undocumented production hit, repository-only unit tests pass, and
the affected external-ROM/media integration matrix passes.  A passing
register-only test cannot substitute for the board/ROM proof.
