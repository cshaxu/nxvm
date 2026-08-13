# M5 T334 EGA Registration Transaction

## Outcome

T334 closes the EGA sequencer registration failure-atomicity defect. The
memory owner now validates the provider and observer registries before
publishing either entry. VADP prepares planar VRAM first, invokes that coupled
operation, and commits its own configuration only after it succeeds.

The sole coupled caller is planar EGA configuration. Independent callers are
the immutable-ROM mapping and CPU fixture provider registrations; neither
combines provider publication with a write observer and therefore has no shared
rollback requirement.

## Delivery And Evidence

- S1 implementation: `4c4c66e3`.
- New strict owner-bound smoke:
  `current.core-machine-ega-registration-transaction-smoke`, marker
  `M5:T334:S1:EGA-REGISTRATION-TRANSACTION:OK`.
- The smoke proves allocation failure followed by same-instance retry, full
  provider capacity, and full observer capacity. Every failure preserves the
  previous counts and unconfigured VADP state; success has exactly one EGA
  provider and observer.
- Fresh GCC configuration, `verify-ega-sequencer-boundary`, current-artifact
  verification, documentation governance, and all 50 specialized verifiers
  passed. The first aggregate specialized run hit a transient Ninja dependency
  log permission failure; after no Ninja process or lock remained, the exact
  verifier and then the full specialized set passed.
- The exact `ctest -N -L current-gate` listing includes the new smoke;
  parallel `current-gate` passed 215/215 tests in 10.25 seconds.
- T334 developer artifact: `build/output/nxvm_0_5_0334.exe`, SHA-256
  `B71758ABB6EA25B6F818786FCDA8D668F773925917E0C6385F26B0D419C25579`.
  `verify-current-artifact-target` selects only `vm-0-5-0334`; the CMake build
  identity is `0.5.0334` and the retained product name is unchanged.

## Boundary

This is a narrow coupled-registration contract, not a generic transaction
framework or a registry-capacity increase. Non-planar EGA retains its sole
observer registration path because no later fallible registration remains.
The xasm API capacity/failure-semantics TODO remains separate and open.
