# T367 S1: VM Profile-Contract Ownership Inventory

## Decision

T367 S1 establishes the before-state for the queued VM profile-contract
migration.  A concrete machine is already selected in `vm`: the default
PC/AT and IBM 5170 Model 339 descriptors are returned by
`vm_session_profile_select`, then materialized into one `core_machine_config`.
`core/machine` does not select IBM, Compaq, XT, firmware, board topology,
media, or a per-machine executor.

The migration is therefore deliberately narrow.  S2 must make the VM-owned
CPU capability/timing contract explicit at the profile-to-session binding
seam, while retaining core's one shared decoder, execution engine and
CPU/DMA transaction lifecycle.  It must not move CPU semantic or
source-timing switches merely because they mention a CPU family.

## Complete selector and consumer inventory

| Surface | Current owner and behavior | S2 disposition |
| --- | --- | --- |
| `src/core/machine/cpu_interface.h` | Defines the generic `DEFAULT`, 8086, 80186, 80286 and 80386 CPU capability enum. | Retain as the private injected core capability vocabulary; later add the distinct 8088 contract here only when its CPU work is admitted. |
| `src/core/machine/cpu.c` and `src/core/machine/machine.c` | Resolve `DEFAULT` to 80386, bind the selected generic CPU/FPU profiles, and use profile switches for decoder semantics and source-timing ledger selection. | Retain shared capability validation, decoder/execution and timing mechanisms.  Core must not acquire a concrete-machine selector. |
| `src/vm/profile/default_profile/pc_at_profile.h` and `.c` | `vm_profile_default_pc_at_descriptor` combines the CPU/FPU, instruction timing, clocks and board data.  Its two constructors select the default PC/AT and IBM 5170 Model 339. | This is the concrete machine/profile owner.  S2 should expose its CPU/timing portion as an explicit VM contract without duplicating the rest of the board descriptor. |
| `src/vm/composition/session/session.c` | `vm_session_profile_select` chooses a VM profile; `vm_session_materialize_profile_core_config` copies its CPU, timing, clock and keyboard values once into the generic core input. | This is the required binding seam.  S2 must preserve the single materialization path and prove it remains the only production profile-to-core binding. |
| `src/vm/composition/session/session_factory.c` | Parses `--cpu` as a generic core enum, defaults it to 80386, and the session override then directly replaces the descriptor CPU profile. | S2 must make this a VM-contract selection/validation path, or explicitly restrict it to a supported VM contract.  It must not permit a board identity to be inferred from a raw core enum. |
| `tests/products/vm_session_initialization_atomicity_smoke.c` | Proves descriptor CPU/FPU/timing materialization and the current override behavior through an initialized core. | Required regression owner for the new binding contract and its rejection/compatibility behavior. |
| `tests/products/nxvm_*_smoke.c`, `tests/firmware/default_pc_at_profile_smoke.c`, and `CMakeLists.txt` | Exercise product option profiles, default profile construction and their registered smoke targets. | Required product/build sweep; preserve current four-family coverage while S2 changes ownership expression. |
| `tests/machine/**` direct `core_machine_config` fixtures | Select generic CPU capabilities to test core semantics and individual timing rows. | Intentionally retain: these are core-level fixtures, not concrete machine-profile selectors. |

The source sweep covered production `src/vm`, profile/composition headers,
all product and firmware smoke consumers, the build graph, and generic core
CPU-profile resolution.  No IBM 5170/Compaq DeskPro/IBM 5150/XT concrete
machine switch was found in `core/machine`; only the 5170 descriptor exists
today, and it is VM-owned.

## Bounded S2 contract

S2's acceptance boundary is a VM-private CPU profile contract containing the
already materialized CPU family, FPU family and timing/clock inputs.  The VM
profile chooses that contract; session composition validates and binds it to
the existing generic `core_machine_config`; core executes the injected
capability through its one existing path.  The existing core `DEFAULT ->
80386` resolution remains a generic-library compatibility fallback unless S2
can retire it without changing direct-core test callers; it is not a concrete
machine choice.

Before S2 may claim completion it must prove all four currently supported
families (8086, 80186, 80286, 80386) retain their exact ledger/timing inputs,
that product option handling cannot silently manufacture a machine identity,
and that there is still one shared execution and CPU/DMA transaction owner.
8088 is a later explicit CPU/profile admission, not an alias for 8086 in this
inventory.

## Transfers and non-claims

This evidence makes no CPU semantic, timing-value, board, device, ROM, media,
or L3 claim.  The actual ownership migration transfers to T367 S2.  Complete
80286 successful-retirement timing, PC/AT bus availability, device service
timing, phase refinement and the 5170 L3 audit remain in their separately
queued receivers; this inventory does not advance any of them.

## Verification

- Repository sweep: `rg -l "CORE_MACHINE_CPU_PROFILE|cpu_profile|instruction_timing|ticks_per_instruction" src/vm tests/products tests/firmware CMakeLists.txt`.
- Core boundary sweep: `rg -n "core_machine_resolve_cpu_profile|CORE_MACHINE_CPU_PROFILE_DEFAULT" src/core/machine/machine.c src/core/machine/machine.h src/core/machine/cpu.c src/core/machine/cpu_interface.h`.
- Manual caller/consumer review of the files named in the table.  No runtime
  code changed in S1; S1 requires documentation governance and diff check
  only.
