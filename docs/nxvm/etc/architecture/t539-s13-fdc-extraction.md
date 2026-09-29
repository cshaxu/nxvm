# T539 S13: FDC Chip Extraction Boundary

Baseline `579f4e35a`, with accepted implementation `408a31cc7`. This record
refines the S10 split using the S11/S12 repairs; it does not requalify timing
or expand the implemented command set. Current owns admission and acceptance.

## Source Review And Complete Batch

The batch consumes the finite ledger's `fdc.c`, `fdc.h` and
`fdc_observation_interface.h` row and their caller changes. Inspection covers
the entire command switch, register handlers, DMA service, seek/reset/refresh,
deadline selection, machine construction/scheduler, profile drive channel,
existing FDC fixtures and boot diagnostics.

The current 1,536-line implementation still combines three responsibilities:

- Chip: command/result phase, SPECIFY, PCN and seek completion causes, status
  construction, CHRN/byte/SCAN/FORMAT state, IRQ/DRQ and deadlines.
- PC adapter: DOR/DIR/CCR, port registration, motor/select/reset decoding,
  IRQ6/DMA binding, diagnostic ports and media-change latches.
- Drive: physical cylinder and STEP/Track0, immutable installation/geometry,
  recording-rate/pitch qualification and flat-media access.

S12 already distinguishes physical head position from PCN; they must remain
separate owners, not become copies inside a public snapshot. The scheduler
still reads `fdc.data.phase`; NXVM tests/diagnostics also read command buffers,
phase, gates and result bytes. These are explicit migration consumers, not
justifications for exposing the shared chip's mutable layout.

## Resulting Files And Dependency Direction

- `src/x86/devices/fdc8272/fdc8272_interface.h`: minimal opaque chip contract.
- `src/x86/devices/fdc8272/fdc.h` and `fdc.c`: private state and cohesive
  command implementation. Preserve handler style and copyright; no wholesale
  command rewrite, generic device registry or peer chip dependency.
- NXVM `devices/fdc.c` and `fdc.h`: retain the real PC adapter and drive
  provider, not an old controller behind a forwarding wrapper. Rename to
  `fdc_bus` only if it improves consistency after the split, using git mv.
- Shared chip fixtures live in `test/x86/devices/fdc8272`; port, media registry,
  composition and machine tests remain `test/app-nxvm`.

The chip links only Types. NXVM binds the chip to media/ports/PIC/DMA and the
existing timeline; no Common, Storage, machine pointer, media ID, PC port
number or profile name enters the chip. The borrowed provider context is
opaque, immutable as a binding and valid until chip destruction.

## Contract Decisions

1. One opaque allocated chip owns all command state. Create returns failure
   without publishing a partial instance; destroy occurs only after callers
   and routes are withdrawn. No test-only mutable state getter.
2. Status/data reads and data writes are byte operations. A rejected data read
   preserves the caller's byte, matching the current bus behavior. DMA service
   also uses bytes and a terminal-count input, never NXVM's `t_latch`.
3. Electrical inputs are sampled separately from record availability. A drive
   provider supplies READY, Track0, write protection, two-sided/fault inputs;
   STEP is an output to that provider. It retains physical position and the
   board-selected routing. PC DOR decoding never migrates into the chip.
4. Record operations use copied track/record identity and bounded byte offsets,
   plus explicit absent/protected/failure outcomes. The adapter alone converts
   them to flat-image offsets/media-registry calls. The chip alone interprets
   CHRN, SK, deleted marks, SCAN and terminal status. No second sector cache,
   logical-disk registry or controller decision in the provider.
   A successful track query supplies record bounds independently of its copied
   `id_readable` result. Command admission checks ID readability; active byte
   transfers check record bounds, preserving the accepted qualification point.
   Do not merge these facts or retain a second cached geometry owner.
5. Board input changes notify the chip through that same input contract. READY
   polling and active-transfer input revalidation are distinct operations:
   the accepted DOR write path revalidates a transfer but does not poll READY
   or enqueue a SIS cause; the existing refresh path polls READY. Preserve
   those trigger boundaries without a second input/cause owner. Reset
   cancels command/byte/seek causes and releases outputs while preserving
   frozen bindings and drive mechanics. First-reset media baseline and DIR
   change acknowledgement remain adapter responsibilities.
6. Timing inputs are already-resolved durations on the board's guest axis,
   including the qualified reset, step and byte-service paths. The existing
   logical next-step fallback remains explicit; this is not a timing upgrade.
   No CCR enum/port lookup, host time or physical-frequency guess in Shared.
7. The chip publishes IRQ/DRQ through bounded signal callbacks or copied signal
   values at the operation boundary. The adapter is the only PIC/DMA router;
   it applies the board enable wiring without maintaining another chip cause
   queue. Preserve pulse/acknowledgement ordering, not merely final levels.
8. `advance(now)` and `next_due` cover command/completion, reset, all four
   seeks and byte gates. Due-now and no-event are distinct results. Scheduler
   blocking derives from this contract, not private phases or a second
   scheduling flag. Preserve existing event ordering at equal timestamps.
9. A copied observation may expose the live diagnostic values used by current
   consumers; it is read-only and does not become a machine-control interface.
   Terminal observation remains one publication per completion; adapter owns
   the product-facing sequence/sink. Tests must not mutate copied observations
   to inject state. Original private chip tests may use their own private header.

Providers run synchronously on the sole machine execution owner and may not
re-enter advance/reset/destroy. Invalid provider output/failure has a bounded
error path; do not translate it into successful data or add BIOS exceptions.
These decisions preserve the existing S12 limited READ TRACK and flat-media
model. Full flux/rotational simulation and new timing grades are not this S.

## Construction, Tests And Acceptance

Chip allocation introduces a real failure branch into configure-FDC. Preserve
the existing port checkpoint rollback and ensure allocation, registration,
DMA/IRQ ownership and destruction unwind once; test allocation and registration
failure and repeat construction. Do not leave a bound callback pointing at a
destroyed chip.

Map every original FDC scenario to its new owner before removing its old
fixture: command/status matrix; per-drive completion identity/capacity; READY,
Track0, media loss and reset; DMA/NDMA gates and TC; deleted marks/SCAN/FORMAT;
physical seek and rate/pitch; topology/DIR; guest firmware and boot integration.
Use code-owned providers for chip-only tests, not App media or external assets.
Independent tools-off x86 build/tests must pass with no App/Common dependency.

The old `verify_fdc_state_machine_boundary.cmake` currently requires chip
phases, media calls and PIC assertion in the same NXVM file; the DMA/FDC gate
likewise searches the old combined owner. Replace these positive shape checks
with the new chip/adapter boundary and negative controls, rather than keeping
dead symbols to satisfy a historical gate. MyNES CMake has no x86 reference;
the root nevertheless builds shared tests, so distinguish suite rebuilds from
actual MyNES executable link inputs before deciding artifact impact.

Run full NXVM units on x64/x86, default integration on both, and each remaining
profile/width boot once. Rebuild all eight 0539 EXEs. Review MyNES target linkage:
rebuild its two 0043 receivers if an executable input is affected; do not edit
its source/INI merely because Shared changed. Check all six manifests, corpus
boundaries, static gates, documentation and actual diff. Commit Shared and NXVM
in separate complete target deliveries, then coordinator acceptance. No partial
P, old command path, removed coverage or machine-specific shared branch may
remain at closure.
