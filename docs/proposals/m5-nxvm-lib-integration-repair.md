# M5 NXVM Canonical-Library Integration Repair

## Purpose

Repair NXVM's product-side adaptation to the canonical SoftPC `src/lib`
corpus.  S3 refreshes NXVM to the owner-approved, byte-identical SoftPC
revision `7cc408ec2e27c8243ae2a3f719d313bb7e851e1d`; NXVM does not retain a
wrapper, compatibility copy, or local lib patch.

## Ownership

- Core remains the sole owner of copied guest display snapshots.
- `vm/composition/session` owns one session's execution, Core snapshot capture,
  guest-input submission and actual lifecycle acknowledgements.
- `vm/product` owns process-wide command/control arbitration and the application
  presentation policy.  It alone binds an actual session to a lib Window or
  Console surface and formats product-visible completion text.
- `src/lib` remains the canonical, platform-neutral presenter implementation;
  it knows neither session nor machine state.

## Subtasks

1. **S1 - copied-frame correctness.** Repair the Core-to-UX conversion and
   audit every differing value width, count and coordinate representation.
   Add a repository-only regression that exercises both the first and final
   cells of a full 80x25 text frame.
2. **S2 - native binding sweep.** Audit NXVM's calls into the canonical public
   UX controls for duplicate, stale or target-inconsistent requests.  Correct
   every in-scope product binding defect without adding a lib fork or native
   SDK call to VM code.
3. **S3 - canonical refresh and lifecycle/Console coherence.** Adopt SoftPC's
   owner-approved canonical corpus and product-side rule:
   report a lifecycle result only after NXVM has actually reached it.  A
   Console-mode pause must release the product Console without changing the
   session from paused to stopped; resume must reclaim the requested Console
   or Window surface.  Route running, paused and stopped outcomes through one
   NXVM product Console observer, not through lib or direct runner printing.
   The observer retains bounded completion facts and is the sole Console-loop
   formatter; an overflow is a visible product failure, never a dropped state.
   The earlier multi-session form is withdrawn before product-control
   migration. It is neither an implementation requirement nor a compatibility
   path.
4. **S4 - single-session convergence.** NXVM's product owns exactly one
   `vm_session`; it has no runtime session manager, selected-session state,
   session id, or `SESSION LIST/OPEN/SELECT/CLOSE` command surface. Startup
   resolves one YAML request before the product control loop begins. Reset,
   stop and profile construction remain VM/composition concerns. Core still
   has no cardinality policy and may be instantiated independently by another
   host. Delete, rather than cap, the product-level multi-session manager and
   its duplicate command/test paths. Preserve one process Console broker and
   one session run-generation so stale asynchronous completion from an earlier
   run cannot affect a later run.
5. **S5 - product-control and presentation migration.** Replace the mixed
   `vm/platform` UX route rather than wrapping it. The single session publishes
   copied lifecycle/frame/input facts through its composition contract. One
   process-owned product-control FIFO stamps asynchronous facts with the one
   run generation, rejects stale ordinary input, and derives the next action
   from completed facts. One product presentation binding owns the sole lib
   Console or Window leaf and reports completion back to that FIFO. The product
   Console broker remains process-owned and is the only authority that leases
   its one Console. Delete the old direct session-to-leaf, leaf-to-run-handle,
   and `vm/platform` UX ownership paths; do not leave a forwarding route.
6. **S6 - closure.** Run the complete unit suite, focused native-binding
   proofs, the external integration suite, manifest/boundary gates and fresh
   optimized stripped x64/x86 `0526` artifacts.  Review the actual diff for a
   second presentation path before closure.
7. **S7 - debugger completion event.** Delete the Core sleep callback and
   its polling loops. Extend only the neutral debugger target contract with a
   cancellable wait for the next completed execution transition; composition owns
   the host event and runner signal. The debugger remains host-neutral and
   never owns execution, while pause, trace, reset, stop and teardown each
   release a waiter exactly once.

## Acceptance

- A Core 80x25 text snapshot preserves every character and attribute through
  the NXVM `ux_frame` conversion; Console and Window therefore receive the
  same complete copied frame.
- `src/lib` is byte-identical to the admitted SoftPC revision and manifest-valid.
- NXVM retains one Core snapshot route and one public-lib binding route, with
  no product-native presenter implementation or compatibility path.
- A lifecycle completion has exactly one product Console report; Console
  borrowing/release never changes a session's running/paused/stopped truth.
- NXVM's runtime product admits exactly one session, while Core contains no
  single-session policy or product manager.
- `vm/platform` has no lib UX leaf, product hotkey, Console-broker, or
  presentation-policy ownership after S5; each has exactly one owner in the
  product control/presentation route.
- Complete unit and external integration gates pass, and both stripped
  developer artifacts are produced for revision 0526.
- Runtime debugger `GO` and trace commands wait on composition completion
  events rather than a periodic Core-to-host sleep callback.
