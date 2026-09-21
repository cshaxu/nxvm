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
  debug access contract. It optionally notifies the bound product driver only
  when a copied request makes its FIFO nonempty, so the product can wake its
  existing executor; the notification carries no request or state and Common
  creates no executor. Start and media operations join the shared protocol
  where the two real adapters require them; no arbitrary command envelope or
  generic executor framework is introduced.
- `common_machine_debug_execute_with_lease()` remains a synchronous typed
  adapter call. Each product adapter owns any thread-affinity bridge behind
  that callback: NXVM may access its paused Core boundary directly, while
  SoftPC synchronously relays to its existing CCPU rendezvous and returns the
  copied result. Common does not gain a second Debug queue or executor.
- Debug state access follows `x86/debug -> common/machine -> product machine
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
   console_control input and uses the common behavior. Specify SoftPC's
   synchronous CCPU-affinity adapter behind the unchanged Common Debug API,
   then verify the SoftPC binding against the same package and remove replaced
   product control/UI paths.
9. **S9: Common API and XASM32 cleanup.** Remove every audit-proven unused
   public Common API and its parallel fallback path; state the borrowed-binding
   and reducer-thread contracts; then remove all current XASM32 compiler
   diagnostics without changing its original table-driven parsing,
   assembly/disassembly or user-visible Debug behavior.
10. **S10: canonical Lib refresh.** Freeze SoftPC's current project-owned
    `src/lib/` revision, replace NXVM's complete Lib corpus byte-for-byte and
    make only the necessary Common/VM consumer adaptations.  Prove exactness,
    manifest/component boundaries and both NXVM architectures; do not claim
    whole-task two-product execution acceptance.
11. **S11: canonical KVM Lib refresh.** Freeze SoftPC's current project-owned
    `src/lib/` revision; atomically replace NXVM's complete Lib corpus
    byte-for-byte; accept the source-proven `ui-*` to `kvm-*` component rename
    through CMake, Common, VM and test consumers; and delete every retired
    component target/include/path rather than retaining a compatibility alias.
    Prove exact corpus equality, KVM-name gate, manifest/component boundaries
    and both NXVM architectures. This does not claim whole-task two-product
    execution acceptance.
12. **S12: canonical Common package import and NXVM adapter cutover.** Freeze
    the audited project-owned SoftPC `src/lib`, `src/common`, `test/lib` and
    `test/common` corpus; replace NXVM's four trees byte-for-byte; make the
    NXVM product adapter directly consume that corpus without a compatibility
    shim; and prove corpus identity, Common/Lib gates, both NXVM architectures
    and the complete repository-only unit suite. This is an NXVM import and
    consumer-cutover subtask, not whole-task two-product execution acceptance.
13. **S13: canonical Lib and Common refresh.** Freeze SoftPC's next approved
    project-owned `src/lib`, `src/common`, `test/lib` and `test/common` corpus;
    replace NXVM's four trees byte-for-byte; directly adapt NXVM consumers to
    its public contracts; and prove corpus identity, component boundaries,
    both NXVM architectures and the complete repository-only unit suite.
    This remains an NXVM import and consumer-cutover subtask, not whole-task
    two-product execution acceptance.
14. **S14: raw-console monitor prompt correction.** Repair the NXVM injected
    monitor callback so it arms a prompt only when Common reports that the
    cooked monitor is the Current Console. Prove raw guest input cannot
    publish monitor text, retain the one broker and one guest-input route, and
    make no Lib/Common corpus change.
15. **S15: VM-machine media ownership layout.** Move NXVM's FDD/HDD
    Core-media-provider adaptation from `src/vm/media/` to
    `src/vm/machine/media/`. Update direct includes, CMake source lists and
    source-layout checks atomically; retain the existing `vm-media` target and
    all public `vm_machine_*` names. This is a structural relocation only: it
    neither changes storage semantics nor tries to make NXVM controller
    adaptation a Common capability.
16. **S16: NXVM App consolidation and canonical shared-corpus refresh.**
    Freeze SoftPC's committed `src/lib`, `src/common`, `test/lib` and
    `test/common` corpus, import the four trees byte-for-byte, and directly
    adapt NXVM consumers.  Consolidate NXVM's separate `vm/product` files
    into `vm/app` by real responsibility: product configuration/catalogue,
    command provider/interaction, recording, version and composition.  Move
    request-string-to-machine-config resolution to App configuration while
    leaving CPU/FPU/device creation at the VM-machine owner.  Delete the
    `vm-product` target, namespace, paths and forwarding edges.  Command and
    hotkey policy may use only public VM-machine operations; it must not
    include runtime lifecycle implementation headers.  This does not move
    Common execution/UI ownership back into NXVM.
17. **S17: NXVM App responsibility convergence.** Move NXVM hotkey/input
    product policy from command parsing into one `app/keyboard` component;
    leave command grammar/result formatting in `app/command`; and leave
    catalogue/configuration resolution and App composition at their existing
    owners. Mirror the relevant SoftPC App responsibility split without
    importing SoftPC product code, adding a queue, changing Common/VM-machine
    contracts, or retaining a duplicate hotkey-policy path.  The existing
    single Common callback context may use one App-local context adapter to
    reach the keyboard owner; it carries no hotkey policy itself.
18. **S18: Common-machine composition ownership.** Align NXVM with the
    corrected SoftPC composition model: `vm/machine` constructs only the
    product driver adapter; `vm/app/composition` creates and owns the one
    `common_machine`, then creates/binds its Common session and UI.  Teardown
    is `UI -> session -> common_machine -> VM adapter`; no owner destroys an
    object still referenced by a live dependent.  Retire the internal
    VM-machine Common creation/ownership path rather than adding a second
    construction mode or a compatibility wrapper.
19. **S19: canonical Common shutdown refresh.** Freeze SoftPC's current
    project-owned `src/common` and `test/common` corpus and import it
    byte-for-byte.  Adopt its permanent, idempotent `common_machine_shutdown`
    contract so App first quiesces the Common worker and its callbacks, then
    releases UI/session callback targets, destroys Common, revokes the VM
    binding and destroys the VM adapter.  Prove exact Common/test identity and
    the new shutdown race proof; do not fork Common or add a second NXVM
    cleanup path.
20. **S20: VM-machine responsibility layout.** Remove the misleading
    `vm/machine/runtime` shell by relocating its machine-driver implementation
    directly beneath `vm/machine`. Keep `vm/machine/media` as the one Core
    FDD/HDD host-storage-provider adaptation, and keep paused-Debug mapping
    within the same machine adapter. Update includes, CMake and owner gates
    atomically; do not add a generic `compat` layer or alter runtime behavior.
21. **S21: presentation-contract corrective closure and package acceptance.**
    Repair the three owner-level defects reported after S20 before attempting
    whole-task acceptance: Common must never re-arm a cooked-monitor prompt
    for an ordinary frame completion; the Win32 Console backend must establish
    the full 80x25 visible viewport before writing a text frame; and the NXVM
    Core-to-VM copied display contract must retain VADP's current character
    scan-line height to map CRTC cursor coordinates into the stable 8x16 glyph
    cell instead of silently treating CRTC coordinates as glyph coordinates.
    The first two are
    canonical Common/Lib behavior and must be transferred to SoftPC without an
    NXVM-only branch; the third remains the NXVM Core display-owner contract.
    Add direct owner tests for all three, prove the two shared trees are either
    byte-identical to the accepted SoftPC revision or explicitly awaiting its
    matching corrective import, then run full unit and external integration
    plus optimized stripped x64/x86 artifacts before package acceptance.
22. **S22: canonical SoftPC Common and Lib corrective import.** Freeze the
    committed SoftPC `fa14c62bd34182bf9930988e25296bfb7d5be646` source and
    test corpus; atomically replace NXVM `src/lib`, `src/common`, `test/lib`,
    and `test/common` byte-for-byte; then adapt only NXVM consumers to the
    revised public contracts. The imported Common contract must keep product
    notifications separate from pending cooked-reader admission, and the
    imported Win32 Console owner must retain its no-shrink viewport policy and
    explicit failure behavior. Delete the superseded NXVM-local shared hunks;
    retain only a necessary NXVM Core-to-frame adapter difference. Prove exact
    four-tree identity, both builds, full repository-only unit coverage, and
    an honestly recorded integration result. This S does not itself claim the
    eventual two-product execution gate.
23. **S23: canonical Host/Common corpus refresh.** Freeze SoftPC's committed
    `src/lib`, `src/common`, `test/lib` and `test/common` corpus; atomically
    replace NXVM's four trees byte-for-byte; then cut NXVM directly to the
    renamed Host, Console and mailbox public contracts.  Delete every retired
    Host-name or prior-mailbox reference rather than retaining a compatibility
    alias.  Prove exact corpus identity, dependency and manifest gates, both
    stripped Release architectures, the complete repository-only unit suite,
    and the external integration suite.  This remains an NXVM import and
    adapter-cutover subtask, not whole-task two-product acceptance.
24. **S24: canonical queue/storage corpus refresh.** Freeze SoftPC's committed
    `src/lib`, `src/common`, `test/lib` and `test/common` corpus; atomically
    replace NXVM's four trees byte-for-byte.  Adopt the canonical embedded
    session-queue/resource-disposal, Common input-queue, Console binding and
    Storage contracts by direct NXVM consumer adaptation only.  Delete every
    superseded NXVM assumption rather than retaining a compatibility spelling
    or parallel queue.  Prove four-tree identity, manifests and component
    gates, both stripped Release architectures, complete repository-only unit
    coverage and the external integration suite.  This remains an import and
    adapter-cutover S, not whole-task two-product acceptance.
25. **S25: canonical frame/state corpus refresh.** Freeze SoftPC's committed
    `src/lib`, `src/common`, `test/lib` and `test/common` corpus; atomically
    replace NXVM's four trees byte-for-byte. Adopt the canonical bounded-frame
    copy, raw-Console mouse ownership, paused-window and optional
    state-transfer contracts only through direct NXVM consumer adaptation.
    State transfer remains unavailable to NXVM product commands until a later
    Core-backed owner is admitted; no placeholder snapshot path may be added.
    Delete superseded shared code and test aggregation omissions. Prove exact
    four-tree identity, manifests and component gates, both stripped Release
    architectures, complete repository-only unit coverage and the external
    integration suite. This remains an import and adapter-cutover S, not
    whole-task two-product acceptance.
26. **S26: canonical modeful-media Common refresh.** Freeze SoftPC's committed
    `src/lib`, `src/common`, `test/lib` and `test/common` corpus; atomically
    replace NXVM's four trees byte-for-byte. Adopt Common's modeful removable
    media request as the one runtime path. Extend NXVM's YAML construction
    request so every `media.floppy` and `media.fixed_disk` entry may declare
    `mode: direct|readonly|overlay`; omitted modes retain the established
    `overlay` default. App resolves this immutable value into VM-machine
    construction, and VM-machine alone opens the selected Storage medium for
    FDD/HDD. Do not add a second mode enum, a YAML-only media path, or edit
    output-session YAML without separate owner direction. Prove exact corpus
    identity, mode parsing/default/error coverage, all three FDD/HDD modes,
    both stripped Release architectures, the complete repository-only unit
    suite and the external integration suite.
27. **S27: canonical six-package x86/Common/Lib import.** Freeze SoftPC's
    committed `src/lib`, `src/common`, `src/x86`, `test/lib`, `test/common`, and
    `test/x86` corpus; atomically replace NXVM's corresponding trees
    byte-for-byte. Adopt the architecture-specific Debug/xasm32 split as a
    product-selected `x86` sibling of neutral Common and Lib, not as a new
    Core owner or a compatibility copy. Adapt only NXVM's App/VM/CMake consumers
    to the public contracts, deleting superseded Common Debug/xasm32 references
    and tests rather than preserving aliases. Prove six-tree identity, each
    corpus manifest/dependency gate, direct NXVM debug/assembly functionality,
    both stripped Release architectures, complete repository-only unit coverage
    and the external integration suite. This remains an import/adaptation S;
    T531 stays open for its two-product execution gate.

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
