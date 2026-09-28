# T539 S2: Concrete Extraction Contracts

Design baseline: 2197486b0. This record refines the
[S1 design](t539-independent-chip-design.md); it is not an implemented ABI.
Owner approved the first PIT Shared/NXVM extraction on 2026-09-28. Other
components retain their own implementation/review gates. No production changes
are part of S2.

## CPU Firmware Interception: Remove, Do Not Replace

The S1 observation of a bound callback is not proof of a live firmware service.
Inspection of all production/test `core_machine_firmware_provider` definitions
finds three production providers and four test providers. Every
`software_interrupt` slot is `LIB_NULL`; no assignment populates it.

- XT: `profiles/xt/rom/xt_5160_268_rom.c`.
- Model 40: `profiles/model40/rom/model40_rom.c`.
- AT/default: `profiles/default_profile/external_pc_at_rom.c`.
- Tests: two providers in `machine_firmware_capability_smoke.c`, one each in
  `machine_reset_rom_alias_smoke.c` and `vm_runner_error_propagation_smoke.c`.

Therefore the CPU extraction should delete the interception helper, callback
binding/context, copied firmware interrupt types, machine forwarding function
and unused provider member. Real-mode INT goes directly to its existing
architectural path; protected-mode behavior is unchanged. Preserve configure,
reset, after-run and immutable-ROM services, which have separate live consumers.
Re-run the caller sweep at implementation and preserve INT/IVT, fault, timing
and firmware lifecycle tests. Do not build a neutral replacement for an unused
service. This is a source-supported cleanup recommendation, not permission to
change an out-of-repository ABI consumer without review.

## FDC: Do Not Pretend Media Availability Equals Electrical READY

The current `core_machine_fdc_drive_ready_for` combines select/reset/motor/media;
Sense Drive Status separately samples the frozen `ready_mask`. Model 40 selects
`DESKPRO_REFERENCE` and ready mask 0Fh. The historical
[T431 evidence](../evidence/t431-s1-deskpro-fdc-not-ready-reference.md) qualifies
the unready READ result as reference-derived, not measured hardware behavior.

The extraction must distinguish three inputs: installed mechanical unit and
position, electrical input signals, and available media records. The controller
owns result/IRQ generation; the board cannot patch result bytes after execution.
No evidence currently proves that simply deasserting READY preserves all Sense,
seek, reset and READ behavior. Nor may the old policy merely be renamed and
presented as a manual-defined chip variant. FDC's own S must reconcile the
entire unready class, retain existing behavior until verified, and obtain review
for any resulting behavioral change. This does not block PIT extraction.

## Bus, Interrupt And DMA Boundaries

- CPU translation remains CPU-owned. Its physical bus supplies bounded reads,
  writes, I/O and architectural interrupt acknowledgement, with explicit width,
  access provenance and success/failure. No RAM/port/PIC private pointer crosses.
- Preserve current committed partial effects; a failing multi-access instruction
  is not necessarily an all-or-nothing transaction. CPU architectural faults and
  host/provider failures remain distinct. Preview/diagnostic reads must not
  acknowledge IRQs or consume device FIFOs.
- PIC owns pending/in-service state and acknowledge phases. Board connects one
  chip or a cascade; CPU requests an acknowledgement, not a private PIC scan.
- DMA owns its channels, mode, priority and phase; board supplies page/lane
  decode, bus grants and transfers. Validate a target before consuming a device
  byte when the current transaction guarantees that order. Do not promise a
  provider can roll back irreversible I/O.
- Synchronous signal propagation is allowed; callbacks may update a connected
  input, but may not recursively run/destroy the originating component.
  One machine execution owner orders operations. No chip thread or lock.
- Contracts are owner-local. Do not publish an unneeded universal bus/device
  registry or make all chips implement a common vtable.

## Time And Lifecycle

One board scheduler owns source time. Existing clock ratios retain fractional
phase in `clock.c`; chips do not store a second board clock. Each API states
whether its duration is input-clock cycles or an explicitly configured service
unit. Next-event queries are observations, not permission to skip side effects.
No-event and invalid-argument results must be distinguishable; zero delay must
not silently mean both no work and work due now. Keep existing grade/provenance.

Construct opaque chip instances while the board is not running. Bind borrowed
output contexts, install address routes, then publish the machine. Failed
construction rolls back routes before destroying their contexts. Reset retains
frozen bindings and releases asserted outputs according to existing semantics.
Stop execution before teardown; destroy chips while signal recipients still
exist. No public mutable state, reference counting or lazy singleton.

## Approved First Implementation: PIT

Create `src/x86/devices/pit825x/{pit825x_interface.h,pit.h,pit.c}` and mirrored
standalone tests. Public names below illustrate responsibility; final spelling
must be consistent with current Types/corpus rules, not an alias layer:

```text
create(8253 | 8254, out opaque handle)
destroy(handle)
reset(handle)
read_counter(handle, counter 0..2, out byte)
write_register(handle, selector 0..3, byte)
set_output(handle, counter, callback, context)
set_gate(handle, counter, boolean)
get_output(handle, counter) -> boolean
advance(handle, input_clock_cycles)
ticks_until_output(handle, counter, out cycles) -> status
```

Only needed observation: OUT and next change. Do not expose counter arrays,
callback pointers or generic state dumps to satisfy private tests. Existing
8253 ignores readback; 8254 supports it. Reading control selector 3 is a board
open-bus route, not a fabricated readable chip register.

Existing `set_output` replaces one sink; preserve that construction behavior.
The Model-40 D4 private check of `auxiliary_pit.connect.output` must instead use
board composition ownership/conflict validation, not a new public getter for
private callbacks. Primary OUT0 goes to PIC, OUT1/OUT2 to selected board wiring;
auxiliary PIT at 48h has its own handle and clock. It is not a singleton.

Preserve all waveform/BCD/latch/edge code and reset/output-release ordering.
Only local byte access replaces the old port-latch argument. NXVM uses its sole
port registry for the two possible address ranges. No seven-function legacy
port adapter copy retained in Shared. Creation/registration failures propagate
through existing machine construction cleanup. No font, media or INI changes.

Build target links only Types. Its standalone source/test entry must not require
Common, x86 Debug, NXVM generated catalogs or external assets. Extend the exact
nested-component include/link grammar and its negative tests; do not whitelist
all `devices` private cross-includes. Regenerate changed manifests. Pure timer
tests move; board IRQ0/divider/refresh/auxiliary tests remain NXVM and consume
public chip APIs. Full affected units and x64/x86 product builds are required;
the approved scope does not alter MyNES executable inputs.

## Proposed Remaining Batches

S1 inventory is closed; S2 supplies these contracts. Next admission is PIT,
followed by RTC, PIC, DMA, AT keyboard chain, XT PPI/keyboard, FDC, HDC, video,
CPU/FPU and final whole-ledger acceptance. These are forecast batches, not
pre-admitted S identifiers. Each batch reconnects consumers, removes old code,
keeps all behavioral cases and repairs its affected board paths before closure.

## Evidence And Limits

Source searches cover firmware provider types/members/bindings in src and test;
FDC unready selections and ready predicates; PIT instances and all production
calls/private field reads in devices/profiles; current x86 CMake/verify grammar.
Seven provider definitions were read, not merely counted by filename. S2 has
no source diff and makes no new runtime or manual-completeness claim.
