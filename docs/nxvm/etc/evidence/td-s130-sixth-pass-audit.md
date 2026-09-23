# Td S130 Sixth-Pass Architecture And Coding Audit

Status: closed audit evidence. This record supplements the closed
[S125](td-s125-manual-architecture-coding-audit.md) through
[S129](td-s129-fifth-pass-audit.md) audits. The active contract is
[CURRENT.md](../../states/CURRENT.md); rules and design authorities remain
linked from [docs/README.md](../../README.md).

## Review Scope

This pass manually traces selected production `src/vm` dependencies on
Core-private machine headers. It distinguishes unused include residues from a
real interface gap, and records only the resulting confirmed contract issue.
It does not classify every private include in the repository, implement the
repair, or make a whole-repository conformance or runtime-correctness claim.

## Reviewed Surface

| Area | Manual evidence | Disposition |
| --- | --- | --- |
| Default-profile keyboard mapping | `src/vm/profile/default_profile/keyboard_mapper.c` includes private `core/machine/kbc.h` and uses only `CORE_MACHINE_KBC_SCAN_SET_1` and `_2`; `src/core/machine/kbc.h` also exposes private KBC state and helpers. The existing public `core_machine_keyboard_get_native_scan_set` operation in `machine_interface.h` returns the same untyped byte but its contract exports neither bounded scan-set vocabulary nor a typed result. | Confirmed finding A: VM depends on a private owner header for a public cross-owner fact. Extend the existing cross-module VM contract-boundary repair to make the two-valued scan-set result a bounded public contract at the current machine operation boundary, without moving KBC layout/state across it. |
| Floppy media implementation | `src/vm/machine/fdd.c` includes private `core/machine/dma.h`; include/use sweep finds no `core_machine_dma_*`, DMA type, or private macro use in that file. | Unused private implementation dependency. Transfer removal with finding A's existing repair; no separate abstraction or behavior change is justified. |
| Session firmware assembly | `src/vm/composition/session/profile_firmware.c` includes private `dma.h`, `pic.h`, and `pit.h`; include/use sweep finds no corresponding type, operation, or macro use. | Three unused private implementation dependencies. Transfer removal with finding A's existing repair; they do not independently establish a state or lifecycle violation. |
| Session factory | `src/vm/composition/session/session_factory.c` includes private `core/machine/memory.h`; its selected memory use is supplied through `machine_interface.h`/declared public types, not that private header. | Unused private implementation dependency. Transfer removal with finding A's existing repair. |
| Model-40 composition | `src/vm/profile/model40/model40.c` uses declared `core_machine_memory_*` capability types through the public machine interface while registering profile-selected composition. | Bounded non-finding for this pass: it is not evidence of the KBC private-header leak or an additional private state-layout dependency. |

## Confirmed Transfer

| ID | Finding | TODO transfer |
| --- | --- | --- |
| A | Private KBC scan-set constants leak into VM; four selected unused Core-private include residues | Cross-module VM contract boundary normalization (scope expanded, no duplicate debt entry). |

## Review Limit

This pass disposes only the selected VM-to-Core machine-header dependencies.
The remaining Core/VM headers, public layouts, composition, test white-box
access, lifecycle, and runtime behavior remain within the earlier transfers or
unreviewed audit scope.
