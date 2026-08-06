# M5 T243: Checked Guest-Memory Contract

## Status

**Complete.** S1 changed planning documentation only. S2 implemented the
bounded public query and its focused smoke. S3 closed the current matrix and
the required developer artifact.

## Original Request

Establish the first executable core-boundary task after M5 T242 so a future
direct-entry VDM path can safely inspect and prepare guest state while
preserving the full NXVM machine baseline.

## Objective

Specify the minimum `core/machine` checked guest-memory contract needed by:

* validated initial-state/entry-plan application; and
* a later consumer-registered undefined-instruction transition facility.

The contract must permit checked copies and mapping queries without exposing a
raw RAM pointer, a product session layout, a DOS structure, or a host callback
that mutates guest memory.

## Non-Goals

* No runtime implementation in S1.
* No direct-entry loader, DOS service, external wrapper, mantle module, DLL, or
  package boundary.
* No A20/HMA, protected-mode, ROM mapping, initial-state application, or
  transition implementation; those are separately numbered follow-on tasks.
* No FDC/HDC/CMOS migration and no NXVM UI, firmware, media, or path-policy
  change.

## Reference Baseline

* M5 T242 is the current verified NXVM device baseline.
* `src/core/machine/memory_interface.h` and the execution/memory contracts are
  the current core boundary to inspect.
* `src/vm/composition/session/*`, planned `src/mantle/composition/*`, and
  planned `src/vdm/{machine,platform,product,profile}/*` are the
  product/composition callers to classify.
* `TODO.md` records T243 through T248 and the source-level future topology.

## S1 Inventory And Findings

| Owner and file | Existing operation | Current guard | T243 disposition |
| --- | --- | --- | --- |
| `core/machine/memory_interface.h` and `.c` | `core_machine_memory_read`, `_write`, and `_query` copy or classify physical ranges. | Rejects null/zero input and permits only `STOPPED` or `PAUSED`; delegates a shared route/fault decision to RAM. | Retain as the only product-facing physical copy surface. |
| `core/machine/memory.h` and `.c` | RAM backing, A20 wrapping, physical and real-mode helpers, mapping/device-provider registration, write observers. | Raw `t_ram` capabilities are available during configuration; mapping/provider registration freezes when the machine starts. | Keep as core implementation/configuration machinery. Do not expose it to a future composition or wrapper boundary. |
| `core/machine/machine_interface.h` | Configuration borrows expose `t_ram`, CPU, and `core_machine_cpu_execution_context`; profile binding exposes RAM and execution context. | Documented for composition while `INITIALIZED`; providers can retain child references through core teardown. | This is the principal boundary gap. It remains an internal composition facility for now, but T243 S2 must ensure new callers do not require it merely to copy or validate guest memory. |
| `core/machine/cpu_instructions.h` | Checked linear reads/writes exist on `core_machine_cpu_execution_context`. | Available only to holders of the borrowed internal execution context. | Leave unchanged in T243. T244 will decide the narrow execution-view needed for transition handlers. |
| `vm/profile/default_profile/firmware/*` | Firmware/profile code writes BDA, ROM, IVT, and real-mode service bytes through RAM/profile binding helpers. | VM-owned configuration and firmware policy. | Remain VM policy; T245/T246 may adapt its call site without moving BIOS content or services. |
| `vm/composition/session/debug_target.c` | Debug target adapts borrowed RAM/execution context for real/linear access. | Session/debugger-owned callback boundary; real-mode callback currently discards helper status. | Do not treat as the future generic contract. Preserve behavior during T243; separately audit error propagation when a neutral debug binding has a real consumer. |
| `vdm/machine/dos_minimal.c` | Creates, freezes, and resets a core machine without borrowing memory. | Standard machine lifecycle. | Confirms no present VDM consumer justifies a broad memory facade. |

The S1 query sweep found no `core/machine -> vm`, `core/machine -> vdm`,
`core/machine -> platform`, or `core/machine -> product` implementation edge.
It also confirms that A20 address wrapping already exists in core memory. Any
later A20/HMA work concerns externally observable machine semantics and tests,
not initial relocation of the current A20 implementation.

## Approved S2 Contract Boundary

T243 S2 may make only this narrow addition or clarification to the existing
physical-copy interface:

* checked physical read and write remain copied-buffer operations owned by
  `core_machine`;
* `size == 0` is `TYPE_STATUS_INVALID_ARGUMENT` for read, write, and query;
* a physical address plus size that overflows or extends outside the selected
  route is `TYPE_STATUS_FAULT`, never a wrapping or modulo range;
* a checked physical-range query takes only a `READ` or `WRITE` access enum.
  It introduces neither a fetch permission nor a future ROM-specific access
  class;
* read, write, and query use one shared frozen physical-route resolver: the
  same device-provider selection and ordinary-RAM fallback used by physical
  reads/writes. A query must not infer accessibility from RAM capacity alone,
  bypass a registered provider, or invoke a provider's data callback;
* the query reports route/access eligibility and only the neutral route class
  `ORDINARY_RAM` or `PROVIDER`; it returns no RAM pointer, mapping internals,
  device-provider owner, or product data;
* all operations remain valid only at a defined non-running machine boundary;
* invalid arguments, invalid lifecycle, and inaccessible ranges remain distinct
  deterministic `type_status` results; and
* no linear, real-mode, CPU-context, callback, firmware, DOS, file, or host
  policy field enters this contract.

This is sufficient for a later entry-plan validator to reject impossible
prepared copies before mutation. It is deliberately insufficient for arbitrary
instruction interception: T244 must define its own constrained CPU/execution
view rather than re-exporting `core_machine_cpu_execution_context`.

## Requirements And Evidence

| Requirement | Planned S1 evidence |
| --- | --- |
| Current API inventory | Completed in the S1 inventory table above. |
| Gap classification | Completed: physical copies are usable; range/mapping query is missing; borrowed RAM/CPU internals remain composition-private. |
| Contract proposal | Completed in the approved S2 contract boundary above. |
| Dependency audit | Completed by the S1 query sweep; no prohibited core implementation edge was found. |
| S2 packet | `memory_interface.h/.c` plus the shared physical-route resolver, focused zero-length, overflow, RAM, and registered-provider query tests in `tests/core/machine_memory_reconfigure_smoke.c` or a new core-memory contract smoke, current GCC gate, full CTest matrix, and the required developer artifact. |

## Applicable Rules

* Module Layout: `core/machine` owns mutable guest state; composition integrates
  product peers; platform does not mutate guest state.
* Contracts: cross-module use is an explicit `*_interface.h` or provider
  contract with copied data and defined lifetime.
* Coding Standard: use the smallest existing boundary and do not invent a
  facade merely for test convenience.
* Source Policy: no external historical source or protected guest asset enters
  the repository.
* Execution Policy: one active subtask, scoped evidence, and no runnable
  artifact for this design-only S1 subtask.

## Similar-Issue Sweep

Not applicable in S1. This task defines a prospective contract and performs no
defect repair or production-source change. S2 must define its own applicable
similar-issue sweep before implementation.

## Planned Queries And Verification

```powershell
rg -n "core_machine_.*(read|write|memory|linear|real|map)" src/core src/vm src/mantle src/dos src/vdm tests
rg -n "memory_interface|execution_provider|memory_read|memory_write" src/core src/vm src/mantle src/dos src/vdm tests
rg -n "core/machine|core/platform|core/product|vm/|vdm/" CMakeLists.txt src docs/architecture
git diff --check
```

S1 result: the inventory and contract proposal above, with no source or build
changes. S2 will add focused range/mapping tests and run the current GCC/CTest
gate before producing its required developer artifact.

## S2 Implementation

`core_machine_memory_query()` accepts only `READ` or `WRITE`, returns the
neutral `ORDINARY_RAM` or `PROVIDER` route class, and exposes no storage,
mapping, owner, or provider data. `memory.c` now has one private resolver used
by copied reads, copied writes, and queries. Device providers supply a narrow
query callback so classification never invokes a data callback; an unsupported
provider query falls back through the existing RAM route. The existing EGA
provider was updated to report its active aperture through that callback.

`core-machine-checked-memory-smoke` proves the stopped-boundary guard, A20
route selection, RAM and provider classification, callback non-invocation,
zero length, invalid access, 32-bit overflow, and RAM-end fault behavior.
No product, firmware, DOS, host callback, CPU execution borrow, or new memory
path was added.

## S3 Evidence And Closure

The current GCC gate passed all 34 static/ownership checks and 81/81 CTest
smokes. `build/output/nxvm_0_5_0243.exe` SHA-256 is
`982485420BA4325A1B9A83F1DE54DA68F0CE638C244A6F96F6C037FD1935C076`.

The gate also exposed an existing flaky `vm-timer-firmware-smoke` startup
watchdog: three isolated runs produced two passes and one 3-second timeout.
The test-only host observation limit is now 10 seconds, still below the CTest
30-second limit and still not a guest-clock input. The final full run passed.

## Stop Conditions

Stop and seek owner direction if any proposed operation requires:

* raw access to core RAM or a pointer whose lifetime bypasses core;
* mutation from a platform callback or host thread;
* product-private DOS, firmware, path, or external-runtime state in core;
* a second machine execution path; or
* an unbounded memory map or callback lifetime that cannot be tested.
