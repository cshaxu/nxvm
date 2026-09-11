# M5 NXVM Canonical-Library Integration Repair

## Purpose

Repair NXVM's product-side adaptation to the canonical SoftPC `src/lib`
corpus. S3 adopted its then-current owner-approved revision; S8 refreshes it
to the byte-identical SoftPC revision `291afe48a0eefb703569a5cc090b7fd54bf81388`.
NXVM does not retain a wrapper, compatibility copy, or local lib patch.

## Ownership

- Core remains the sole owner of copied guest display snapshots.
- `vm/machine` owns one machine's execution, Core snapshot capture,
  guest-input submission and actual lifecycle acknowledgements through its
  ordered executor FIFO and copied result sink.
- `vm/product` owns process-wide command/control arbitration and the application
  presentation policy.  It alone binds an actual session to a lib Window or
  Console surface and formats product-visible completion text.
- `src/lib` remains the canonical, platform-neutral presenter implementation;
  it knows neither session nor machine state.

## Owner-Approved Target Architecture

The current product binding is an intermediate T526 result. The remaining
subtasks converge it to this shape without importing SoftPC application code:

```text
core/debug      core/machine      core/product
     \               |               /
      \              |              /
       +---------- vm/machine -----------+
       |       executor request FIFO     |
       |  copied machine-result sink     |
       +-------------+-------------------+
                     |
                 vm/events
                     |
CLI + presenter input --> vm/session control FIFO --> vm/machine requests
                                          \-------> vm/presentation plan API
                                                      |
                                                   src/lib
```

- `core/debug`, `core/machine`, and `core/product` are independent Core
  modules. `core/debug` owns command parsing and a neutral opaque target
  contract only; it imports neither Core machine state nor host facilities.
  `core/machine` owns emulated state, and `core/product` retains only its
  independent product-neutral capabilities.
- `vm/events` is a value-only leaf ABI: copied request/result records and sink
  signatures. It owns no queue, state, Core pointer, session pointer, or
  policy.
- `vm/machine` is the only Core composition and executor owner. It binds the
  three Core modules, consumes its executor FIFO, and reports copied lifecycle,
  debugger, fault, and frame facts through the `vm/events` sink. It does not
  know or call `vm/session` or `vm/presentation`.
- `vm/session` is the sole product control/reducer owner. Its one control FIFO
  receives CLI requests, copied machine facts, and copied presentation input.
  It owns lifecycle decisions and run generation, submits execution requests
  to `vm/machine`, and publishes only copied presentation plans for the public
  plan API of `vm/presentation`.
- `vm/presentation` is the sole NXVM integration of `src/lib` UI components.
  It creates the one active Console or Window leaf, converts copied frames,
  and returns copied input/completion facts through the `vm/events` sink. It
  neither chooses machine lifecycle nor accesses Core.
- `vm/main` is the only VM product composition root. It constructs the three
  VM owners and supplies their callbacks. `vm/machine` and
  `vm/presentation` never link back to or retain `vm/session`.

The two FIFOs have non-overlapping purposes: session FIFO serializes product
decisions; machine FIFO serializes Core access. Neither is a forwarding copy
of the other. Every replacement deletes the former owner and route in the
same subtask; no compatibility wrapper or dual presenter/control path is
allowed.

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
   `vm_machine`; it has no runtime session manager, selected-session state,
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
8. **S8 - canonical refresh and complete consumer repair.** Inventory the
   latest owner-controlled SoftPC `src/lib` manifest and public ABI against the
   admitted NXVM corpus, preserve the source provenance, then replace the
   corpus byte-for-byte. Sweep every affected NXVM source, test and CMake
   consumer in one ledger and migrate it to the canonical contract or delete
   it as obsolete. No local lib patch, compatibility facade or duplicate
   presentation/control route may remain.
9. **S9 - independent Core debug extraction.** Move
   `core/product/debug` to `core/debug`; make its target contract independent
   of `core/machine` and `core/product`. Partition the current mixed
   `core/product/utils` surface at its real ownership boundary: debugger
   assembler/disassembler and debugger-local text helpers move with
   `core/debug`, while any retained independent product utility neither
   includes nor links debug. Then repair VM's one machine-side adapter. Remove
   the old directory, symbols and target names rather than forwarding them.
   Prove the three Core modules have no mutual source or target dependency,
   and run the complete repository-only unit suite.
10. **S10 - VM event ABI and machine executor ownership.** Establish the
    value-only `vm/events` contract, move Core assembly/executor ownership to
    `vm/machine`, and give it one request FIFO plus one copied-result sink.
    Route debugger completion, lifecycle, fault and copied display facts
    through that sink. Delete the displaced composition execution route and
    prove Core is never accessed outside `vm/machine`.
11. **S11 - session control convergence.** Replace the remaining product
    Console/control decision path with one `vm/session` FIFO and reducer. It
    accepts CLI, machine and presentation facts; owns run generation and
    lifecycle decisions; and emits only machine requests or presentation
    plans. Delete duplicate command routing, direct leaf decisions, and
    session-pointer back references from peers.
12. **S12 - presentation leaf convergence.** Move the NXVM-facing lib binding
    to `vm/presentation`. It must consume session plans, expose no native SDK
    API, retain exactly one active UI leaf, and emit copied events back through
    `vm/events`. Delete retired `vm/product` presentation/Console ownership
    rather than preserving aliases. Cover target switching, title, mouse,
    full-frame publication, shortcuts, close and session lifecycle reporting.
13. **S13 - whole-route closure.** Audit every Core/VM/lib edge against the
    target architecture; remove stale gates and replace them only with gates
    over the final owners. Run complete unit and external integration suites,
    review the actual diff for duplicate state/routes, and produce fresh
    stripped x64/x86 `0526` artifacts for native UX review.

## Acceptance

- A Core 80x25 text snapshot preserves every character and attribute through
  the NXVM `ui_frame` conversion; Console and Window therefore receive the
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
- The S9--S13 architecture has exactly three independent Core modules, a
  value-only VM event ABI, one machine executor FIFO, one session control FIFO
  and one lib-facing presentation owner; no peer retains a Core/session/UI
  internal pointer across its declared boundary.
