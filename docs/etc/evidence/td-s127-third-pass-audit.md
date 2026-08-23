# Td S127 Third-Pass Architecture And Coding Audit

Status: closed audit evidence. This record supplements the closed
[S125](td-s125-manual-architecture-coding-audit.md) and
[S126](td-s126-second-pass-audit.md) audits. The active contract is
[CURRENT.md](../../states/CURRENT.md); rules and design authorities remain
linked from [docs/README.md](../../README.md).

## Reviewed Surface So Far

| Area | Manual evidence | Disposition |
| --- | --- | --- |
| CMake platform ownership | `CMakeLists.txt` VM platform/composition target declarations | `vm-composition` unconditionally publicly links `user32` and `gdi32`; no alias or host condition exists. Confirmed finding A. |
| CMake source ownership | `VM_COMPOSITION_SOURCES`, `add_current_vm_artifact()`, target-owner mapping | `machine_info.c` belongs to and is linked through `vm-composition`, but the current artifact also compiles it directly. `VM_RUNTIME_SOURCES` is unused. Confirmed finding B. |
| Product session commands | `core/product/session/command.c`, `vm/product/console.c`, caller search | Core implements an uncalled session grammar while VM console implements an overlapping grammar with different behavior. Confirmed finding C. |
| Test-specific source builds | failure-injection target declarations, `PROJECT_T317_SOURCE_OWNERSHIP`, and source consumers | Direct compilation is limited to three named allocation-failure targets, each recorded as a target-local source-ownership exception and compiled with an allocator substitution. They do not add a production header contract or create a second production route. No new finding; retain the exceptions as test-local instrumentation while the wider test-boundary task inventories their consumers. |
| VM integration test ownership | `tests/machine/vm_*`, `tests/products/*`, `tests/support/core_machine_cpu_fixture.h` | At least 33 VM machine tests dereference `session->core_machine` internals; the shared fixture itself directly manipulates Core executor/device storage and is consumed by both Core and VM tests. Confirmed finding D: the existing cross-owner test-boundary debt is materially broader than its initial examples. |

## Confirmed Transfers

| ID | Finding | TODO transfer |
| --- | --- | --- |
| A | CMake VM-composition native-link separation | CMake VM-composition native-link separation. |
| B | CMake duplicate production-source ownership repair | CMake duplicate production-source ownership repair. |
| C | Product session-command authority unification | Product session-command authority unification. |
| D | Systemic cross-owner VM/Core test coupling | Cross-owner test-boundary repair (scope expanded). |

## Review Limit

This pass disposes only the selected CMake, product-command, test-boundary,
and allocation-failure instrumentation surfaces. It does not dispose every
Core-machine internal, product, test, build-path, document, or runtime path,
and does not make a whole-repository conformance or runtime-correctness claim.
