# M5 T246: Validated Initial-State / Entry-Plan

## Status

**S1 active.** This subtask is contract design only. It does not add a DOS
loader, external wrapper, Microsoft ABI/assets, host file/path policy, or a
second execution loop.

## Objective

Apply a direct prepared real-mode state to the one existing `core_machine` only
after a clean reset and complete validation. Traditional NXVM reset-vector ROM
boot remains its current composition-owned behavior; a future VDM/external
consumer is not implemented here.

## Frozen Contract

* Topology, ROM/image mappings, providers, ports, and profile bindings are
  registered only in `INITIALIZED` and freeze before reset. An entry plan never
  registers a mapping or image.
* `core_machine_apply_entry_plan()` is valid only once per cold reset at a
  `STOPPED` boundary. A later reset clears the applied marker and restores the
  ordinary reset vector/state. It never starts a run loop.
* The plan is call-scoped and copied only during application. It may contain a
  bounded real-mode CPU state, one entry physical address plus its expected
  neutral route class, and a bounded array of copied preload sources.
* All preloads must be nonempty, have nonnull source bytes, and preflight as
  writable `ORDINARY_RAM` through T243. A plan may read its entry from ordinary
  RAM or a provider (including T245 ROM), but cannot preload provider memory.
* Before any CPU/RAM change, core validates lifecycle, plan bounds, real-mode
  entry `CS:IP` physical equality, allowed flag mask, every preload range, and
  entry route class. It builds/validates a private candidate CPU state first.
* Only after all validation succeeds does core commit the candidate state and
  all copied RAM preloads. With frozen ordinary RAM routing, the commit has no
  fallible provider callback; an invalid plan leaves reset CPU/RAM/mappings
  unchanged. The plan exposes no RAM pointer or execution-context borrow.
* Accepted state is real mode only: 16-bit `CS/DS/ES/SS`, `IP`, `SP`, general
  registers, and the admitted arithmetic/control flags. It cannot set segment
  caches, CRx/DRx, FPU, A20, or CPU mode.

## Implementation And Verification Plan

S2 adds `core/machine/entry_plan_interface.h` and private reset marker/state
logic. Focused smoke must prove traditional reset remains unchanged; a ROM- or
RAM-backed direct plan reaches its entry through the existing run path; invalid
range/route/source/state plans fail atomically; repeated apply fails until
reset; and reset restores standard behavior. It must also prove an ordinary
RAM preload cannot be redirected to a provider.

S3 runs current GCC/CTest gates and records
`build/output/nxvm_0_5_0246.exe` plus SHA-256.

## Stop Conditions

Stop for owner direction if a requirement needs a second machine/session,
loader loop, raw memory pointer, direct VDM implementation, provider mutation
after freeze, or any NXVM Console/debugger/start/boot behavior change.
