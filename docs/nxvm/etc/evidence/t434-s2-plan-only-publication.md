# T434 S2: Plan-Only Production Publication

`M5:T434:S2:PLAN-ONLY-PUBLICATION:OK`
`M5:T434:S2:PROFILE-MIGRATION:OK`
`M5:T434:S2:ROLLBACK-EQUIVALENCE:OK`

## Delivered boundary

The three frozen VM materialization families now construct one copied
`core_machine_plan` and publish only through `core_machine_create_from_plan()`:
the default PC/AT path, the IBM 5170 Model 339 path (through that shared
default-profile materializer), and the private Model-40 BYOB path.  A plan
carries the current absent-memory, planar/D4 board, memory-device, display,
DMA, RTC, FDC and HDC declarations.  Core applies them in dependency order
before returning a machine; any failure destroys the in-progress machine and
leaves the caller's result null.

The FDC DMA binding remains Core-owned: plan application creates it, then
uses it internally for FDC connection.  VM can observe the opaque binding only
after successful publication.  VM retains media and display provider lifetime,
but no VM timing algorithm or profile-local construction callback enters Core.
The Model-40 D4 parity/NMI and write-clear behavior is a Core D4-plan
mechanism; the profile supplies only its existing mapped memory semantics and
the bounded parity-mask storage.

The prior post-create controller configuration functions and their lifecycle
calls were removed.  `core_machine_create()` remains available solely to the
explicit non-production VDM/test classes; no `src/vm` call remains.

## Verification

All executable smokes were linked from the current full source set with GCC
16.1.0, excluding only the Linux-only platform sources and linking the Win32
host libraries.  They passed:

- `core_machine_plan_smoke`: declaration/copy behavior plus invalid FDC
  topology and invalid absent-memory rejection with null output;
- `vm_session_initialization_atomicity_smoke`: default PC/AT materialization,
  invalid Core/profile/controller/HDC rollback and recovery;
- `vm_model40_byob_s20_smoke`: BYOB validation, composition and reset;
- `vm_model40_d4_parity_s22_smoke`: D4 parity diagnostic, IOCHK clear and
  shared memory-parity owner; and
- `vm_ibm_5170_model_339_composition_smoke`: Model-339 composition, no-XMS
  route and shared speaker behavior;
- `vm_fdc_dma_boundary_smoke`: FDC DMA/IRQ boundary and Model-339 512 KiB
  FDC startup; `vm_host_cancellation_smoke`: host cancellation; and
  `cpu_trace_context_smoke`: trace publication context.

Affected sources and the focused plan smoke also pass C11
`-Wall -Wextra -Wpedantic -Werror -fsyntax-only`.  The static production sweep
was `rg -n "core_machine_(create|configure_)\\(" src/vm`; it has no hits.
The broader creation sweep retains only `src/vdm/machine/dos_minimal.c` and
tests, which are the frozen non-production classes named by Td S117.

The local developer artifact is
`build/output/nxvm_0_5_0434.exe`, banner `0.5.0434`, SHA-256
`0252F8FDA17BEC2131606F19E3547B46894AC6B56DD37EC3B16BD302494FAFDC`.

## Architecture and code review

Core still contains no VM profile identity or VM include.  VM selects copied
data and host/provider endpoints; Core owns creation, validation, topology
ordering, DMA binding, controller state and failure cleanup.  The plan did not
create a compatibility publisher or a second controller route.  The only
retained direct creation API callers are low-level test builders and the
future VDM skeleton, outside this runnable VM cutover.

The counted tracked source/test paths are every changed `src/` and `tests/`
path in this S; documentation and the ignored developer artifact are excluded.
`git diff --numstat -- src tests` reports `+329/-104`, net `+225` lines.  The
positive implementation is material: it replaces two VM direct publishers and
all of their post-publication Core configuration routes with the new one-time
copied topology contract; no obsolete production route remains.
