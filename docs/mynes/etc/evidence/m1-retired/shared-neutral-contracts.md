# Neutral Driver And Paused Debug Transport

## Goal And Dependency

Consume the accepted [import/build](../../../history/M1-T2-shared-refresh-import.md) baseline and the
[M1 context](first-emulator-increment-planning.md). Remove CPU-specific public
records and command semantics from the shared corpus while preserving the one
paused executor rendezvous. No NES or x86 engine is introduced.

Before extraction admission, reconcile the shared-foundation source/build gate
with these explicitly optional x86 roots through the required governance path.
The present gate admits only the imported roots; this proposal does not bypass
it or authorize source scaffolding. Keep neutral-root checks separate from optional
component checks without weakening dependency or product-exclusion constraints.

## Scope And Proof Batches

- Inventory every driver callback, debug operation, register/address record,
  assembler/disassembler consumer and affected inherited test before changing ABI.
- Replace x86 operation/register/snapshot structures with bounded copied request
  and result transport, operation identity, generation/lease validation, explicit
  lengths/status and executor-side product dispatch. No frontend function-pointer
  request, universal CPU schema, borrowed deferred payload or direct machine access.
- Specify payload/result limits, ownership, null/size validation, cancellation,
  reentrancy and output-on-failure before implementation. Derive bounds from
  inspected consumer operations; oversized requests fail without partial publication.
- Extract common/debug and common/xasm32 into optional x86-debug and x86-xasm32
  components as mapped by [Source Layout](../../../design/CODING.md). Preserve source,
  notices, behavior and focused tests outside the four neutral roots; this is not
  deletion of x86 capabilities. x86-debug uses neutral Common transport and may
  use x86-xasm32. Common's default build has no dependency on either component.
  Receiving SoftPC/NXVM App composition opts in and connects the machine adapter;
  do not create a MyNes app/debug placeholder or edit a sibling repository.
  Port shared mechanism tests to fake providers; retain x86 tests with their owner.
- Product-owned headers define operation meanings and payload/result layouts.
  Common only defines transport identity, bounded lengths/capacities, copying,
  lifecycle and errors. Test that machine adapters can consume x86 protocol values
  without linking command parsing or including Common implementation headers.
- Classify media/state-stream/heartbeat hooks as neutral retained capabilities or
  product adaptation; absent hooks explicitly reject. Retain signal-only stop/wake
  hooks, with hardware mutation confined to the executor. Keep generic storage
  and state-stream APIs typed; move drive geometry, device identifiers and any
  single-floppy assumptions into product adaptation as needed. Do not add a
  universal disk framework or opaque-message wrappers around already neutral APIs.
- Preserve Lib bytes and existing Common frame-kind mode selection. Any upstream
  Lib prerequisite follows the shared context's reviewed-import gate.

## Acceptance And Stop Conditions

Two fake providers with deliberately different payload layouts exercise the same
production rendezvous, without CPU engines. Prove paused success, running-state
rejection, malformed/oversized input, stale lease/generation, cancellation/shutdown,
unsupported operation, unchanged failure output and no callback after teardown.
The final shared public/build surface has no concrete x86 or NES semantics; static
checks include negative fixtures and actual call-graph review. Build Common without
x86 components; separately build/test the extracted components with fake machine
access where needed, preserving assembly/debug semantics without an engine.
Record relocated/replaced tests individually and verify Lib baseline identity.
Run the common completion contract.

Stop if preserving an actual consumer requires new product semantics in Common.
Hand off the accepted ABI and adapter migration table to lifecycle work and final
transfer qualification; changes to SoftPC/NXVM repositories require separate admission.
