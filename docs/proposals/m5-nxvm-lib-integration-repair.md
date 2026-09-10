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
   The NXVM multi-session form is a single bounded product-control FIFO of
   copied command lines, UX input, lifecycle facts, frame facts and delivery
   failures.  Every asynchronous record carries session identity plus run
   generation; stale input cannot reach a recreated session, while source
   retirement remains ledger cleanup.  The product control owner alone derives
   presenter work; a per-session presentation binding applies that work through
   lib's public Window/Console APIs and reports completion back to the same
   FIFO.  Core remains outside this route.
4. **S4 - product-control and presentation migration.** Replace the mixed
   `vm/platform` UX route rather than wrapping it.  A session publishes only
   copied lifecycle/frame/input facts through its composition contract.  One
   process-owned product-control FIFO stamps every asynchronous fact with
   `(session_id, run_generation)`, rejects stale ordinary input, and derives
   the next action from completed facts.  A per-session product presentation
   binding owns the sole lib Console or Window leaf and reports its completion
   to that FIFO.  The product Console broker remains process-owned and is the
   only authority that leases its one Console.  Delete the old direct
   session-to-leaf, leaf-to-run-handle, and `vm/platform` UX ownership paths;
   do not leave a forwarding compatibility route.
5. **S5 - closure.** Run the complete unit suite, focused native-binding
   proofs, the external integration suite, manifest/boundary gates and fresh
   optimized stripped x64/x86 `0526` artifacts.  Review the actual diff for a
   second presentation path before closure.

## Acceptance

- A Core 80x25 text snapshot preserves every character and attribute through
  the NXVM `ux_frame` conversion; Console and Window therefore receive the
  same complete copied frame.
- `src/lib` is byte-identical to the admitted SoftPC revision and manifest-valid.
- NXVM retains one Core snapshot route and one public-lib binding route, with
  no product-native presenter implementation or compatibility path.
- A lifecycle completion has exactly one product Console report; Console
  borrowing/release never changes a session's running/paused/stopped truth.
- `vm/platform` has no lib UX leaf, product hotkey, Console-broker, or
  presentation-policy ownership after S4; each has exactly one owner in the
  product control/presentation route.
- Complete unit and external integration gates pass, and both stripped
  developer artifacts are produced for revision 0526.
