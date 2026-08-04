# Repository Rules

This file is the compact working reference for source layout, code ownership,
and engineering governance. It complements, but does not replace,
`docs/planning/status.md` (active work), `docs/architecture/module-layout.md`
(layout authority), `docs/architecture/contracts.md` (public contract
authority), `docs/source-policy.md` (source/license policy), and
`docs/planning/execution-policy.md` (task execution policy).

## Product Forms And Modules

A product form is one of `core`, `vm`, or `vdm`.

- `core` is the shared, product-neutral foundation.
- `vm` is the bootable NXVM product.
- `vdm` is the future non-bootable ntvdm64 DOS application runner.

A module is one of `machine`, `platform`, `product`, or `profile`.
`core` has only `machine`, `platform`, and `product`; `profile` exists only
under `vm` and `vdm`.

```text
src/
  type.c, type.h
  core/{machine,platform,product}/
  vm/main.c
  vm/composition/
  vm/{machine,platform,product,profile}/
  vdm/main.c                 (only when the VDM product entry exists)
  vdm/composition/
  vdm/{machine,platform,product,profile}/
```

No other top-level source root is permitted. Headers live beside their
implementation. `vm/main.c` and `vdm/main.c` are thin product entries; each
builds its own composition and enters that product's outer control loop.

## Ownership And Dependencies

The three core modules are independent libraries:

- `core/machine` owns product-neutral guest mechanics: CPU, memory, port bus,
  interrupts, deterministic bounded execution, shared devices, guest-domain
  timing, and machine-side provider registries.
- `core/platform` owns product-neutral host capabilities and reusable host
  integrations. It never mutates guest state.
- `core/product` owns product-neutral user-control tooling: command/debug/trace
  contracts, assembler/disassembler, opaque product-session registry/command
  tooling, result and registry utilities. It owns no concrete machine session,
  host policy, boot media, profile, or concrete product UI.

Within `vm` and `vdm`, `machine`, `platform`, `product`, and `profile` are
peers. A peer module may depend on the matching `core` contract it needs, but
may not include or call another peer module directly.

```text
core/machine      core/platform      core/product
     ^                  ^                  ^
     |                  |                  |
vm/machine       vm/platform       vm/product       vm/profile
vdm/machine      vdm/platform      vdm/product      vdm/profile
      \              |              /
       +---- product-root composition ----+
```

Only `vm/composition` and `vdm/composition` may integrate modules. Composition
selects a profile, creates the session, binds providers, converts machine
snapshots to platform frames, owns the outer event loop/threads/pacing, and
performs teardown. It does not create a second CPU executor, duplicate guest
state, or implement a VM-side guest instruction loop.

`core/composition` is forbidden. A reusable, opaque, policy-free product tool
belongs in `core/product`; concrete session/profile/platform assembly belongs
only in `vm/composition` or `vdm/composition`.

Forbidden dependencies include:

- any `core` include of `vm/*` or `vdm/*`;
- any dependency between `core/machine`, `core/platform`, and `core/product`;
- any `vm` to `vdm` or `vdm` to `vm` dependency;
- any sibling-module dependency inside one product form;
- platform code that mutates guest state;
- profile code that creates a machine, selects product policy, or calls a
  sibling module directly; and
- dependency cycles hidden through aggregate CMake targets.

Reuse decides ownership. Concrete Win32/Linux code shared by both products
belongs in `core/platform/{win32,linux}`; product-only mechanisms remain under
the corresponding `vm/*` or `vdm/*` module. A profile may contain declarative
topology, ROM assets, and limited firmware overrides only through public core
provider contracts.

## Interfaces, Providers, And Names

- A cross-module exposed contract is named `<subject>_interface.h`, for
  example `machine_interface.h`, `memory_interface.h`, `port_interface.h`,
  `debug_interface.h`, `trace_interface.h`, or `lifecycle_interface.h`.
- An implementation injected into an owner is named `*_provider`; callback
  types end in `_provider`, and installation functions use
  `_bind_provider` or `_install_provider`.
- Registries own registration, conflict handling, lookup, and freeze. A
  descriptor is not a provider.
- Private implementation layout and helpers use `<subject>.h` beside their
  implementation. They are not cross-module contracts merely because they are
  included today; minimize and remove such boundary leaks deliberately.
- Public symbols state their owner path: `core_machine_*`,
  `core_platform_*`, `core_product_*`, `vm_machine_*`, `vm_platform_*`,
  `vm_product_*`, `vm_profile_*`, and VDM counterparts. Root composition owns
  `vm_session_*` and `vdm_session_*`.
- A file must accurately describe its real responsibility. Do not place
  lifecycle ownership, profile wiring, provider binding, UI policy, or test
  support in a generic file name merely because it is convenient. Split,
  relocate, rename, or delete it once its true responsibility is known.
- File size and file count are not quality metrics. A large, cohesive domain
  implementation is allowed when it has one clear owner and responsibility, a
  narrow external boundary, consistent internal organization, and focused
  verification. Keeping such code together can be more readable than
  scattering one subsystem across many small files.
- Split a file only when ownership, lifecycle, policy, or independent change
  and test boundaries genuinely differ; never impose a line-count threshold or
  fragment a well-encapsulated implementation merely for visual uniformity.
- Do not copy an NXVM implementation merely to fill a new directory. Use
  `git mv`, repair direct dependencies, verify, then proceed to the next
  bounded move.

Approved compact domain vocabulary is retained: `kbc`, `vadp`, `win32app`,
`win32con`, `linuxapp`, `linuxcon`, `w32*`, `xasm32`, `aasm`, `dasm`, `debug`,
and documented `qd*` default-profile firmware hacks. These exceptions do not
remove the required owner prefix from public APIs.

## Machine And Session Rules

- `core_machine` is the sole owner of guest CPU, decoder, memory, port bus,
  core shared-device state, and synchronous bounded `core_machine_run()`.
- `core_machine` owns its generic device prepare/reset/finalize order.
  Composition may install VM/VDM-only devices, profile firmware, and providers,
  but may not become a second generic-device lifecycle owner.
- A machine is configured through explicit providers and registries, frozen
  before execution, and reset without changing frozen topology. Provider order,
  failure handling, lifetime, and re-entry constraints are fixed by the owning
  core contract.
- `INITIALIZED` is configuration-only; a completed initial or explicit reset is
  `STOPPED`; `PAUSED` is only a post-execution/debug boundary; `RUNNING` exists
  only inside `core_machine_run`; and `FAULTED` requires reset. A stopped-only
  cold operation must reject every other lifecycle state. `STOPPED` is always
  cold-start-ready: CPU, RAM, shared devices, and frozen providers have
  completed reset, the reset vector is valid, and the next run may begin
  without an intermediate initialization state. A running stop or reset request
  must complete that cold reset before exposing `STOPPED`; a failed reset must
  not expose a half-initialized stopped state.
- RAM is core-owned. Its installed range is explicit and accesses outside it
  must fail by the machine mapping contract, never wrap modulo RAM size.
  Providers may retain `t_ram *` but may not cache or expose a backing pointer.
  Only `core_machine_reconfigure_memory` may replace RAM backing, only while
  stopped; it cold-resets the same machine and frozen provider topology. CPU,
  FPU, profile/ROM, and port/IRQ topology require a new session.
- CPU architecture and FPU capability are independent frozen per-machine
  configuration. An unavailable instruction form must fault through its
  documented guest path before side effects; a legal FPU escape must not be
  mislabeled as an undefined opcode merely because the configured FPU is absent
  or not yet implemented.
- A product outer loop owns host input draining, Console commands, platform
  events, pacing, host waits, cancellation, product exit, and repeated bounded
  calls to `core_machine_run()`. It never executes guest instructions itself.
- Each mutable datum has one explicit owner: a session, its execution thread,
  a caller-owned invocation object, or an explicit host-surface lease.
  Immutable tables and descriptors may be shared.
- No production path may choose a machine/session through a process global,
  `_Thread_local`, singleton, or implicit current-object facade. A necessary
  process-exclusive host resource uses a caller-owned lease with one creator,
  one releaser, and an explicit failure result for a second claimant.
- A `core_product_session_manager` keeps one or more live opaque entries and
  exactly one selected entry. The final session is never closable; a close of
  any other selected entry deterministically selects a remaining entry.
- Workers may report state or completion; only their session-owned run handle
  controls cancellation, join, backend destruction, and release of shared
  session/platform state.
- Machine snapshots and platform frames are separate copied payloads. Only
  product-root composition may translate a machine snapshot to a host-facing
  platform frame. A platform header must never name, embed, or point to a
  machine snapshot type.

## Abstraction And Wrapper Discipline

- Keep an abstraction only when it establishes a real boundary or policy: an
  ownership/lifetime transition, provider registration and freeze, validation
  or error/result normalization, data representation conversion, thread or
  command-boundary synchronization, host-resource lease, or a stable
  cross-module contract that prevents a forbidden dependency.
- A wrapper that merely renames an object, forwards one call unchanged, keeps
  a permanent same-object pointer alias, or exists only because a test still
  calls it is not an abstraction. Remove it, merge it into its owner, or make
  the caller use the owner directly.
- Do not delete a thin adapter merely because its individual callbacks forward.
  Retain it when the adapter is the one explicit product/composition boundary,
  selects an opaque session, translates a provider contract, or confines a
  product-specific implementation behind a core contract.
- Every retained wrapper/adapter must name its owner and its boundary in the
  source shape. It must not cache, duplicate, or rebind state owned elsewhere.
  Same-object aliases are allowed only for a documented temporary migration
  with a convergence task and removal condition.
- Review wrapper chains end to end. A chain is valid only when each hop adds
  one of the above responsibilities; never stack generic facade, context,
  session, or manager objects that merely relay the same operation.

## C Vocabulary And Platform Boundaries

`src/type.h` is the single common type and C-library facade boundary.

- Use `C_*` for C language scalar vocabulary, such as `C_VOID`, `C_CHAR`,
  `C_INT`, and `C_FLOAT`.
- Use `STD_*` for ISO C library types, constants, atomics, and wrappers, such
  as `STD_SIZE_T`, `STD_NULL`, `STD_MALLOC`, `STD_CALLOC`, `STD_FREE`,
  `STD_MEMCPY`, and `STD_ATOMIC_*`.
- Use a platform prefix for project-facing system types and wrappers, such as
  `WIN32_BOOL`. Platform SDK types and calls remain implementation-local below
  their platform adapter.
- Fixed-width `int*_t` and `uint*_t` remain allowed as documented compatibility
  vocabulary. Do not create module-local scalar aliases or status types.
- Add a C/standard/system facade in `type.*` before adopting it at call sites;
  do not introduce raw ISO C headers or direct ISO C calls outside that facade.
  Use conditional compilation in `type.*` for platform-dependent mappings.
- Each product module defines `PRODUCT_NAME`; `core/product/banner.h` defines
  the common `PRODUCT_VERSION`, `PRODUCT_COPYRIGHT`, `PRODUCT_BUILD_TIME`, and
  banner print macro. Module APIs do not carry ABI versions, timestamps, or ad
  hoc compatibility probes.

## Scope, Testing, And Governance

- Code in a directory belongs to that directory's owner. A temporary adapter
  must have a named owner and a removal/convergence plan; it is never a reason
  to weaken boundaries.
- Treat test-only models, smoke fixtures, compatibility shims, and historical
  paths as non-production until a real product entry uses them. Do not retain a
  forwarding wrapper solely because a test references it.
- Keep exactly one active subtask in `docs/planning/status.md`. A task record
  defines scope, non-goals, source baseline, API surface, commands, expected
  markers, budgets, assets, and stop conditions before runtime changes.
- Design closes only after it produces a bounded immediate implementation
  breakdown. Do not start a later milestone merely because supporting code
  exists.
- Preserve the retained NXVM Console, debugger, startup path, full-PC FDD/HDD
  checkpoints, and approved task-artifact behavior during structural work.
  Any user-visible change requires explicit approval and regression evidence.
- Each runnable behavior-changing task produces one verified task-level EXE in
  `build/output/` with the task-number version. Design-only work creates no
  artificial executable. Do not commit guest media, Microsoft binaries,
  machine-local paths, raw traces, or unreviewed third-party code.
- After builds and tests, delete owned temporary build products, logs, traces,
  and generated binaries no longer needed by the active or immediate next task.
  Preserve verified task artifacts only under `build/output/`.
- Raw instruction recording and differential debugging require bounded wall
  time, no-progress, byte budget, process cleanup, and post-run deletion.
  Bochx/Bochs is optional developer research only, never a default dependency.
- Deferred source work uses `TODO(High)`, `TODO(Medium)`, or `TODO(Low)` and
  must also be represented in a governed task or long-term ledger when it
  affects compatibility or architecture.

## Review Checklist

Before declaring structural work complete, review every implemented
`core/{machine,platform,product}`, `vm/{machine,platform,product,profile}`,
`vdm/{machine,platform,product,profile}`, both composition roots, CMake
targets, tests, and product entries for:

1. one real state owner and one real execution/startup path;
2. no duplicate executor, cached mirror state, hidden reset, or forwarding
   session/machine wrapper;
3. no global/TLS/current-object selector in a production path;
4. no forbidden include or build-target dependency;
5. explicit provider callback/lifetime/threading rules;
6. accurate file names and directory ownership; and
7. current CMake, artifact, documentation, and real boot/Console evidence.

Extra

- every task has a context, approach, exit criteria
- every issue fix: when done, scan the entire codebase for similar issue that exists
