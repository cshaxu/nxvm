# Td S126 Second-Pass Architecture And Coding Audit

Status: closed audit evidence. This record supplements, and does not replace,
the closed [Td S125 audit](td-s125-manual-architecture-coding-audit.md). The
active contract remains [CURRENT.md](../../states/CURRENT.md); rules and design
authorities are linked from [docs/README.md](../../README.md).

## Reviewed Surface So Far

| Area | Manual evidence | Disposition |
| --- | --- | --- |
| Core platform stateful contracts | `backing_resource_interface.*`, `input_interface.h`/`input.c`, `presentation_mailbox_interface.h`/`presentation_mailbox.c`, VM consumers | Three public Core interface headers expose locks, callback contexts, provider pointers, and mutable lifecycle state. Confirmed finding A. |
| Core machine collaborator contracts | `media_interface.h`, `display_interface.h`, `machine_interface.h`, VM session/model-40 plan materialization | The public registry, display slot, and plan carry mutable provider/owner endpoints over the Core/VM boundary. Confirmed findings B and C. |
| Core product debugger | `debug.h`, `debug.c`, `debug_target.h`, VM session/console consumers | A public mutable command-interpreter context is embedded in VM session. Confirmed finding D. The target callback table itself remains a bounded injected capability. |
| VM/VDM equivalent paths | VM session/profile/firmware/control headers, VDM headers and CMake target declarations | The S125 VM-contract and test-boundary findings also cover BIOS/control/fault headers; `bios.h` also exposes firmware-construction macros. VDM presentation/machine opaque handles remain a positive narrow boundary, except for the already-recorded pure forwarding session wrapper. |
| Host input route | VM lifecycle/session input path, Core input source, VM request transport, Linux/Win32 ingress | Core input source provides the synchronized copied ingress adapter and the VM transport owns queued command-boundary delivery. No second guest-mutation path was found. |
| Host backing-resource path | Core backing-resource implementation/test uses; VM media, BYOB, catalog and debugger file calls | Core backing resource has no production consumer; parallel direct C-facade file paths remain. Confirmed finding E. |

## Confirmed Transfers

| ID | Finding | Evidence | TODO transfer |
| --- | --- | --- |
| A | Core platform stateful-interface encapsulation | Public backing resource, input source and mailbox layouts expose state that VM embeds/passes. | Core platform stateful-interface encapsulation. |
| B | Core machine collaborator-state interface sealing | Public media registry and display provider slot expose bindings/context/frozen state and are embedded in VM session. | Core machine collaborator-state interface sealing. |
| C | Core machine-plan provider-endpoint closure | Public plan memory-device owner/callbacks and topology endpoint pointers are populated by VM before Core create. | Core machine-plan provider-endpoint closure. |
| D | Core product debugger context boundary repair | `core_product_debug_context` exposes mutable parser/command/dependency state and VM embeds it. | Core product debugger context boundary repair. |
| E | Host backing-resource production-path convergence | Production FDD/HDD/media-save/BYOB/catalog/debug use direct file facade; backing resource has only implementation/test consumers. | Host backing-resource production-path convergence. |

## Review Limit

This pass did not complete the remaining VM/VDM owner, test, and build-path
sweep or assert a whole-repository absence of further violations. The standing
comprehensive audit continues in a later governance packet.
