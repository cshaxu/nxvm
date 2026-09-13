# M5 Shared Common Product Convergence

## Goal And Approved Boundary

Deliver one byte-identical Common package for NXVM and SoftPC, using SoftPC's
control behavior and UX as the reference. Product differences are limited to
startup configuration reading and composition, the injected product CLI
handler, and the injected machine implementation. NXVM also supports
`console_control`; there is no separate product presentation-policy callback.
Both products receive Common Debug and XASM32, not a nominal optional stub.

Common owns shared control semantics, monitor scheduling, lifecycle notices,
presentation reconciliation and input routing. Branding and command grammar
remain CLI/composition inputs. Machine execution, hardware, CPU state and
safe-point formation remain in each machine adapter. Sharing does not mean
merging NXVM's bounded Core runner with SoftPC's continuous CCPU executor.

## Concrete Design

- `common/session` owns one control FIFO and reducer. Requested, in-flight and
  acknowledged machine/UI/broker state are distinct; issuing an action is not
  proof that it completed. All completion and failure facts return here.
- `common/ui` applies presentation actions through Lib public contracts and
  reports copied completion facts. Window existence, raw Console existence
  and broker-current identity are not collapsed into one target field.
- The shared session implements SoftPC's `display`/`console_control` behavior,
  monitor prompt scheduling, close/pause/resume/reset/stop and mouse behavior.
  The product CLI and Common Debug register handlers with this same monitor;
  neither creates a second control loop or reads native Console handles.
- `common/machine` owns the copied request protocol, generation and paused
  debug access contract. Start and media operations join the shared protocol
  where the two real adapters require them; no arbitrary command envelope or
  generic executor framework is introduced.
- Debug state access follows `common/debug -> common/machine -> product machine
  adapter -> machine implementation`. Lifecycle requests go through session.
  Debug never mutates a running machine, fabricates register support, or owns
  another run loop. Original table-driven command, assembler and disassembler
  style and user-visible semantics remain intact.
- Source identity, held-input cleanup, stale generations, queue saturation,
  component failures and broker failures share one documented failure path.
  Product adapters cannot silently swallow failures or mirror Common state.

## Convergence Ledger And Proposed Batches

The first batch freezes source revisions and a finite two-consumer ledger.
Each row records capability, transition/context, source evidence, present
owner, target owner, both adapters, test, disposition and obsolete path removed.
Required families are the following planned S batches; refine only from
recorded evidence, never by following successive isolated failures.

1. **S1: two-product contract and complete gap inventory.** Read actual Common
   and SoftPC app/host code; freeze Lib/reference revisions; enumerate all
   display/console_control, lifecycle, frame, input, monitor, media and Debug
   transitions. Distinguish missing implementation from already shared code.
   Produce a reviewed implementation ledger before changing runtime code.
2. **S2: canonical Lib refresh prerequisite.** Freeze the audited canonical
   SoftPC Lib revision and replace NXVM Lib as one exact corpus if the source
   review admits it.  No Common or product workaround may compensate for a
   Lib semantic delta.
3. **S3: UI actions and acknowledged completion.** Implement component/broker
   transitions and failures; preserve frame/title/mouse semantics across
   creation, switching, close and resume. Delete the replaced target-only path.
4. **S4: common control and monitor.** Converge FIFO/reducer/reconciliation,
   shared console_control behavior, notifications and prompt scheduling;
   retain only the three approved product injection seams.
5. **S5: machine protocol and adapter cutover.** Reconcile start, lifecycle,
   input, media and completion contracts for both execution models. Delete
   duplicate transport/generation state, not the necessary native executors.
6. **S6: input and failure closure.** Cover source revocation, held keys,
   hotkeys, mouse capture, stale facts, queue full, broker and component errors
   across all presentation modes and lifecycle states.
7. **S7: Debug and XASM32 for both adapters.** Complete paused register/memory/
   port access, breakpoints and execution plans, continuation and file commands;
   preserve the original command tables. Unsupported adapter operations are
   reported as unfinished work, not accepted as two-product Debug support.
8. **S8: full consumer integration and subtraction.** NXVM adds startup
   console_control input and uses the common behavior. Verify the SoftPC
   binding against the same package; remove replaced product control/UI paths.
9. **S9: package and whole-task acceptance.** Independent build/manifest,
   byte-identical consumer corpus, full matrix and failure-path review, full
   unit and integration, optimized stripped x64/x86 release artifacts.

## Verification And Exit

Every runtime S runs the complete repository-only unit suite; fixtures stay
in test code. Both real consumer adapters must compile and exercise the same
Common contract. Final acceptance requires actual two-product integration
evidence, not two fake drivers or a Common-only smoke. NXVM integration retains
the full external-assets/session-YAML suite; output YAML is not edited without
specific approval. Build both task-version architectures in `build/output`
and `assets/sessions` when the runnable implementation changes.

Accepted ledger rows require direct evidence; blocked rows remain open.
No required family may be marked deferred merely to close the task. Final
review accounts for every removed/replaced owner, source/test added/removed
lines, manifest identity, tests and residual issues. Shared files have no
product-name branch and need no per-consumer source patch.

## Dependencies, Source And Stop Conditions

The pinned Lib import is the baseline, not a promise to track a moving SoftPC
worktree. Audit and freeze any necessary later canonical revision before use.
Record provenance for imported or derived owner-controlled source. This NXVM
task does not implicitly authorize sibling-repository writes: a real SoftPC
integration change requires its approved work packet or explicit coordinated
authorization; without its evidence, two-product acceptance stays open.

Stop for incompatible licensing, unknown native semantics, missing machine
Debug capabilities, a required new product-policy seam, or a regression in
previously accepted behavior. Do not hide such gaps behind compatibility
wrappers, optional stubs, polling, new queues or duplicated machine state.
