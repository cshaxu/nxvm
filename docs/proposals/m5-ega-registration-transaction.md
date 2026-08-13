# EGA Registration Transaction

Repair the EGA sequencer configuration failure-atomicity defect: a failed
planar-VRAM allocation or device-provider registration must not consume a RAM
write-observer slot, leave a provider behind, or make a later valid
configuration impossible.

## Scope

The shared owner is `core_machine_memory`: it owns both fixed-capacity
registries and their publication. `core_machine_vadp_configure_ega_sequencer`
is the sole current caller that needs the coupled planar-provider and dirty
observer registration. The repair must prepare VADP-owned VRAM first, then ask
memory to validate and publish the coupled registrations as one operation; it
must publish VADP configuration only after that operation succeeds.

The mechanism contract is:

- every unsuccessful sequencer configuration leaves VADP configuration,
  planar-VRAM ownership, provider count, and observer count unchanged;
- provider and observer validation completes before either registry count is
  published;
- a later retry after allocation or either capacity failure can succeed when
  the relevant resource is available; and
- success has exactly one EGA provider and one EGA observer.

## Non-goals

- EGA register, aperture, raster, frame, port, or guest-visible behavior.
- Changing registry capacities, exposing mutable registry state in a public
  product ABI, or adding a VADP-specific memory API.
- A general resource-transaction framework, allocator replacement, or xasm
  capacity/failure-semantics work.

## Proposed Subtasks

### S1 - Coupled registration transaction and regression

Inventory the memory registry callers and every fallible point in EGA
sequencer configuration. Implement the narrow memory-owned coupled
provider/observer registration operation, migrate the sole EGA caller, and add
an owner-bound smoke. The smoke injects planar allocation failure and exhausts
each registry independently; it proves unchanged state, successful retry, and
exactly-one registration after success. Build the T334 artifact and close with
the full current gate.

## Acceptance And Exit

The task closes only when the shared memory registry is failure-atomic for the
coupled EGA operation; the test proves all three failure classes and retry;
the current artifact is `0.5.0334`; and focused, governance, and current-gate
verification pass. Any additional coupled registry caller found by the sweep
is either covered by the common operation or explicitly transferred to TODO or
Queue with its semantic reason.

## Evidence And References

- `src/core/machine/memory.c`, `src/core/machine/memory.h`, and
  `src/core/machine/vadp.c`.
- Existing EGA sequencer and planar smoke coverage under `tests/machine/`.
- [Execution Rules](../rules/EXECUTION.md),
  [Architecture Rules](../rules/ARCHITECTURE.md), and
  [Coding Rules](../rules/CODING.md).
