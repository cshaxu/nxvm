# M5 T531 S1 Two-Product Convergence Ledger

## Frozen universe

This is the complete S1 planning universe: shared Library, Common UI/session/
machine/Debug/XASM32, NXVM product/VM bindings, and SoftPC `app`/`host`
bindings.  The reviewed references are NXVM `971af72f`, its pinned Library
source `3cabea6a`, and read-only SoftPC `e140ec55bbc4cd40956948e57e58dc678e54f699`.
SoftPC was not modified by this S.

The task's acceptance predicate is that every row below reaches **accepted**
with both real consumers and a regression owner, or remains explicitly blocked
with an owner-approved receiver.  Mock-only evidence cannot accept a row.

| Family | Current evidence and owner | Disposition / receiver | Required proof |
| --- | --- | --- | --- |
| Canonical Lib corpus | NXVM `src/lib` is the T530 93-file `3cabea6a` import. SoftPC `e140ec5` has 11 changed Lib files: manifest, README, Console, input, Console presenter, Window presenter and types verifier. | **Open S2.** Freeze/read the whole new corpus, then either exact-import it or record a source/ABI blocker. | Blob-by-blob comparison, manifest/dependency checks and both consumers build against the same revision. |
| Shared UI fact model | `common/session` stores only `requested_target` and immediately sets `active_target`; `common/ui_set_target()` synchronously destroys/recreates a single leaf. | **Open S3.** Replace this with requested, in-flight and acknowledged facts. | Create/destroy/switch failure and completion tests. |
| Surface topology | SoftPC distinguishes Window, VM raw Console, monitor cooked Console and broker-current identity (`app/reconciler.*`). Current Common collapses these to `NONE/CONSOLE/WINDOW`. | **Open S3.** Common owns the three facts and ordered effects. | Console text, graphics, window close, pause/resume and stale-completion matrix. |
| `console_control` | SoftPC parses `console_control=0|1` in `app/main.c` and derives its plan in `presentation_plan.c`. NXVM `vm_session_request` and YAML resolver have no field. | **Open S8.** Add the same startup configuration to NXVM and pass its copied value to Common. | NXVM configuration parse/invalid-value tests plus the shared behavior matrix. |
| Monitor and CLI | SoftPC has a monitor object and command provider; NXVM has a direct `vm/product/console.c` polling loop and table parser. Both currently bind lifecycle themselves. | **Open S4/S8.** Common session owns monitor scheduling and lifecycle notices; each product injects only CLI grammar/formatting. | Command line, prompt, transition-notice and rejected-line tests for both products. |
| Lifecycle protocol | Common requests pause/reset/resume/step/stop, but no start or media request. SoftPC runtime has start and floppy operations. | **Open S5.** Extend only the copied machine protocol needed by both adapters; no generic executor. | Ordered start/reset/pause/resume/stop and media safe-point tests. |
| Executor ownership | NXVM's bounded Core quantum runner is in `vm/machine/runtime/runner.c`; SoftPC has a CCPU worker/rendezvous in `app/runtime.c`. | **Accepted boundary.** Retain both inside injected machine adapters. | Each adapter processes Common requests at its own safe point; no second worker or direct UI route. |
| Input and run identity | Common already has copied UI facts, generation stamps and pressed-key tracking. SoftPC separately has control queue acceptance and delivery-failure records. | **Open S6.** Reconcile the stronger semantics into Common; delete duplicated product control state. | Held key/source revoke, stale run, full FIFO and delivery-failure tests. |
| Frame delivery | Common has latest-frame capture but only one target; SoftPC tracks a delivered sequence per Window and VM Console and republishes after recreation. | **Open S3/S6.** Preserve one latest copied frame and per-surface delivery facts. | Recreate surface without a new guest frame; no stale/double delivery. |
| Title and mouse | Common exposes title, capturable and release calls but has no lifecycle/reducer ordering. SoftPC derives title/freeze from actual runtime state. | **Open S3.** Drive these as acknowledged Common presentation effects. | Running/paused title, mouse release, close and target-switch cases. |
| Failure facts | Common UI silently ignores line-sink result and lacks component/broker completion/failure facts. SoftPC queues component, broker, Console and delivery failures. | **Open S3/S6.** All asynchronous failures return to the one session FIFO. | Injected failure tests; no swallowed status or waiting transition. |
| XASM32 | Common owns assembler/disassembler and NXVM uses it through Common Debug; no SoftPC consumer exists. | **Open S7.** Bind SoftPC to the same preserved command/XASM32 implementation. | Existing XASM corpus plus real SoftPC adapter command tests. |
| Debug | Common has bounded paused lease and full operation vocabulary; NXVM `vm/machine` maps it to Core. SoftPC `host/machine` has no equivalent debug target yet. | **Open S7.** Implement a real SoftPC paused target or report a concrete adapter blocker; no optional no-op. | Register/memory/port, watch, trace/break and lifecycle hand-off tests on both adapters. |
| Product-only seams | Startup configuration/composition, injected CLI handler and injected machine adapter differ. Existing NXVM direct console loop has additional control ownership; SoftPC parser has its own policy state. | **Open S4/S8.** Retain exactly these three seams; remove any other product control/reconciliation path. | Static owner sweep and two real-consumer integration review. |

## Similar-issue sweep

Reviewed all tracked NXVM `src/common`, `src/vm`, and SoftPC `src/app`,
`src/host`, `src/lib` production paths using `rg` queries for lifecycle,
presentation, Console, frame, input, monitor, Debug and `console_control`.
The finite hits are classified above.  Hardware/firmware/profile code is
excluded: it is machine-adapter implementation, not shared product control.
Tests and CMake are regression owners for later S batches, not a parallel
runtime route.

## S1 conclusion

There is no authority or licensing blocker for planning.  The material
implementation gap is the shared state machine, not an NXVM-only defect.
The later SoftPC Lib revision is a prerequisite inventory item, so S2 was
inserted before behavior changes.  This S makes no claim that Common currently
supports SoftPC or that SoftPC Debug is complete.
